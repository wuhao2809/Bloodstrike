#pragma once
#include "System.h"
#include "../core/ECS.h"
#include "../managers/GameManager.h"
#include "../managers/EntityFactory.h"
#include <random>

// Forward declaration to avoid circular dependency
class NetworkSystem;

class MobSpawningSystem : public System
{
private:
    EntityFactory *entityFactory;
    NetworkSystem *networkSystem = nullptr; // Optional network integration
    float timeSinceLastSpawn;
    float spawnInterval;
    std::mt19937 randomGenerator;
    std::uniform_int_distribution<int> mobTypeDistribution;
    std::uniform_real_distribution<float> positionDistribution;
    std::uniform_real_distribution<float> speedDistribution;

    // Screen dimensions for spawning
    float screenWidth;
    float screenHeight;

    // Mob types
    std::vector<std::string> mobTypes = {"flying", "swimming", "walking"};

public:
    MobSpawningSystem(EntityFactory *factory, float screenW, float screenH);
    void update(ECS &ecs, GameManager &gameManager, float deltaTime) override;
    void reset()
    {
        mobKingSpawned = false;
        timeSinceLastSpawn = 0.0f;
    }

    // Network integration
    void setNetworkSystem(NetworkSystem *network) { networkSystem = network; }

    // Create mob from network data (for Client synchronization)
    EntityID createMobFromNetwork(ECS &ecs, uint32_t mobID, float x, float y, float velocityX, float velocityY, const std::string &mobType);

private:
    void spawnMob(ECS &ecs, GameManager &gameManager);
    void spawnMobKing(ECS &ecs, GameManager &gameManager);
    void setSpawnInterval(float interval) { spawnInterval = interval; }

    // Dual player state
    bool mobKingSpawned = false;
};
