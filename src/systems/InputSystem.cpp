#include "InputSystem.h"
#include "../components/Components.h"
#include <vector>

InputSystem::InputSystem()
{
    keyboardState = SDL_GetKeyboardState(nullptr);
}

void InputSystem::update(ECS &ecs, GameManager &gameManager, float deltaTime)
{
    // Handle game state inputs (only for non-menu states)
    if (keyboardState[SDL_SCANCODE_SPACE])
    {
        if (gameManager.currentState == GameManager::GAME_OVER)
        {
            // Clear all existing mobs before starting new game
            clearAllMobs(ecs);
            gameManager.startGame();
        }
        else if (gameManager.currentState == GameManager::LEVEL_COMPLETE)
        {
            // Continue to next level
            clearAllMobs(ecs);
            clearAllProjectiles(ecs);
            gameManager.continueToNextLevel();
        }
    }

    // Handle restart input (R key)
    if (keyboardState[SDL_SCANCODE_R])
    {
        if (gameManager.currentState == GameManager::LEVEL_COMPLETE ||
            gameManager.currentState == GameManager::PLAYING)
        {
            // Restart the game
            clearAllMobs(ecs);
            clearAllProjectiles(ecs);
            gameManager.startGame();
        }
    }

    // Handle player movement (only during gameplay)
    if (gameManager.currentState == GameManager::PLAYING)
    {
        auto &playerEntities = ecs.getComponents<PlayerTag>();

        for (auto &[entityID, playerTag] : playerEntities)
        {
            auto *velocity = ecs.getComponent<Velocity>(entityID);
            auto *movementDir = ecs.getComponent<MovementDirection>(entityID);
            if (!velocity)
                continue;

            // Reset velocity
            velocity->x = 0;
            velocity->y = 0;

            // Track which directions are pressed
            bool movingHorizontal = false;
            bool movingVertical = false;

            // Update velocity based on input
            if (keyboardState[SDL_SCANCODE_LEFT] || keyboardState[SDL_SCANCODE_A])
            {
                velocity->x = -1.0f;
                movingHorizontal = true;
            }
            if (keyboardState[SDL_SCANCODE_RIGHT] || keyboardState[SDL_SCANCODE_D])
            {
                velocity->x = 1.0f;
                movingHorizontal = true;
            }
            if (keyboardState[SDL_SCANCODE_UP] || keyboardState[SDL_SCANCODE_W])
            {
                velocity->y = -1.0f;
                movingVertical = true;
            }
            if (keyboardState[SDL_SCANCODE_DOWN] || keyboardState[SDL_SCANCODE_S])
            {
                velocity->y = 1.0f;
                movingVertical = true;
            }

            // Set movement direction for sprite selection
            if (movementDir && (movingHorizontal || movingVertical))
            {
                if (movingHorizontal && !movingVertical)
                {
                    movementDir->direction = MovementDirection::HORIZONTAL;
                }
                else if (movingVertical && !movingHorizontal)
                {
                    movementDir->direction = MovementDirection::VERTICAL;
                }
                // For diagonal movement, prioritize horizontal sprite
                else if (movingHorizontal && movingVertical)
                {
                    movementDir->direction = MovementDirection::HORIZONTAL;
                }
            }

            // Normalize diagonal movement
            if (velocity->x != 0 && velocity->y != 0)
            {
                velocity->x *= 0.707f; // 1/sqrt(2)
                velocity->y *= 0.707f;
            }
        }
    }
}

void InputSystem::clearAllMobs(ECS &ecs)
{
    // Get all entities with MobTag and remove them
    auto &mobTags = ecs.getComponents<MobTag>();
    std::vector<EntityID> mobsToRemove;

    // Collect all mob entity IDs
    for (auto &[entityID, mobTag] : mobTags)
    {
        mobsToRemove.push_back(entityID);
    }

    // Remove all mob entities
    for (EntityID mobID : mobsToRemove)
    {
        ecs.removeEntity(mobID);
    }
}

void InputSystem::clearAllProjectiles(ECS &ecs)
{
    // Get all entities with ProjectileTag and remove them
    auto &projectileTags = ecs.getComponents<ProjectileTag>();
    std::vector<EntityID> projectilesToRemove;

    // Collect all projectile entity IDs
    for (auto &[entityID, projectileTag] : projectileTags)
    {
        projectilesToRemove.push_back(entityID);
    }

    // Remove all projectile entities
    for (EntityID projectileID : projectilesToRemove)
    {
        ecs.removeEntity(projectileID);
    }
}
