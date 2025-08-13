#pragma once
#include "System.h"
#include "../core/ECS.h"
#include "../managers/GameManager.h"
#include "../managers/EntityFactory.h"
#include "../systems/AudioSystem.h"

class WeaponSystem : public System
{
private:
    EntityFactory *entityFactory;
    AudioSystem *audioSystem;
    class NetworkSystem *networkSystem = nullptr; // Forward declaration

public:
    WeaponSystem(EntityFactory *factory, AudioSystem *audio);
    ~WeaponSystem();

    void update(ECS &ecs, GameManager &gameManager, float deltaTime) override;

    // Network synchronization
    void setNetworkSystem(class NetworkSystem *network) { networkSystem = network; }

    // Create projectile from network data (for Client synchronization)
    EntityID createProjectileFromNetwork(ECS &ecs, uint32_t projectileID, uint32_t shooterID, float x, float y, float velocityX, float velocityY, float damage, bool fromPlayer);

private:
    void handlePlayerShooting(ECS &ecs, GameManager &gameManager, float deltaTime);
    void handleMobShooting(ECS &ecs, GameManager &gameManager, float deltaTime);
    void handleMobKingShooting(ECS &ecs, GameManager &gameManager, float deltaTime);
    void handleRegularMobShooting(ECS &ecs, float deltaTime, GameManager &gameManager);
    void ensureMobsHaveWeapons(ECS &ecs, GameManager &gameManager);
    void updateWeaponTimers(ECS &ecs, float deltaTime);
    EntityID createProjectile(ECS &ecs, GameManager &gameManager, float startX, float startY, float dirX, float dirY, const Weapon &weapon, EntityID owner, float projectileSpeed = 500.0f, bool isPlayerProjectile = true);
    bool isMousePressed();
};
