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

public:
    WeaponSystem(EntityFactory *factory, AudioSystem *audio);
    ~WeaponSystem();

    void update(ECS &ecs, GameManager &gameManager, float deltaTime) override;

private:
    void handlePlayerShooting(ECS &ecs, float deltaTime);
    void handleMobShooting(ECS &ecs, GameManager &gameManager, float deltaTime);
    void handleMobKingShooting(ECS &ecs, float deltaTime);
    void handleRegularMobShooting(ECS &ecs, float deltaTime);
    void updateWeaponTimers(ECS &ecs, float deltaTime);
    EntityID createProjectile(ECS &ecs, float startX, float startY, float dirX, float dirY, const Weapon &weapon, EntityID owner, float projectileSpeed = 500.0f, bool isPlayerProjectile = true);
    bool isMousePressed();
};
