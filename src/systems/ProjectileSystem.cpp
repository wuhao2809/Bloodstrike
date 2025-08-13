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
                    // Check if this mob has health (like Mob King)
                    Health *mobHealth = ecs.getComponent<Health>(mobID);
                    if (mobHealth)
                    {
                        // Damage the mob's health
                        mobHealth->currentHealth -= projectile->damage;
                        std::cout << "Player projectile hit mob! Damage: " << projectile->damage
                                  << ", Health remaining: " << mobHealth->currentHealth << std::endl;

                        // Update health UI if this is the Mob King
                        if (ecs.getComponent<MobKing>(mobID))
                        {
                            updateMobKingHealthUI(ecs, mobID, *mobHealth);
                        }

                        // Remove the mob if health drops to 0 or below
                        if (mobHealth->currentHealth <= 0)
                        {
                            // Check if this is the Mob King
                            if (ecs.getComponent<MobKing>(mobID))
                            {
                                std::cout << "Mob King defeated! Victory!" << std::endl;
                                // Remove the health UI when Mob King is defeated
                                removeMobKingHealthUI(ecs, mobID);
                            }
                            ecs.removeEntity(mobID);
                        }
                    }
                    else
                    {
                        // Regular mob without health - remove immediately
                        std::cout << "Player projectile hit mob!" << std::endl;
                        ecs.removeEntity(mobID);
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

void ProjectileSystem::updateMobKingHealthUI(ECS &ecs, EntityID mobKingEntity, const Health &health)
{
    // Find the health UI entity for this Mob King
    auto &healthUIComponents = ecs.getComponents<MobKingHealthUI>();
    
    for (auto &[uiEntityID, healthUI] : healthUIComponents)
    {
        if (healthUI.mobKingEntity == mobKingEntity)
        {
            // Update the UI text
            UIText *uiText = ecs.getComponent<UIText>(uiEntityID);
            if (uiText)
            {
                std::string healthText = "Mob King: " + std::to_string((int)health.currentHealth) + "/" + std::to_string((int)health.maxHealth);
                uiText->content = healthText;
                
                // Change color based on health percentage
                float healthPercent = health.currentHealth / health.maxHealth;
                if (healthPercent > 0.6f)
                {
                    uiText->color = {255, 255, 255, 255}; // White
                }
                else if (healthPercent > 0.3f)
                {
                    uiText->color = {255, 255, 0, 255}; // Yellow
                }
                else
                {
                    uiText->color = {255, 0, 0, 255}; // Red
                }
                
                std::cout << "Updated Mob King health UI: " << healthText << std::endl;
            }
            break;
        }
    }
}

void ProjectileSystem::removeMobKingHealthUI(ECS &ecs, EntityID mobKingEntity)
{
    // Find and remove the health UI entity for this Mob King
    auto &healthUIComponents = ecs.getComponents<MobKingHealthUI>();
    std::vector<EntityID> uiToRemove;
    
    for (auto &[uiEntityID, healthUI] : healthUIComponents)
    {
        if (healthUI.mobKingEntity == mobKingEntity)
        {
            uiToRemove.push_back(uiEntityID);
        }
    }
    
    for (EntityID uiEntityID : uiToRemove)
    {
        ecs.removeEntity(uiEntityID);
        std::cout << "Removed Mob King health UI" << std::endl;
    }
}
