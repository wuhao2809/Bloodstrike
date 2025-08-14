#include "CollisionSystem.h"
#include "../components/Components.h"
#include <iostream>

void CollisionSystem::update(ECS &ecs, GameManager &gameManager, float deltaTime)
{
    // Only check collisions during gameplay
    if (gameManager.currentState != GameManager::PLAYING)
    {
        return;
    }

    // Get all entities with PlayerTag
    auto &playerTags = ecs.getComponents<PlayerTag>();
    auto &mobTags = ecs.getComponents<MobTag>();

    // Check collisions between all players and mobs
    for (auto &[playerEntityID, playerTag] : playerTags)
    {
        // Get player components
        auto *playerTransform = ecs.getComponent<Transform>(playerEntityID);
        auto *playerCollider = ecs.getComponent<Collider>(playerEntityID);

        if (!playerTransform || !playerCollider)
            continue;

        for (auto &[mobEntityID, mobTag] : mobTags)
        {
            // Get mob components
            auto *mobTransform = ecs.getComponent<Transform>(mobEntityID);
            auto *mobCollider = ecs.getComponent<Collider>(mobEntityID);

            if (!mobTransform || !mobCollider)
                continue;

            // Check for collision
            if (checkCollision(*playerTransform, *playerCollider,
                               *mobTransform, *mobCollider))
            {
                handlePlayerMobCollision(ecs, gameManager, playerEntityID, mobEntityID);
                return; // Exit early since game is over
            }
        }
    }
}

bool CollisionSystem::checkCollision(const Transform &pos1, const Collider &col1,
                                     const Transform &pos2, const Collider &col2)
{
    // Calculate bounding boxes
    float left1 = pos1.x - col1.width / 2.0f;
    float right1 = pos1.x + col1.width / 2.0f;
    float top1 = pos1.y - col1.height / 2.0f;
    float bottom1 = pos1.y + col1.height / 2.0f;

    float left2 = pos2.x - col2.width / 2.0f;
    float right2 = pos2.x + col2.width / 2.0f;
    float top2 = pos2.y - col2.height / 2.0f;
    float bottom2 = pos2.y + col2.height / 2.0f;

    // Check for overlap
    return !(left1 > right2 || right1 < left2 || top1 > bottom2 || bottom1 < top2);
}

void CollisionSystem::handlePlayerMobCollision(ECS &ecs, GameManager &gameManager,
                                               EntityID playerEntity, EntityID mobEntity)
{
    if (gameManager.isDualPlayer())
    {
        std::cout << "Player hit by mob! Mob King Wins!" << std::endl;
        gameManager.gameOver(GameManager::MOB_KING);
    }
    else
    {
        std::cout << "Player hit by mob! Game Over!" << std::endl;
        gameManager.gameOver();
    }

    // Play death sound effect
    if (audioSystem)
    {
        audioSystem->playSound("gameover");
    }

    // Optional: Remove the mob entity that caused the collision
    ecs.removeEntity(mobEntity);
}
