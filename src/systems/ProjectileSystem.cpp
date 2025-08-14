#include "ProjectileSystem.h"
#include "NetworkSystem.h"
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

    // Step 3: Only host handles projectile lifetime and collisions in multiplayer
    if (!gameManager.isMultiplayer() || !networkSystem || networkSystem->isHosting())
    {
        checkProjectileLifetime(ecs, deltaTime);
        handleProjectileCollisions(ecs, gameManager);
    }
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
        // Step 3: Send entity removal for expired projectiles (host only)
        if (networkSystem && networkSystem->isHosting())
        {
            uint32_t projNetworkID = networkSystem->getNetworkEntityID(entityID);
            if (projNetworkID != 0)
            {
                std::cout << "[HOST] Sending ENTITY_REMOVE for expired projectile, network ID: " << projNetworkID << std::endl;
                networkSystem->sendEntityRemove(projNetworkID, "projectile");
            }
        }

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
                    bool mobDestroyed = false;

                    // Step 3: Get network IDs BEFORE removing entities
                    uint32_t projNetworkID = 0;
                    uint32_t mobNetworkID = 0;
                    if (networkSystem && gameManager.isMultiplayer())
                    {
                        projNetworkID = networkSystem->getNetworkEntityID(projID);
                        mobNetworkID = networkSystem->getNetworkEntityID(mobID);
                    }

                    // Check if this mob has health (like Mob King)
                    Health *mobHealth = ecs.getComponent<Health>(mobID);
                    if (mobHealth)
                    {
                        // Damage the mob's health
                        mobHealth->currentHealth -= projectile->damage;
                        std::cout << "Player projectile hit mob! Damage: " << projectile->damage
                                  << ", Health remaining: " << mobHealth->currentHealth << std::endl;

                        // Remove the mob if health drops to 0 or below
                        if (mobHealth->currentHealth <= 0)
                        {
                            mobDestroyed = true;
                            // Check if this is the Mob King
                            if (ecs.getComponent<MobKing>(mobID))
                            {
                                if (gameManager.isDualPlayer())
                                {
                                    std::cout << "Mob King defeated! Player Wins!" << std::endl;
                                    gameManager.gameOver(GameManager::PLAYER);
                                }
                                else
                                {
                                    std::cout << "Mob King defeated! Victory!" << std::endl;
                                }
                            }
                            ecs.removeEntity(mobID);
                        }
                    }
                    else
                    {
                        // Regular mob without health - remove immediately
                        mobDestroyed = true;
                        std::cout << "Player projectile hit mob!" << std::endl;
                        ecs.removeEntity(mobID);
                    }

                    // Send entity removal messages for collision results
                    if (networkSystem && gameManager.isMultiplayer())
                    {
                        // Send removal messages with network IDs
                        if (projNetworkID != 0)
                        {
                            std::cout << "[HOST] Sending ENTITY_REMOVE for projectile, network ID: " << projNetworkID << std::endl;
                            networkSystem->sendEntityRemove(projNetworkID, "projectile");
                        }

                        if (mobDestroyed && mobNetworkID != 0)
                        {
                            std::string mobType = ecs.getComponent<MobKing>(mobID) ? "mobKing" : "mob";
                            std::cout << "[HOST] Sending ENTITY_REMOVE for " << mobType << ", network ID: " << mobNetworkID << std::endl;
                            networkSystem->sendEntityRemove(mobNetworkID, mobType);
                        }
                    }

                    projectilesToRemove.push_back(projID);
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
                    if (gameManager.isDualPlayer())
                    {
                        std::cout << "Mob projectile hit player! Mob King Wins!" << std::endl;
                        gameManager.gameOver(GameManager::MOB_KING);
                    }
                    else
                    {
                        std::cout << "Mob projectile hit player! Game Over!" << std::endl;
                        gameManager.gameOver();
                    }

                    // Step 3: Send entity removal message for projectile
                    if (networkSystem && gameManager.isMultiplayer())
                    {
                        uint32_t projNetworkID = networkSystem->getNetworkEntityID(projID);
                        if (projNetworkID != 0)
                        {
                            std::cout << "[HOST] Sending ENTITY_REMOVE for mob projectile, network ID: " << projNetworkID << std::endl;
                            networkSystem->sendEntityRemove(projNetworkID, "projectile");
                        }
                    }

                    projectilesToRemove.push_back(projID);
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
