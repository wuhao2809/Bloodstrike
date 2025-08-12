#include "NetworkSystem.h"
#include "../components/Components.h"
#include <iostream>
#include <cstring>

NetworkSystem::NetworkSystem()
    : currentState(NetworkState::DISCONNECTED), isHost(false), localPlayerID(0), serverSocket(nullptr), socketSet(nullptr), hostIP(""), port(7777), connectionTimeout(5000) // 5 seconds
      ,
      lastHeartbeat(0)
{
    if (!initializeSDLNet())
    {
        std::cerr << "Failed to initialize SDL_net" << std::endl;
    }
    else
    {
        std::cout << "NetworkSystem initialized" << std::endl;
    }
}

NetworkSystem::~NetworkSystem()
{
    disconnect();
    shutdownSDLNet();
}

bool NetworkSystem::initializeSDLNet()
{
    if (SDLNet_Init() < 0)
    {
        std::cerr << "SDLNet_Init failed: " << SDLNet_GetError() << std::endl;
        return false;
    }

    socketSet = SDLNet_AllocSocketSet(2); // Host socket + client socket
    if (!socketSet)
    {
        std::cerr << "SDLNet_AllocSocketSet failed: " << SDLNet_GetError() << std::endl;
        return false;
    }

    return true;
}

void NetworkSystem::shutdownSDLNet()
{
    if (socketSet)
    {
        SDLNet_FreeSocketSet(socketSet);
        socketSet = nullptr;
    }
    SDLNet_Quit();
}

void NetworkSystem::update(ECS &ecs, GameManager &gameManager, float deltaTime)
{
    if (currentState == NetworkState::DISCONNECTED)
        return;

    // Handle incoming connections (if host)
    if (isHost && currentState == NetworkState::HOST_WAITING)
    {
        handleIncomingConnections();
    }

    // Receive and process messages
    if (receiveMessages())
    {
        processIncomingMessages(ecs, gameManager);
    }

    // Send queued messages
    processOutgoingMessages();

    // Handle heartbeat/ping
    handleHeartbeat();
}

bool NetworkSystem::startHost(uint16_t hostPort)
{
    if (currentState != NetworkState::DISCONNECTED)
    {
        std::cerr << "Cannot start host: already connected" << std::endl;
        return false;
    }

    port = hostPort;
    isHost = true;
    localPlayerID = generatePlayerID();

    IPaddress serverIP;
    if (SDLNet_ResolveHost(&serverIP, nullptr, port) < 0)
    {
        std::cerr << "SDLNet_ResolveHost failed: " << SDLNet_GetError() << std::endl;
        return false;
    }

    serverSocket = SDLNet_TCP_Open(&serverIP);
    if (!serverSocket)
    {
        std::cerr << "SDLNet_TCP_Open failed: " << SDLNet_GetError() << std::endl;
        return false;
    }

    if (SDLNet_TCP_AddSocket(socketSet, serverSocket) < 0)
    {
        std::cerr << "SDLNet_TCP_AddSocket failed: " << SDLNet_GetError() << std::endl;
        SDLNet_TCP_Close(serverSocket);
        serverSocket = nullptr;
        return false;
    }

    currentState = NetworkState::HOST_WAITING;
    std::cout << "Host started on port " << port << ", waiting for connections..." << std::endl;
    return true;
}

bool NetworkSystem::joinGame(const std::string &serverIP, uint16_t serverPort)
{
    if (currentState != NetworkState::DISCONNECTED)
    {
        std::cerr << "Cannot join game: already connected" << std::endl;
        return false;
    }

    hostIP = serverIP;
    port = serverPort;
    isHost = false;
    localPlayerID = generatePlayerID();

    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, hostIP.c_str(), port) < 0)
    {
        std::cerr << "SDLNet_ResolveHost failed: " << SDLNet_GetError() << std::endl;
        return false;
    }

    remoteConnection.socket = SDLNet_TCP_Open(&ip);
    if (!remoteConnection.socket)
    {
        std::cerr << "SDLNet_TCP_Open failed: " << SDLNet_GetError() << std::endl;
        return false;
    }

    if (SDLNet_TCP_AddSocket(socketSet, remoteConnection.socket) < 0)
    {
        std::cerr << "SDLNet_TCP_AddSocket failed: " << SDLNet_GetError() << std::endl;
        SDLNet_TCP_Close(remoteConnection.socket);
        remoteConnection.socket = nullptr;
        return false;
    }

    // Send connection request
    NetworkMessage connectMsg(MessageType::CONNECTION_REQUEST, localPlayerID);
    sendMessage(connectMsg);

    currentState = NetworkState::CLIENT_JOINING;
    std::cout << "Connecting to " << hostIP << ":" << port << "..." << std::endl;
    return true;
}

void NetworkSystem::disconnect()
{
    if (currentState == NetworkState::DISCONNECTED)
        return;

    // Send disconnect message if connected
    if (isConnected())
    {
        NetworkMessage disconnectMsg(MessageType::DISCONNECT, localPlayerID);
        sendMessage(disconnectMsg);
    }

    resetConnection();
    currentState = NetworkState::DISCONNECTED;
    std::cout << "Disconnected from network" << std::endl;
}

void NetworkSystem::resetConnection()
{
    if (serverSocket)
    {
        SDLNet_TCP_DelSocket(socketSet, serverSocket);
        SDLNet_TCP_Close(serverSocket);
        serverSocket = nullptr;
    }

    if (remoteConnection.socket)
    {
        SDLNet_TCP_DelSocket(socketSet, remoteConnection.socket);
        SDLNet_TCP_Close(remoteConnection.socket);
        remoteConnection.socket = nullptr;
    }

    remoteConnection.isConnected = false;
    remoteConnection.playerID = 0;

    // Clear message queues
    while (!incomingMessages.empty())
        incomingMessages.pop();
    while (!outgoingMessages.empty())
        outgoingMessages.pop();
}

bool NetworkSystem::handleIncomingConnections()
{
    if (!serverSocket || currentState != NetworkState::HOST_WAITING)
        return false;

    // Check for incoming connections
    if (SDLNet_CheckSockets(socketSet, 0) > 0)
    {
        if (SDLNet_SocketReady(serverSocket))
        {
            TCPsocket clientSocket = SDLNet_TCP_Accept(serverSocket);
            if (clientSocket)
            {
                std::cout << "Client connected!" << std::endl;

                remoteConnection.socket = clientSocket;
                remoteConnection.isConnected = true;
                remoteConnection.lastPingTime = SDL_GetTicks();

                if (SDLNet_TCP_AddSocket(socketSet, clientSocket) < 0)
                {
                    std::cerr << "Failed to add client socket to set" << std::endl;
                    SDLNet_TCP_Close(clientSocket);
                    remoteConnection.socket = nullptr;
                    remoteConnection.isConnected = false;
                    return false;
                }

                currentState = NetworkState::LOBBY;
                return true;
            }
        }
    }

    return false;
}

bool NetworkSystem::receiveMessages()
{
    if (currentState == NetworkState::DISCONNECTED)
        return false;

    bool receivedAny = false;

    if (SDLNet_CheckSockets(socketSet, 0) > 0)
    {
        TCPsocket activeSocket = nullptr;

        // Check which socket has data
        if (remoteConnection.socket && SDLNet_SocketReady(remoteConnection.socket))
        {
            activeSocket = remoteConnection.socket;
        }

        if (activeSocket)
        {
            // Read message header first
            uint8_t headerBuffer[sizeof(NetworkMessage)];
            int bytesReceived = SDLNet_TCP_Recv(activeSocket, headerBuffer, sizeof(NetworkMessage));

            if (bytesReceived > 0)
            {
                NetworkMessage message;
                if (deserializeMessage(headerBuffer, bytesReceived, message))
                {
                    incomingMessages.push(message);
                    receivedAny = true;
                }
            }
            else if (bytesReceived == 0)
            {
                // Connection closed
                std::cout << "Remote connection closed" << std::endl;
                disconnect();
            }
        }
    }

    return receivedAny;
}

void NetworkSystem::processIncomingMessages(ECS &ecs, GameManager &gameManager)
{
    while (hasIncomingMessages())
    {
        NetworkMessage message = popIncomingMessage();

        switch (message.type)
        {
        case MessageType::CONNECTION_REQUEST:
            if (isHost && currentState == NetworkState::HOST_WAITING)
            {
                std::cout << "Received connection request from player " << message.playerID << std::endl;
                remoteConnection.playerID = message.playerID;

                // Send acceptance
                NetworkMessage acceptMsg(MessageType::CONNECTION_ACCEPT, localPlayerID);
                sendMessage(acceptMsg);

                currentState = NetworkState::LOBBY;
            }
            break;

        case MessageType::CONNECTION_ACCEPT:
            if (!isHost && currentState == NetworkState::CLIENT_JOINING)
            {
                std::cout << "Connection accepted by host!" << std::endl;
                remoteConnection.playerID = message.playerID;
                remoteConnection.isConnected = true;
                currentState = NetworkState::LOBBY;
            }
            break;

        case MessageType::DISCONNECT:
            std::cout << "Received disconnect from remote player" << std::endl;
            disconnect();
            break;

        case MessageType::PING:
            // Respond with pong
            {
                NetworkMessage pongMsg(MessageType::PONG, localPlayerID);
                sendMessage(pongMsg);
            }
            break;

        case MessageType::PONG:
            remoteConnection.lastPingTime = SDL_GetTicks();
            break;

        default:
            std::cout << "Received unhandled message type: " << static_cast<int>(message.type) << std::endl;
            break;
        }
    }
}

void NetworkSystem::processOutgoingMessages()
{
    while (!outgoingMessages.empty())
    {
        NetworkMessage message = outgoingMessages.front();
        outgoingMessages.pop();

        if (remoteConnection.socket && remoteConnection.isConnected)
        {
            uint8_t buffer[sizeof(NetworkMessage)];
            size_t bufferSize;

            if (serializeMessage(message, buffer, bufferSize))
            {
                int bytesSent = SDLNet_TCP_Send(remoteConnection.socket, buffer, bufferSize);
                if (bytesSent < static_cast<int>(bufferSize))
                {
                    std::cerr << "Failed to send complete message" << std::endl;
                }
            }
        }
    }
}

void NetworkSystem::handleHeartbeat()
{
    uint32_t currentTime = SDL_GetTicks();

    // Send ping every 2 seconds
    if (currentTime - lastHeartbeat > 2000)
    {
        if (isConnected())
        {
            NetworkMessage pingMsg(MessageType::PING, localPlayerID);
            sendMessage(pingMsg);
        }
        lastHeartbeat = currentTime;
    }

    // Check for connection timeout (10 seconds)
    if (isConnected() && currentTime - remoteConnection.lastPingTime > 10000)
    {
        std::cout << "Connection timeout - disconnecting" << std::endl;
        disconnect();
    }
}

void NetworkSystem::sendMessage(const NetworkMessage &message)
{
    outgoingMessages.push(message);
}

NetworkMessage NetworkSystem::popIncomingMessage()
{
    if (!incomingMessages.empty())
    {
        NetworkMessage message = incomingMessages.front();
        incomingMessages.pop();
        return message;
    }
    return NetworkMessage();
}

bool NetworkSystem::hasIncomingMessages() const
{
    return !incomingMessages.empty();
}

bool NetworkSystem::isConnected() const
{
    return currentState == NetworkState::LOBBY || currentState == NetworkState::IN_GAME;
}

bool NetworkSystem::serializeMessage(const NetworkMessage &message, uint8_t *buffer, size_t &bufferSize)
{
    if (!buffer)
        return false;

    // Simple serialization - just copy the struct
    memcpy(buffer, &message, sizeof(NetworkMessage));
    bufferSize = sizeof(NetworkMessage);
    return true;
}

bool NetworkSystem::deserializeMessage(const uint8_t *buffer, size_t bufferSize, NetworkMessage &message)
{
    if (!buffer || bufferSize < sizeof(NetworkMessage))
        return false;

    // Simple deserialization - just copy the struct
    memcpy(&message, buffer, sizeof(NetworkMessage));
    return true;
}

uint32_t NetworkSystem::generatePlayerID()
{
    return SDL_GetTicks() + (rand() % 1000);
}
