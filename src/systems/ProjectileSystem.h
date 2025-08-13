#pragma once
#include "System.h"
#include "../core/ECS.h"
#include "../managers/GameManager.h"
#include "../components/Components.h"

class ProjectileSystem : public System
{
private:
    class NetworkSystem *networkSystem = nullptr; // Forward declaration

public:
    ProjectileSystem();
    ~ProjectileSystem();

    void update(ECS &ecs, GameManager &gameManager, float deltaTime) override;

    // Network synchronization
    void setNetworkSystem(class NetworkSystem *network) { networkSystem = network; }

private:
    void moveProjectiles(ECS &ecs, float deltaTime);
    void checkProjectileLifetime(ECS &ecs, float deltaTime);
    void handleProjectileCollisions(ECS &ecs, GameManager &gameManager);
    void removeProjectile(ECS &ecs, EntityID entityID);
    bool checkProjectileCollision(const Transform &projTransform, const Collider &projCollider,
                                  const Transform &targetTransform, const Collider &targetCollider);
};
