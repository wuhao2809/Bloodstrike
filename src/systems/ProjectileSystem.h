#pragma once
#include "System.h"
#include "../core/ECS.h"
#include "../managers/GameManager.h"
#include "../components/Components.h"

class ProjectileSystem : public System
{
public:
    ProjectileSystem();
    ~ProjectileSystem();

    void update(ECS &ecs, GameManager &gameManager, float deltaTime) override;

private:
    void moveProjectiles(ECS &ecs, float deltaTime);
    void checkProjectileLifetime(ECS &ecs, float deltaTime);
    void handleProjectileCollisions(ECS &ecs, GameManager &gameManager);
    void removeProjectile(ECS &ecs, EntityID entityID);
    bool checkProjectileCollision(const Transform &projTransform, const Collider &projCollider,
                                  const Transform &targetTransform, const Collider &targetCollider);
};
