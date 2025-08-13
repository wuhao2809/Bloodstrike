#pragma once
#include "System.h"
#include "../managers/GameManager.h"
#include "../components/Components.h"
#include <SDL2/SDL_net.h>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>

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
    PLAYER_READY,
    LOBBY_STATUS,

    // Gameplay
    PLAYER_INPUT,
    MOB_KING_INPUT,
    GAME_STATE_UPDATE,
    ENTITY_POSITION_UPDATE, // New: for syncing entity positions
    MOB_SPAWN,
    PROJECTILE_CREATE,
    PROJECTILE_HIT,
    ENTITY_REMOVE, // New: for removing entities (projectiles, mobs)
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

// Input message structures
struct PlayerInputData
{
    float velocityX, velocityY;
    int mouseX, mouseY;
    bool shooting;
    uint32_t timestamp;
};

struct MobKingInputData
{
    float velocityX, velocityY;
    bool shooting;
    uint32_t timestamp;
};

// Lobby message structures
struct PlayerReadyData
{
    uint32_t playerID;
    bool isReady;
    char playerName[32];
    uint32_t timestamp;
};

struct LobbyStatusData
{
    bool hostReady;
    bool clientReady;
    bool gameStarting;
    uint32_t countdown; // Seconds until game starts
    uint32_t timestamp;
};

// Game state synchronization structures
// Optimized game state structure (Step 2 Complete)
struct GameStateData
{
    uint32_t score;             // Only authoritative data from host
    float mobKingCurrentHealth; // For client UI display
    float mobKingMaxHealth;     // For client UI display
    uint32_t gameStartTime;     // For local time calculation
    uint32_t timestamp;         // For sync verification
    // Removed: gameTime, levelTime, levelDuration, currentLevel (calculated locally)
};

struct MobSpawnData
{
    uint32_t mobID;
    float x, y;
    float velocityX, velocityY;
    char mobType[32]; // Fixed-size string for network transmission
    uint32_t timestamp;
};

struct ProjectileData
{
    uint32_t projectileID;
    uint32_t shooterID; // Entity ID of shooter
    float x, y;
    float velocityX, velocityY;
    float damage;
    bool fromPlayer; // true if from player, false if from mob
    uint32_t timestamp;
};

struct ProjectileHitData
{
    uint32_t projectileID;
    uint32_t targetID; // Entity ID that was hit
    float damage;
    bool destroyed; // true if target was destroyed
    uint32_t timestamp;
};

struct EntityPositionData
{
    uint32_t entityID;
    float x, y;                 // Position
    float velocityX, velocityY; // Velocity
    char entityType[16];        // "player" or "mobKing"
    uint32_t timestamp;
};

struct EntityRemoveData
{
    uint32_t entityID;
    char entityType[16]; // "projectile", "mob", "mobKing"
    uint32_t timestamp;
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

    // Entity ID mapping for network synchronization
    std::unordered_map<uint32_t, EntityID> networkToLocalEntityMap; // networkID -> localEntityID
    std::unordered_map<EntityID, uint32_t> localToNetworkEntityMap; // localEntityID -> networkID

    // Configuration
    std::string hostIP;
    uint16_t port;
    bool debugMode;

    // System references
    ECS *ecs = nullptr;
    GameManager *gameManager = nullptr;
    class MobSpawningSystem *mobSpawningSystem = nullptr;
    class WeaponSystem *weaponSystem = nullptr;
    class MovementSystem *movementSystem = nullptr;

    // Connection management
    uint32_t connectionTimeout;
    uint32_t lastHeartbeat;

    // Lobby state
    bool localPlayerReady = false;
    bool remotePlayerReady = false;
    bool gameStartCountdown = false;
    uint32_t countdownStartTime = 0;
    uint32_t gameStartTime = 0; // Step 2: Track when the actual game began

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

    // Input synchronization
    void sendPlayerInput(float velocityX, float velocityY, int mouseX, int mouseY, bool shooting);
    void sendMobKingInput(float velocityX, float velocityY, bool shooting);

    // Lobby management
    void setPlayerReady(bool ready);
    void sendLobbyStatus();
    bool isLocalPlayerReady() const { return localPlayerReady; }
    bool isRemotePlayerReady() const { return remotePlayerReady; }
    bool areBothPlayersReady() const { return localPlayerReady && remotePlayerReady; }
    bool isGameStarting() const { return gameStartCountdown; }

    // Game state synchronization
    void sendGameStateUpdate(ECS &ecs, uint32_t score, uint32_t gameStartTime); // Step 2: Optimized version
    void sendEntityPositionUpdate(uint32_t entityID, float x, float y, float velocityX, float velocityY, const std::string &entityType);
    void sendMobSpawn(uint32_t mobID, float x, float y, float velocityX, float velocityY, const std::string &mobType);
    void sendProjectileCreate(uint32_t projectileID, uint32_t shooterID, float x, float y, float velocityX, float velocityY, float damage, bool fromPlayer);
    void sendProjectileHit(uint32_t projectileID, uint32_t targetID, float damage, bool destroyed);
    void sendEntityRemove(uint32_t entityID, const std::string &entityType); // Step 3: Replace PROJECTILE_HIT with entity removal
    void sendGameStart();
    void sendGameOver();

    // Entity ID mapping for network synchronization
    void registerNetworkEntity(uint32_t networkID, EntityID localID);
    EntityID getLocalEntityID(uint32_t networkID);
    uint32_t getNetworkEntityID(EntityID localID);
    void unregisterNetworkEntity(uint32_t networkID);

    // System references for synchronization
    void setGameManager(GameManager *manager) { gameManager = manager; }
    void setMobSpawningSystem(class MobSpawningSystem *system) { mobSpawningSystem = system; }
    void setWeaponSystem(class WeaponSystem *system) { weaponSystem = system; }
    void setMovementSystem(class MovementSystem *system) { movementSystem = system; }

    // State queries
    NetworkState getState() const { return currentState; }
    bool isConnected() const;
    bool isHosting() const { return isHost; }
    uint32_t getLocalPlayerID() const { return localPlayerID; }
    uint32_t getRemotePlayerID() const { return remoteConnection.playerID; }

    // Debug control
    void setDebugMode(bool enable) { debugMode = enable; }
    bool isDebugMode() const { return debugMode; }

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
