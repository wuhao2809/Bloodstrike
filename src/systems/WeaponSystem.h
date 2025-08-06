#pragma once
#include "System.h"
#include "../core/ECS.h"
#include "../managers/GameManager.h"
#include "../managers/EntityFactory.h"

class WeaponSystem : public System
{
private:
    EntityFactory *entityFactory;

public:
    WeaponSystem(EntityFactory *factory);
    ~WeaponSystem();

    void update(ECS &ecs, GameManager &gameManager, float deltaTime) override;

private:
    void handleShooting(ECS &ecs, float deltaTime);
    void updateWeaponTimers(ECS &ecs, float deltaTime);
    EntityID createProjectile(ECS &ecs, float startX, float startY, float dirX, float dirY, const Weapon &weapon, EntityID owner);
    bool isMousePressed();
};
