#include "BoundarySystem.h"
#include "../components/Components.h"
#include <iostream>

void BoundarySystem::update(ECS &ecs, GameManager &gameManager, float deltaTime)
{
    // Always keep player in bounds
    keepPlayerInBounds(ecs);

    // Remove off-screen mobs during gameplay
    if (gameManager.currentState == GameManager::PLAYING)
    {
        removeOffScreenMobs(ecs);
    }
}

void BoundarySystem::keepPlayerInBounds(ECS &ecs)
{
    auto &playerTags = ecs.getComponents<PlayerTag>();

    for (auto &[entityID, playerTag] : playerTags)
    {
        auto *transform = ecs.getComponent<Transform>(entityID);
        auto *sprite = ecs.getComponent<Sprite>(entityID);

        if (!transform || !sprite)
            continue;

        // Calculate boundaries considering sprite size
        float halfWidth = sprite->width / 2.0f;
        float halfHeight = sprite->height / 2.0f;

        // Clamp position to screen bounds
        if (transform->x - halfWidth < 0)
        {
            transform->x = halfWidth;
        }
        else if (transform->x + halfWidth > screenWidth)
        {
            transform->x = screenWidth - halfWidth;
        }

        if (transform->y - halfHeight < 0)
        {
            transform->y = halfHeight;
        }
        else if (transform->y + halfHeight > screenHeight)
        {
            transform->y = screenHeight - halfHeight;
        }
    }
}

void BoundarySystem::removeOffScreenMobs(ECS &ecs)
{
    auto &mobTags = ecs.getComponents<MobTag>();
    std::vector<EntityID> mobsToRemove;

    for (auto &[entityID, mobTag] : mobTags)
    {
        auto *transform = ecs.getComponent<Transform>(entityID);
        auto *sprite = ecs.getComponent<Sprite>(entityID);

        if (!transform || !sprite)
            continue;

        // Check if mob is completely off-screen to the left
        float rightEdge = transform->x + sprite->width / 2.0f;
        if (rightEdge < -50.0f) // Give some buffer
        {
            mobsToRemove.push_back(entityID);
        }
    }

    // Remove off-screen mobs
    for (EntityID mobID : mobsToRemove)
    {
        std::cout << "Removing off-screen mob: " << mobID << std::endl;
        ecs.removeEntity(mobID);
    }
}
