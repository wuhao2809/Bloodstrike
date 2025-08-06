#pragma once
#include "System.h"
#include "../core/ECS.h"
#include "../managers/GameManager.h"
#include "../managers/EntityFactory.h"
#include <random>

class MobSpawningSystem : public System
{
private:
    EntityFactory *entityFactory;
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

private:
    void spawnMob(ECS &ecs);
    void setSpawnInterval(float interval) { spawnInterval = interval; }
};
