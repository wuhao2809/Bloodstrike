#include "MovementSystem.h"
#include "../components/Components.h"
#include <iostream>

void MovementSystem::update(ECS &ecs, float deltaTime)
{
    auto &transforms = ecs.getComponents<Transform>();
    static float debugTimer = 0.0f;
    debugTimer += deltaTime;

    for (auto &[entityID, transform] : transforms)
    {
        auto *velocity = ecs.getComponent<Velocity>(entityID);
        auto *speed = ecs.getComponent<Speed>(entityID);

        if (velocity && speed)
        {
            // Apply velocity * speed * deltaTime to position
            transform.x += velocity->x * speed->value * deltaTime;
            transform.y += velocity->y * speed->value * deltaTime;
        }
    }

    // Debug entity positions every 2 seconds
    if (debugTimer >= 2.0f)
    {
        debugTimer = 0.0f;

        // Show player position
        auto &playerComponents = ecs.getComponents<PlayerTag>();
        for (auto &[entityID, player] : playerComponents)
        {
            auto *transform = ecs.getComponent<Transform>(entityID);
            if (transform)
            {
                std::cout << "[DEBUG] Player position: (" << transform->x << ", " << transform->y << ")" << std::endl;
            }
        }

        // Show mob king position
        auto &mobKingComponents = ecs.getComponents<MobKing>();
        for (auto &[entityID, mobKing] : mobKingComponents)
        {
            auto *transform = ecs.getComponent<Transform>(entityID);
            if (transform)
            {
                std::cout << "[DEBUG] Mob King position: (" << transform->x << ", " << transform->y << ")" << std::endl;
            }
        }
    }
}

void MovementSystem::updateEntityFromNetwork(ECS &ecs, uint32_t networkEntityID, float x, float y, float velocityX, float velocityY, const std::string &entityType)
{
    // Find the entity by type (since we can't directly map network ID to ECS entity ID)
    auto &transforms = ecs.getComponents<Transform>();

    EntityID targetEntity = 0;
    bool entityFound = false;

    if (entityType == "player")
    {
        // Find player entity
        auto &playerComponents = ecs.getComponents<PlayerTag>();
        for (auto &[entityID, player] : playerComponents)
        {
            targetEntity = entityID;
            entityFound = true;
            break;
        }
    }
    else if (entityType == "mobKing")
    {
        // Find mob king entity
        auto &mobKingComponents = ecs.getComponents<MobKing>();
        for (auto &[entityID, mobKing] : mobKingComponents)
        {
            targetEntity = entityID;
            entityFound = true;
            break;
        }
    }

    if (entityFound)
    {
        // Update position
        auto *transform = ecs.getComponent<Transform>(targetEntity);
        if (transform)
        {
            transform->x = x;
            transform->y = y;
            std::cout << "[CLIENT] Updated " << entityType << " position to (" << x << ", " << y << ")" << std::endl;
        }

        // Update velocity
        auto *velocity = ecs.getComponent<Velocity>(targetEntity);
        if (velocity)
        {
            velocity->x = velocityX;
            velocity->y = velocityY;
        }
    }
    else
    {
        std::cout << "[CLIENT] Warning: Could not find " << entityType << " entity to update" << std::endl;
    }
}
