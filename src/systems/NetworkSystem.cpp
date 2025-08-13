#include "NetworkSystem.h"
#include "MobSpawningSystem.h"
#include "WeaponSystem.h"
#include "MovementSystem.h"
#include "../components/Components.h"
#include <iostream>
#include <cstring>

NetworkSystem::NetworkSystem()
    : currentState(NetworkState::DISCONNECTED), isHost(false), localPlayerID(0), serverSocket(nullptr), socketSet(nullptr), hostIP(""), port(7777), connectionTimeout(5000) // 5 seconds
      ,
      lastHeartbeat(0), debugMode(false) // Disable debug mode by default for cleaner output
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

    // Host sends periodic game state updates during gameplay
    if (isHost && this->gameManager && this->gameManager->currentState == GameManager::PLAYING && isConnected())
    {
        static float gameStateUpdateTimer = 0.0f;
        gameStateUpdateTimer += deltaTime;

        // Step 2: Send optimized game state update every 0.2 seconds (5 FPS instead of 10 FPS)
        if (gameStateUpdateTimer >= 0.2f)
        {
            sendGameStateUpdate(ecs, this->gameManager->score, gameStartTime);
            gameStateUpdateTimer = 0.0f;
        }
    }
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

    // Initialize connection state
    remoteConnection.isConnected = true;
    remoteConnection.lastPingTime = SDL_GetTicks();

    currentState = NetworkState::LOBBY; // Directly go to LOBBY instead of CLIENT_JOINING
    std::cout << "Connecting to " << hostIP << ":" << port << "..." << std::endl;
    std::cout << "Client: Successfully connected, transitioning to LOBBY state" << std::endl;
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

                // Send initial lobby status to newly connected client
                sendLobbyStatus();
                std::cout << "Host: Sent initial lobby status to connected client" << std::endl;

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

        if (debugMode)
        {
            std::cout << "[DEBUG] Processing incoming message type: " << static_cast<int>(message.type)
                      << " from player " << message.playerID << " (dataSize: " << message.dataSize << ")" << std::endl;
        }
        else
        {
            std::cout << "Processing message type: " << static_cast<int>(message.type) << " from player " << message.playerID << std::endl;
        }

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
                std::cout << "Host: Connection accepted, transitioning to LOBBY state" << std::endl;
            }
            break;

        case MessageType::CONNECTION_ACCEPT:
            if (!isHost && currentState == NetworkState::CLIENT_JOINING)
            {
                std::cout << "Connection accepted by host!" << std::endl;
                remoteConnection.playerID = message.playerID;
                remoteConnection.isConnected = true;
                currentState = NetworkState::LOBBY;
                std::cout << "Client: Connection accepted, transitioning to LOBBY state" << std::endl;
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

        case MessageType::PLAYER_INPUT:
            std::cout << "Received PLAYER_INPUT message" << std::endl;
            // Handle player input (this will be processed by InputSystem)
            break;

        case MessageType::MOB_KING_INPUT:
            std::cout << "Received MOB_KING_INPUT message" << std::endl;
            // Handle mob king input (this will be processed by InputSystem)
            break;

        case MessageType::PLAYER_READY:
        {
            std::cout << "Received PLAYER_READY message" << std::endl;
            PlayerReadyData readyData;
            if (message.dataSize >= sizeof(PlayerReadyData))
            {
                memcpy(&readyData, message.data, sizeof(PlayerReadyData));
                remotePlayerReady = readyData.isReady;
                std::cout << "Remote player " << readyData.playerID << " is " << (readyData.isReady ? "READY" : "NOT READY") << std::endl;

                // If we're host, send updated lobby status
                if (isHost)
                {
                    sendLobbyStatus();
                }
            }
        }
        break;

        case MessageType::LOBBY_STATUS:
        {
            std::cout << "Received LOBBY_STATUS message" << std::endl;
            LobbyStatusData lobbyData;
            if (message.dataSize >= sizeof(LobbyStatusData))
            {
                memcpy(&lobbyData, message.data, sizeof(LobbyStatusData));
                if (!isHost) // Client receives lobby updates from Host
                {
                    remotePlayerReady = lobbyData.hostReady; // Host's ready state
                    gameStartCountdown = lobbyData.gameStarting;

                    if (gameStartCountdown && lobbyData.countdown > 0)
                    {
                        std::cout << "Game starting in " << lobbyData.countdown << " seconds..." << std::endl;
                    }
                    else if (gameStartCountdown && lobbyData.countdown == 0)
                    {
                        std::cout << "Game starting now!" << std::endl;
                        // Trigger game start (will be handled by MenuSystem)
                    }
                }
            }
        }
        break;

        case MessageType::GAME_STATE_UPDATE:
        {
            std::cout << "Received GAME_STATE_UPDATE message" << std::endl;

            // Step 2: Handle optimized game state data
            if (message.dataSize == sizeof(GameStateData))
            {
                GameStateData stateData;
                memcpy(&stateData, message.data, sizeof(GameStateData));

                if (!isHost)
                {
                    std::cout << "[CLIENT] Applying optimized game state: score=" << stateData.score
                              << ", mobKing=" << stateData.mobKingCurrentHealth << "/" << stateData.mobKingMaxHealth << std::endl;

                    // Apply authoritative data from host
                    gameManager.score = stateData.score;

                    // Calculate local time from game start timestamp
                    uint32_t currentTime = SDL_GetTicks();
                    gameManager.gameTime = (currentTime - stateData.gameStartTime) / 1000.0f;

                    // Update mob king health on client for UI
                    auto &mobKingEntities = ecs.getComponents<MobKing>();
                    std::cout << "[CLIENT] Found " << mobKingEntities.size() << " mob king entities" << std::endl;

                    for (auto &[entityID, mobKing] : mobKingEntities)
                    {
                        auto *health = ecs.getComponent<Health>(entityID);
                        if (health)
                        {
                            std::cout << "[CLIENT] Updating mob king health from " << health->currentHealth
                                      << "/" << health->maxHealth << " to " << stateData.mobKingCurrentHealth
                                      << "/" << stateData.mobKingMaxHealth << std::endl;
                            health->currentHealth = stateData.mobKingCurrentHealth;
                            health->maxHealth = stateData.mobKingMaxHealth;
                            break;
                        }
                        else
                        {
                            std::cout << "[CLIENT] Mob king entity " << entityID << " found but no Health component!" << std::endl;
                        }
                    }
                }
            }
            else
            {
                std::cout << "[ERROR] Received GAME_STATE_UPDATE with unexpected data size: " << message.dataSize << std::endl;
            }
        }
        break;

        case MessageType::ENTITY_POSITION_UPDATE:
        {
            EntityPositionData posData;
            if (message.dataSize >= sizeof(EntityPositionData))
            {
                memcpy(&posData, message.data, sizeof(EntityPositionData));
                // Apply position update on Client side (Host is authoritative)
                if (!isHost)
                {
                    std::string entityType(posData.entityType);
                    std::cout << "[CLIENT] Received position update for " << entityType << " ID:" << posData.entityID
                              << " at (" << posData.x << ", " << posData.y << ")" << std::endl;

                    // Find and update the entity position
                    // This will be handled by MovementSystem or a dedicated sync system
                    if (movementSystem)
                    {
                        movementSystem->updateEntityFromNetwork(ecs, posData.entityID, posData.x, posData.y,
                                                                posData.velocityX, posData.velocityY, entityType);
                    }
                }
            }
        }
        break;

        case MessageType::MOB_SPAWN:
        {
            std::cout << "Received MOB_SPAWN message" << std::endl;
            MobSpawnData spawnData;
            if (message.dataSize >= sizeof(MobSpawnData))
            {
                memcpy(&spawnData, message.data, sizeof(MobSpawnData));
                // Create mob entity on Client side (Host is authoritative)
                if (!isHost && mobSpawningSystem)
                {
                    std::string mobType(spawnData.mobType);
                    EntityID localEntityID = mobSpawningSystem->createMobFromNetwork(ecs, spawnData.mobID,
                                                                                     spawnData.x, spawnData.y,
                                                                                     spawnData.velocityX, spawnData.velocityY,
                                                                                     mobType);

                    // Register the entity mapping for future removal
                    registerNetworkEntity(spawnData.mobID, localEntityID);
                    std::cout << "[CLIENT] Registered mob mapping: network ID " << spawnData.mobID
                              << " -> local ID " << localEntityID << std::endl;
                }
            }
        }
        break;

        case MessageType::PROJECTILE_CREATE:
        {
            std::cout << "Received PROJECTILE_CREATE message" << std::endl;
            ProjectileData projectileData;
            if (message.dataSize >= sizeof(ProjectileData))
            {
                memcpy(&projectileData, message.data, sizeof(ProjectileData));
                // Create projectile entity on Client side (Host is authoritative)
                if (!isHost && weaponSystem)
                {
                    EntityID localEntityID = weaponSystem->createProjectileFromNetwork(ecs, projectileData.projectileID,
                                                                                       projectileData.shooterID,
                                                                                       projectileData.x, projectileData.y,
                                                                                       projectileData.velocityX, projectileData.velocityY,
                                                                                       projectileData.damage, projectileData.fromPlayer);

                    // Register the entity mapping for future removal
                    registerNetworkEntity(projectileData.projectileID, localEntityID);
                    std::cout << "[CLIENT] Registered projectile mapping: network ID " << projectileData.projectileID
                              << " -> local ID " << localEntityID << std::endl;
                }
            }
        }
        break;

        case MessageType::PROJECTILE_HIT:
        {
            std::cout << "Received PROJECTILE_HIT message" << std::endl;
            ProjectileHitData hitData;
            if (message.dataSize >= sizeof(ProjectileHitData))
            {
                memcpy(&hitData, message.data, sizeof(ProjectileHitData));
                // TODO: Apply damage and remove entities (will be handled by ProjectileSystem)
            }
        }
        break;

        case MessageType::ENTITY_REMOVE:
        {
            std::cout << "Received ENTITY_REMOVE message" << std::endl;
            EntityRemoveData removeData;
            if (message.dataSize >= sizeof(EntityRemoveData))
            {
                memcpy(&removeData, message.data, sizeof(EntityRemoveData));
                if (!isHost)
                {
                    std::string entityType(removeData.entityType);
                    std::cout << "[CLIENT] Removing " << entityType << " entity with network ID:" << removeData.entityID << std::endl;

                    // Find the local entity ID using the mapping
                    EntityID localEntityID = getLocalEntityID(removeData.entityID);
                    if (localEntityID != 0)
                    {
                        std::cout << "[CLIENT] Found local entity ID " << localEntityID << " for network ID " << removeData.entityID << std::endl;
                        ecs.removeEntity(localEntityID);
                        unregisterNetworkEntity(removeData.entityID);
                    }
                    else
                    {
                        std::cout << "[CLIENT] Warning: Could not find local entity for network ID " << removeData.entityID << std::endl;
                    }
                }
            }
        }
        break;

        case MessageType::GAME_START:
            std::cout << "Received GAME_START message" << std::endl;
            if (!isHost)
            {
                std::cout << "Client starting networked multiplayer game!" << std::endl;
                gameManager.startNetworkedMultiplayerGame();
            }
            break;

        case MessageType::GAME_OVER:
            std::cout << "Received GAME_OVER message" << std::endl;
            if (!isHost)
            {
                gameManager.gameOver();
            }
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
    if (debugMode)
    {
        std::cout << "[DEBUG] Sending message type: " << static_cast<int>(message.type)
                  << " from player " << message.playerID << " (dataSize: " << message.dataSize << ")" << std::endl;
    }
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

void NetworkSystem::sendPlayerInput(float velocityX, float velocityY, int mouseX, int mouseY, bool shooting)
{
    if (currentState != NetworkState::LOBBY && currentState != NetworkState::CONNECTED)
        return;

    PlayerInputData inputData;
    inputData.velocityX = velocityX;
    inputData.velocityY = velocityY;
    inputData.mouseX = mouseX;
    inputData.mouseY = mouseY;
    inputData.shooting = shooting;
    inputData.timestamp = SDL_GetTicks();

    NetworkMessage message(MessageType::PLAYER_INPUT, localPlayerID);
    message.dataSize = sizeof(PlayerInputData);
    memcpy(message.data, &inputData, sizeof(PlayerInputData));

    sendMessage(message);
}

void NetworkSystem::sendMobKingInput(float velocityX, float velocityY, bool shooting)
{
    if (currentState != NetworkState::LOBBY && currentState != NetworkState::CONNECTED)
        return;

    MobKingInputData inputData;
    inputData.velocityX = velocityX;
    inputData.velocityY = velocityY;
    inputData.shooting = shooting;
    inputData.timestamp = SDL_GetTicks();

    NetworkMessage message(MessageType::MOB_KING_INPUT, localPlayerID);
    message.dataSize = sizeof(MobKingInputData);
    memcpy(message.data, &inputData, sizeof(MobKingInputData));

    sendMessage(message);
}

// Step 2: Optimized game state update with mob king health
void NetworkSystem::sendGameStateUpdate(ECS &ecs, uint32_t score, uint32_t gameStartTime)
{
    if (!isConnected())
        return;

    GameStateData stateData;
    stateData.score = score;
    stateData.gameStartTime = gameStartTime;
    stateData.timestamp = SDL_GetTicks();

    // Get Mob King health for UI display
    stateData.mobKingCurrentHealth = 0.0f;
    stateData.mobKingMaxHealth = 0.0f;

    auto &mobKingEntities = ecs.getComponents<MobKing>();
    for (auto &[entityID, mobKing] : mobKingEntities)
    {
        auto *health = ecs.getComponent<Health>(entityID);
        if (health)
        {
            stateData.mobKingCurrentHealth = health->currentHealth;
            stateData.mobKingMaxHealth = health->maxHealth;
            break; // Only one mob king
        }
    }

    std::cout << "[HOST] Sending optimized game state: score=" << score
              << ", mobKing=" << stateData.mobKingCurrentHealth << "/" << stateData.mobKingMaxHealth << std::endl;

    NetworkMessage message(MessageType::GAME_STATE_UPDATE, localPlayerID);
    message.dataSize = sizeof(GameStateData);
    memcpy(message.data, &stateData, sizeof(GameStateData));

    sendMessage(message);
}

void NetworkSystem::sendEntityPositionUpdate(uint32_t entityID, float x, float y, float velocityX, float velocityY, const std::string &entityType)
{
    if (!isConnected())
        return;

    EntityPositionData posData;
    posData.entityID = entityID;
    posData.x = x;
    posData.y = y;
    posData.velocityX = velocityX;
    posData.velocityY = velocityY;
    strncpy(posData.entityType, entityType.c_str(), sizeof(posData.entityType) - 1);
    posData.entityType[sizeof(posData.entityType) - 1] = '\0';
    posData.timestamp = SDL_GetTicks();

    NetworkMessage message(MessageType::ENTITY_POSITION_UPDATE, localPlayerID);
    message.dataSize = sizeof(EntityPositionData);
    memcpy(message.data, &posData, sizeof(EntityPositionData));

    sendMessage(message);
}

void NetworkSystem::sendMobSpawn(uint32_t mobID, float x, float y, float velocityX, float velocityY, const std::string &mobType)
{
    if (!isConnected())
        return;

    // Register the network entity mapping on the host side
    if (isHost)
    {
        registerNetworkEntity(mobID, mobID); // Use same ID for host mapping
        std::cout << "[HOST] Registered mob mapping: network ID " << mobID << " -> local ID " << mobID << std::endl;
    }

    MobSpawnData spawnData;
    spawnData.mobID = mobID;
    spawnData.x = x;
    spawnData.y = y;
    spawnData.velocityX = velocityX;
    spawnData.velocityY = velocityY;
    strncpy(spawnData.mobType, mobType.c_str(), sizeof(spawnData.mobType) - 1);
    spawnData.mobType[sizeof(spawnData.mobType) - 1] = '\0'; // Ensure null termination
    spawnData.timestamp = SDL_GetTicks();

    NetworkMessage message(MessageType::MOB_SPAWN, localPlayerID);
    message.dataSize = sizeof(MobSpawnData);
    memcpy(message.data, &spawnData, sizeof(MobSpawnData));

    sendMessage(message);
}

void NetworkSystem::sendProjectileCreate(uint32_t projectileID, uint32_t shooterID, float x, float y, float velocityX, float velocityY, float damage, bool fromPlayer)
{
    if (!isConnected())
        return;

    // Register the network entity mapping on the host side
    if (isHost)
    {
        registerNetworkEntity(projectileID, projectileID); // Use same ID for host mapping
        std::cout << "[HOST] Registered projectile mapping: network ID " << projectileID << " -> local ID " << projectileID << std::endl;
    }

    ProjectileData projectileData;
    projectileData.projectileID = projectileID;
    projectileData.shooterID = shooterID;
    projectileData.x = x;
    projectileData.y = y;
    projectileData.velocityX = velocityX;
    projectileData.velocityY = velocityY;
    projectileData.damage = damage;
    projectileData.fromPlayer = fromPlayer;
    projectileData.timestamp = SDL_GetTicks();

    NetworkMessage message(MessageType::PROJECTILE_CREATE, localPlayerID);
    message.dataSize = sizeof(ProjectileData);
    memcpy(message.data, &projectileData, sizeof(ProjectileData));

    sendMessage(message);
}

void NetworkSystem::sendProjectileHit(uint32_t projectileID, uint32_t targetID, float damage, bool destroyed)
{
    if (!isConnected())
        return;

    ProjectileHitData hitData;
    hitData.projectileID = projectileID;
    hitData.targetID = targetID;
    hitData.damage = damage;
    hitData.destroyed = destroyed;
    hitData.timestamp = SDL_GetTicks();

    NetworkMessage message(MessageType::PROJECTILE_HIT, localPlayerID);
    message.dataSize = sizeof(ProjectileHitData);
    memcpy(message.data, &hitData, sizeof(ProjectileHitData));

    sendMessage(message);
}

void NetworkSystem::sendEntityRemove(uint32_t entityID, const std::string &entityType)
{
    if (!isConnected())
        return;

    EntityRemoveData removeData;
    removeData.entityID = entityID;
    strncpy(removeData.entityType, entityType.c_str(), sizeof(removeData.entityType) - 1);
    removeData.entityType[sizeof(removeData.entityType) - 1] = '\0';
    removeData.timestamp = SDL_GetTicks();

    NetworkMessage message(MessageType::ENTITY_REMOVE, localPlayerID);
    message.dataSize = sizeof(EntityRemoveData);
    memcpy(message.data, &removeData, sizeof(EntityRemoveData));

    sendMessage(message);
}

void NetworkSystem::sendGameOver()
{
    if (!isConnected())
        return;

    NetworkMessage message(MessageType::GAME_OVER, localPlayerID);
    sendMessage(message);
}

void NetworkSystem::sendGameStart()
{
    if (!isConnected() || !isHost)
        return;

    // Step 2: Record when the game actually starts for time calculation
    gameStartTime = SDL_GetTicks();

    NetworkMessage message(MessageType::GAME_START, localPlayerID);
    sendMessage(message);

    if (debugMode)
    {
        std::cout << "Host sent GAME_START message to client (startTime=" << gameStartTime << ")" << std::endl;
    }
}

void NetworkSystem::setPlayerReady(bool ready)
{
    localPlayerReady = ready;

    if (debugMode)
    {
        std::cout << "[DEBUG] Local player ready state changed to: " << (ready ? "READY" : "NOT READY") << std::endl;
    }

    // Send ready status to other player
    if (isConnected())
    {
        PlayerReadyData readyData;
        readyData.playerID = localPlayerID;
        readyData.isReady = ready;
        strncpy(readyData.playerName, "Player", sizeof(readyData.playerName) - 1);
        readyData.playerName[sizeof(readyData.playerName) - 1] = '\0';
        readyData.timestamp = SDL_GetTicks();

        NetworkMessage message(MessageType::PLAYER_READY, localPlayerID);
        message.dataSize = sizeof(PlayerReadyData);
        memcpy(message.data, &readyData, sizeof(PlayerReadyData));

        sendMessage(message);
        std::cout << "Sent ready status: " << (ready ? "READY" : "NOT READY") << std::endl;
    }

    // Update lobby status if host
    if (isHost)
    {
        sendLobbyStatus();
    }
}

void NetworkSystem::sendLobbyStatus()
{
    if (!isConnected() || !isHost)
        return;

    LobbyStatusData lobbyData;
    lobbyData.hostReady = localPlayerReady;
    lobbyData.clientReady = remotePlayerReady;
    lobbyData.gameStarting = gameStartCountdown;
    lobbyData.countdown = gameStartCountdown ? (3 - (SDL_GetTicks() - countdownStartTime) / 1000) : 0;
    lobbyData.timestamp = SDL_GetTicks();

    NetworkMessage message(MessageType::LOBBY_STATUS, localPlayerID);
    message.dataSize = sizeof(LobbyStatusData);
    memcpy(message.data, &lobbyData, sizeof(LobbyStatusData));

    sendMessage(message);

    // Check if we should start countdown
    if (areBothPlayersReady() && !gameStartCountdown)
    {
        gameStartCountdown = true;
        countdownStartTime = SDL_GetTicks();
        std::cout << "Both players ready! Starting countdown..." << std::endl;
    }
}

// Entity ID mapping methods for network synchronization
void NetworkSystem::registerNetworkEntity(uint32_t networkID, EntityID localID)
{
    networkToLocalEntityMap[networkID] = localID;
    localToNetworkEntityMap[localID] = networkID;
}

EntityID NetworkSystem::getLocalEntityID(uint32_t networkID)
{
    auto it = networkToLocalEntityMap.find(networkID);
    return it != networkToLocalEntityMap.end() ? it->second : 0;
}

uint32_t NetworkSystem::getNetworkEntityID(EntityID localID)
{
    auto it = localToNetworkEntityMap.find(localID);
    return it != localToNetworkEntityMap.end() ? it->second : 0;
}

void NetworkSystem::unregisterNetworkEntity(uint32_t networkID)
{
    auto it = networkToLocalEntityMap.find(networkID);
    if (it != networkToLocalEntityMap.end())
    {
        EntityID localID = it->second;
        networkToLocalEntityMap.erase(it);
        localToNetworkEntityMap.erase(localID);
    }
}
