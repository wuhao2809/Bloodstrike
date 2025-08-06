#include "ProjectileSystem.h"
#include "../components/Components.h"
#include <iostream>

ProjectileSystem::ProjectileSystem()
{
}

ProjectileSystem::~ProjectileSystem()
{
}

void ProjectileSystem::update(ECS &ecs, GameManager &gameManager, float deltaTime)
{
    moveProjectiles(ecs, deltaTime);
    checkProjectileLifetime(ecs, deltaTime);
    handleProjectileCollisions(ecs, gameManager);
}

void ProjectileSystem::moveProjectiles(ECS &ecs, float deltaTime)
{
    // Move all projectiles based on their velocity
    auto &projectileTags = ecs.getComponents<ProjectileTag>();

    for (auto &[entityID, projectileTag] : projectileTags)
    {
        Transform *transform = ecs.getComponent<Transform>(entityID);
        Velocity *velocity = ecs.getComponent<Velocity>(entityID);

        if (transform && velocity)
        {
            // Update position
            transform->x += velocity->x * deltaTime;
            transform->y += velocity->y * deltaTime;
        }
    }
}

void ProjectileSystem::checkProjectileLifetime(ECS &ecs, float deltaTime)
{
    // Check lifetime and remove expired projectiles
    std::vector<EntityID> toRemove;
    auto &projectileTags = ecs.getComponents<ProjectileTag>();

    for (auto &[entityID, projectileTag] : projectileTags)
    {
        Projectile *projectile = ecs.getComponent<Projectile>(entityID);

        if (projectile)
        {
            projectile->timer += deltaTime;

            if (projectile->timer >= projectile->lifetime)
            {
                toRemove.push_back(entityID);
            }
        }
    }

    // Remove expired projectiles
    for (EntityID entityID : toRemove)
    {
        removeProjectile(ecs, entityID);
    }
}

void ProjectileSystem::handleProjectileCollisions(ECS &ecs, GameManager &gameManager)
{
    std::vector<EntityID> projectilesToRemove;
    auto &projectileTags = ecs.getComponents<ProjectileTag>();

    // Check collisions for each projectile
    for (auto &[projID, projectileTag] : projectileTags)
    {
        Transform *projTransform = ecs.getComponent<Transform>(projID);
        Collider *projCollider = ecs.getComponent<Collider>(projID);
        Projectile *projectile = ecs.getComponent<Projectile>(projID);

        if (!projTransform || !projCollider || !projectile)
            continue;

        // Check if projectile owner is a player (player projectiles hit mobs)
        bool isPlayerProjectile = false;
        auto &playerTags = ecs.getComponents<PlayerTag>();
        for (auto &[playerID, playerTag] : playerTags)
        {
            if (playerID == projectile->owner)
            {
                isPlayerProjectile = true;
                break;
            }
        }

        if (isPlayerProjectile)
        {
            // Player projectile - check collision with mobs
            auto &mobTags = ecs.getComponents<MobTag>();
            for (auto &[mobID, mobTag] : mobTags)
            {
                Transform *mobTransform = ecs.getComponent<Transform>(mobID);
                Collider *mobCollider = ecs.getComponent<Collider>(mobID);

                if (!mobTransform || !mobCollider)
                    continue;

                if (checkProjectileCollision(*projTransform, *projCollider, *mobTransform, *mobCollider))
                {
                    std::cout << "Player projectile hit mob!" << std::endl;
                    projectilesToRemove.push_back(projID);
                    ecs.removeEntity(mobID);
                    break;
                }
            }
        }
        else
        {
            // Mob projectile - check collision with player
            auto &playerTags = ecs.getComponents<PlayerTag>();
            for (auto &[playerID, playerTag] : playerTags)
            {
                Transform *playerTransform = ecs.getComponent<Transform>(playerID);
                Collider *playerCollider = ecs.getComponent<Collider>(playerID);

                if (!playerTransform || !playerCollider)
                    continue;

                if (checkProjectileCollision(*projTransform, *projCollider, *playerTransform, *playerCollider))
                {
                    std::cout << "Mob projectile hit player! Game Over!" << std::endl;
                    projectilesToRemove.push_back(projID);
                    gameManager.gameOver(); // Trigger game over
                    break;
                }
            }
        }
    }

    // Remove projectiles that hit something
    for (EntityID entityID : projectilesToRemove)
    {
        removeProjectile(ecs, entityID);
    }
}

void ProjectileSystem::removeProjectile(ECS &ecs, EntityID entityID)
{
    ecs.removeEntity(entityID);
}

bool ProjectileSystem::checkProjectileCollision(const Transform &projTransform, const Collider &projCollider,
                                                const Transform &targetTransform, const Collider &targetCollider)
{
    // Simple AABB collision detection
    float projLeft = projTransform.x - projCollider.width / 2;
    float projRight = projTransform.x + projCollider.width / 2;
    float projTop = projTransform.y - projCollider.height / 2;
    float projBottom = projTransform.y + projCollider.height / 2;

    float targetLeft = targetTransform.x - targetCollider.width / 2;
    float targetRight = targetTransform.x + targetCollider.width / 2;
    float targetTop = targetTransform.y - targetCollider.height / 2;
    float targetBottom = targetTransform.y + targetCollider.height / 2;

    // Check if rectangles overlap
    return (projRight > targetLeft && projLeft < targetRight &&
            projBottom > targetTop && projTop < targetBottom);
}
