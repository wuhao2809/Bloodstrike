#include "InputSystem.h"
#include "NetworkSystem.h"
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

        // Handle Mob King input (IJKL movement, P to shoot) - only in dual player mode
        if (gameManager.isDualPlayer())
        {
            auto &mobKingEntities = ecs.getComponents<MobKing>();

            for (auto &[entityID, mobKing] : mobKingEntities)
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

                // IJKL movement for Mob King (I=up, J=left, K=down, L=right)
                if (keyboardState[SDL_SCANCODE_J]) // Left
                {
                    velocity->x = -1.0f;
                    movingHorizontal = true;
                }
                if (keyboardState[SDL_SCANCODE_L]) // Right
                {
                    velocity->x = 1.0f;
                    movingHorizontal = true;
                }
                if (keyboardState[SDL_SCANCODE_I]) // Up
                {
                    velocity->y = -1.0f;
                    movingVertical = true;
                }
                if (keyboardState[SDL_SCANCODE_K]) // Down
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

                // P key to shoot (handle in weapon system - just set facing direction)
                // The actual shooting will be handled by the weapon system
                if (keyboardState[SDL_SCANCODE_P])
                {
                    // Set a flag or component to indicate Mob King wants to shoot
                    // This will be picked up by the weapon system
                    auto *weapon = ecs.getComponent<Weapon>(entityID);
                    if (weapon)
                    {
                        weapon->canFire = true; // Enable shooting
                    }
                }
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

void InputSystem::update(ECS &ecs, GameManager &gameManager, NetworkSystem *networkSystem, float deltaTime)
{
    // Handle general inputs (menus, restart, etc.) but not player movement
    if (keyboardState[SDL_SCANCODE_SPACE])
    {
        if (gameManager.currentState == GameManager::GAME_OVER)
        {
            clearAllMobs(ecs);
            gameManager.startGame();
        }
        else if (gameManager.currentState == GameManager::LEVEL_COMPLETE)
        {
            clearAllMobs(ecs);
            clearAllProjectiles(ecs);
            gameManager.continueToNextLevel();
        }
    }

    if (keyboardState[SDL_SCANCODE_R])
    {
        if (gameManager.currentState == GameManager::LEVEL_COMPLETE ||
            gameManager.currentState == GameManager::PLAYING)
        {
            clearAllMobs(ecs);
            clearAllProjectiles(ecs);
            gameManager.startGame();
        }
    }

    // If we're in multiplayer online mode, handle networked input
    if (gameManager.isMultiplayer() && networkSystem && gameManager.currentState == GameManager::PLAYING)
    {
        bool isHost = networkSystem->isHosting();

        // Process local input and send to remote
        if (isHost)
        {
            // Host controls Player (WASD + mouse) locally and sends to client
            auto &playerEntities = ecs.getComponents<PlayerTag>();
            for (auto &[entityID, playerTag] : playerEntities)
            {
                auto *velocity = ecs.getComponent<Velocity>(entityID);
                auto *movementDir = ecs.getComponent<MovementDirection>(entityID);
                if (!velocity)
                    continue;

                float velX = 0.0f, velY = 0.0f;
                bool movingHorizontal = false, movingVertical = false;

                // Process WASD input locally
                if (keyboardState[SDL_SCANCODE_W])
                {
                    velY = -1.0f;
                    movingVertical = true;
                }
                if (keyboardState[SDL_SCANCODE_S])
                {
                    velY = 1.0f;
                    movingVertical = true;
                }
                if (keyboardState[SDL_SCANCODE_A])
                {
                    velX = -1.0f;
                    movingHorizontal = true;
                }
                if (keyboardState[SDL_SCANCODE_D])
                {
                    velX = 1.0f;
                    movingHorizontal = true;
                }

                // Normalize diagonal movement
                if (velX != 0 && velY != 0)
                {
                    velX *= 0.707f;
                    velY *= 0.707f;
                }

                // Apply locally
                velocity->x = velX;
                velocity->y = velY;

                // Set movement direction for sprite
                if (movementDir && (movingHorizontal || movingVertical))
                {
                    if (movingHorizontal && !movingVertical)
                        movementDir->direction = MovementDirection::HORIZONTAL;
                    else if (movingVertical && !movingHorizontal)
                        movementDir->direction = MovementDirection::VERTICAL;
                    else if (movingHorizontal && movingVertical)
                        movementDir->direction = MovementDirection::HORIZONTAL;
                }

                // Get mouse state and send over network
                int mouseX, mouseY;
                Uint32 mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
                bool shooting = (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;

                // Send input to client
                networkSystem->sendPlayerInput(velX, velY, mouseX, mouseY, shooting);
                break;
            }
        }
        else
        {
            // Client controls Mob King (IJKL + P) locally and sends to host
            auto &mobKingEntities = ecs.getComponents<MobKing>();
            for (auto &[entityID, mobKing] : mobKingEntities)
            {
                auto *velocity = ecs.getComponent<Velocity>(entityID);
                if (!velocity)
                    continue;

                float velX = 0.0f, velY = 0.0f;
                bool shooting = false;

                // Process IJKL input locally
                if (keyboardState[SDL_SCANCODE_I])
                    velY = -1.0f;
                if (keyboardState[SDL_SCANCODE_K])
                    velY = 1.0f;
                if (keyboardState[SDL_SCANCODE_J])
                    velX = -1.0f;
                if (keyboardState[SDL_SCANCODE_L])
                    velX = 1.0f;

                // P for shooting
                shooting = keyboardState[SDL_SCANCODE_P];

                // Normalize diagonal movement
                if (velX != 0 && velY != 0)
                {
                    velX *= 0.707f;
                    velY *= 0.707f;
                }

                // Apply locally
                velocity->x = velX;
                velocity->y = velY;

                // Send input to host
                networkSystem->sendMobKingInput(velX, velY, shooting);
                break;
            }
        }

        // Process incoming network messages
        while (networkSystem->hasIncomingMessages())
        {
            auto message = networkSystem->popIncomingMessage();

            if (message.type == MessageType::PLAYER_INPUT && !isHost)
            {
                // Client receives player input from host
                PlayerInputData inputData;
                memcpy(&inputData, message.data, sizeof(PlayerInputData));

                // Apply to local player entity
                auto &playerEntities = ecs.getComponents<PlayerTag>();
                for (auto &[entityID, playerTag] : playerEntities)
                {
                    auto *velocity = ecs.getComponent<Velocity>(entityID);
                    if (velocity)
                    {
                        velocity->x = inputData.velocityX;
                        velocity->y = inputData.velocityY;
                    }
                    // TODO: Handle mouse position and shooting
                    break;
                }
            }
            else if (message.type == MessageType::MOB_KING_INPUT && isHost)
            {
                // Host receives mob king input from client
                MobKingInputData inputData;
                memcpy(&inputData, message.data, sizeof(MobKingInputData));

                // Apply to local mob king entity
                auto &mobKingEntities = ecs.getComponents<MobKing>();
                for (auto &[entityID, mobKing] : mobKingEntities)
                {
                    auto *velocity = ecs.getComponent<Velocity>(entityID);
                    if (velocity)
                    {
                        velocity->x = inputData.velocityX;
                        velocity->y = inputData.velocityY;
                    }
                    // TODO: Handle shooting
                    break;
                }
            }
        }
    }
    else
    {
        // Non-networked mode: call regular update for player movement
        update(ecs, gameManager, deltaTime);
    }
}
