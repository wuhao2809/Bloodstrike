#pragma once
#include "System.h"
#include "../managers/GameManager.h"
#include "../components/Components.h"
#include <SDL2/SDL_net.h>
#include <string>
#include <vector>
#include <queue>

enum class MessageType : uint8_t
{
    // Connection & Lobby
    CONNECTION_REQUEST = 1,
    CONNECTION_ACCEPT,
    CONNECTION_REJECT,
    DISCONNECT,

    // Lobby System
    DICE_ROLL_REQUEST,
    DICE_ROLL_RESULT,
    ROLE_SELECTION,
    GAME_START,

    // Gameplay
    PLAYER_INPUT,
    MOB_KING_INPUT,
    GAME_STATE_UPDATE,
    MOB_SPAWN,
    PROJECTILE_CREATE,
    PROJECTILE_HIT,
    MOB_KING_DEATH,
    GAME_OVER,

    // Heartbeat
    PING,
    PONG
};

struct NetworkMessage
{
    MessageType type;
    uint32_t playerID;
    uint32_t timestamp;
    uint16_t dataSize;
    uint8_t data[256]; // Max message payload

    NetworkMessage() : type(MessageType::PING), playerID(0), timestamp(0), dataSize(0) {}
    NetworkMessage(MessageType t, uint32_t id) : type(t), playerID(id), timestamp(SDL_GetTicks()), dataSize(0) {}
};

struct ConnectionInfo
{
    TCPsocket socket;
    IPaddress address;
    uint32_t playerID;
    bool isConnected;
    uint32_t lastPingTime;

    ConnectionInfo() : socket(nullptr), playerID(0), isConnected(false), lastPingTime(0) {}
};

class NetworkSystem : public System
{
private:
    // Network state
    NetworkState currentState;
    bool isHost;
    uint32_t localPlayerID;

    // SDL_net objects
    TCPsocket serverSocket;
    ConnectionInfo remoteConnection;
    SDLNet_SocketSet socketSet;

    // Message handling
    std::queue<NetworkMessage> incomingMessages;
    std::queue<NetworkMessage> outgoingMessages;

    // Configuration
    std::string hostIP;
    uint16_t port;

    // Connection management
    uint32_t connectionTimeout;
    uint32_t lastHeartbeat;

public:
    NetworkSystem();
    ~NetworkSystem();

    void update(ECS &ecs, GameManager &gameManager, float deltaTime) override;

    // Connection management
    bool startHost(uint16_t port = 7777);
    bool joinGame(const std::string &hostIP, uint16_t port = 7777);
    void disconnect();

    // Message handling
    void sendMessage(const NetworkMessage &message);
    bool receiveMessages();
    NetworkMessage popIncomingMessage();
    bool hasIncomingMessages() const;

    // State queries
    NetworkState getState() const { return currentState; }
    bool isConnected() const;
    bool isHosting() const { return isHost; }
    uint32_t getLocalPlayerID() const { return localPlayerID; }
    uint32_t getRemotePlayerID() const { return remoteConnection.playerID; }

private:
    // Internal network operations
    bool initializeSDLNet();
    void shutdownSDLNet();
    bool handleIncomingConnections();
    void processIncomingMessages(ECS &ecs, GameManager &gameManager);
    void processOutgoingMessages();
    void handleHeartbeat();

    // Message serialization
    bool serializeMessage(const NetworkMessage &message, uint8_t *buffer, size_t &bufferSize);
    bool deserializeMessage(const uint8_t *buffer, size_t bufferSize, NetworkMessage &message);

    // Connection helpers
    void setConnected(bool connected);
    void resetConnection();
    uint32_t generatePlayerID();
};
