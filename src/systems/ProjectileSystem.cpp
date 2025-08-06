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
    handleProjectileCollisions(ecs);
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

void ProjectileSystem::handleProjectileCollisions(ECS &ecs)
{
    std::vector<EntityID> projectilesToRemove;
    auto &projectileTags = ecs.getComponents<ProjectileTag>();

    // Check collisions between projectiles and mobs
    for (auto &[projID, projectileTag] : projectileTags)
    {
        Transform *projTransform = ecs.getComponent<Transform>(projID);
        Collider *projCollider = ecs.getComponent<Collider>(projID);

        if (!projTransform || !projCollider)
            continue;

        auto &mobTags = ecs.getComponents<MobTag>();

        for (auto &[mobID, mobTag] : mobTags)
        {
            Transform *mobTransform = ecs.getComponent<Transform>(mobID);
            Collider *mobCollider = ecs.getComponent<Collider>(mobID);

            if (!mobTransform || !mobCollider)
                continue;

            // Simple AABB collision detection
            float projLeft = projTransform->x - projCollider->width / 2;
            float projRight = projTransform->x + projCollider->width / 2;
            float projTop = projTransform->y - projCollider->height / 2;
            float projBottom = projTransform->y + projCollider->height / 2;

            float mobLeft = mobTransform->x - mobCollider->width / 2;
            float mobRight = mobTransform->x + mobCollider->width / 2;
            float mobTop = mobTransform->y - mobCollider->height / 2;
            float mobBottom = mobTransform->y + mobCollider->height / 2;

            // Check if rectangles overlap
            if (projRight > mobLeft && projLeft < mobRight &&
                projBottom > mobTop && projTop < mobBottom)
            {
                // Collision detected!
                std::cout << "Projectile hit mob!" << std::endl;

                // Mark projectile for removal
                projectilesToRemove.push_back(projID);

                // For now, just remove the mob (later we can add health system)
                ecs.removeEntity(mobID);
                break; // This projectile is done
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
