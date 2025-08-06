#pragma once
#include "System.h"
#include "../core/ECS.h"
#include "../managers/GameManager.h"

class ProjectileSystem : public System
{
public:
    ProjectileSystem();
    ~ProjectileSystem();

    void update(ECS &ecs, GameManager &gameManager, float deltaTime) override;

private:
    void moveProjectiles(ECS &ecs, float deltaTime);
    void checkProjectileLifetime(ECS &ecs, float deltaTime);
    void handleProjectileCollisions(ECS &ecs);
    void removeProjectile(ECS &ecs, EntityID entityID);
};
