#include "InputSystem.h"
#include "NetworkSystem.h"
#include "../components/Components.h"
#include <vector>
#include <cmath>

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

            // Restart based on current game mode
            if (gameManager.currentGameMode == GameManager::DUAL_PLAYER_LOCAL)
            {
                gameManager.startDualPlayerGame();
            }
            else if (gameManager.currentGameMode == GameManager::MULTIPLAYER_ONLINE)
            {
                gameManager.startNetworkedMultiplayerGame();
            }
            else
            {
                gameManager.startGame(); // Single player
            }
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
            // Restart the game based on current game mode
            clearAllMobs(ecs);
            clearAllProjectiles(ecs);

            if (gameManager.currentGameMode == GameManager::DUAL_PLAYER_LOCAL)
            {
                gameManager.startDualPlayerGame();
            }
            else if (gameManager.currentGameMode == GameManager::MULTIPLAYER_ONLINE)
            {
                gameManager.startNetworkedMultiplayerGame();
            }
            else
            {
                gameManager.startGame(); // Single player
            }
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

        // Handle Mob King input - controls depend on game mode
        if (gameManager.isDualPlayer() && !gameManager.isMultiplayer())
        {
            // LOCAL DUAL PLAYER MODE ONLY: IJKL movement, P to shoot
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

                // DUAL PLAYER LOCAL MODE: IJKL movement, P to shoot
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

                // P key to shoot - handle directly in local mode
                if (keyboardState[SDL_SCANCODE_P])
                {
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

        // STEP 1: Only Host processes input, Client does nothing
        if (isHost)
        {
            // Process local input and send to remote
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

                // Send position update to client (throttled to ~30 FPS for performance)
                auto *transform = ecs.getComponent<Transform>(entityID);
                if (transform && networkSystem)
                {
                    static float positionUpdateTimer = 0.0f;
                    positionUpdateTimer += deltaTime;

                    // Send position updates at ~30 FPS (every 0.033 seconds)
                    if (positionUpdateTimer >= 0.033f)
                    {
                        networkSystem->sendEntityPositionUpdate(entityID, transform->x, transform->y,
                                                                velocity->x, velocity->y, "player");
                        std::cout << "[HOST] Sending player position: (" << transform->x << ", " << transform->y << ") @ 30 FPS" << std::endl;
                        positionUpdateTimer = 0.0f;
                    }
                }

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

                // Get mouse state for shooting (handled by WeaponSystem)
                int mouseX, mouseY;
                Uint32 mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
                bool shooting = (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;

                // Mouse state is handled locally by WeaponSystem
                // Position sync is handled by sendEntityPositionUpdate above
                break;
            }

            // HOST does NOT control Mob King - it only receives input from client
        }
        else
        {
            // CLIENT - Handle Mob King input and send to host
            if (gameManager.isDualPlayer())
            {
                auto &mobKingEntities = ecs.getComponents<MobKing>();
                std::cout << "[CLIENT DEBUG] Found " << mobKingEntities.size() << " mob king entities" << std::endl;

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
                    bool shooting = false;

                    // Debug keyboard state
                    bool wPressed = keyboardState[SDL_SCANCODE_W];
                    bool aPressed = keyboardState[SDL_SCANCODE_A];
                    bool sPressed = keyboardState[SDL_SCANCODE_S];
                    bool dPressed = keyboardState[SDL_SCANCODE_D];
                    bool spacePressed = keyboardState[SDL_SCANCODE_SPACE];

                    if (wPressed || aPressed || sPressed || dPressed || spacePressed)
                    {
                        std::cout << "[CLIENT DEBUG] Keys pressed: W=" << wPressed << " A=" << aPressed
                                  << " S=" << sPressed << " D=" << dPressed << " SPACE=" << spacePressed << std::endl;
                    }

                    // MULTIPLAYER MODE: WASD movement, SPACE to shoot
                    if (keyboardState[SDL_SCANCODE_A]) // Left
                    {
                        velocity->x = -1.0f;
                        movingHorizontal = true;
                    }
                    if (keyboardState[SDL_SCANCODE_D]) // Right
                    {
                        velocity->x = 1.0f;
                        movingHorizontal = true;
                    }
                    if (keyboardState[SDL_SCANCODE_W]) // Up
                    {
                        velocity->y = -1.0f;
                        movingVertical = true;
                    }
                    if (keyboardState[SDL_SCANCODE_S]) // Down
                    {
                        velocity->y = 1.0f;
                        movingVertical = true;
                    }

                    // SPACE key to shoot
                    if (keyboardState[SDL_SCANCODE_SPACE])
                    {
                        shooting = true;
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

                    // Apply movement locally on client and send position to host
                    auto *transform = ecs.getComponent<Transform>(entityID);
                    auto *speed = ecs.getComponent<Speed>(entityID);
                    if (transform && speed)
                    {
                        // Apply movement locally (client-side prediction)
                        float speedValue = speed->value;
                        transform->x += velocity->x * speedValue * deltaTime;
                        transform->y += velocity->y * speedValue * deltaTime;
                        
                        // Send position update to host (throttled to reduce network traffic)
                        static float mobKingPositionTimer = 0.0f;
                        mobKingPositionTimer += deltaTime;
                        
                        if (mobKingPositionTimer >= 0.033f) // ~30 FPS
                        {
                            networkSystem->sendEntityPositionUpdate(entityID, transform->x, transform->y, 
                                                                   velocity->x, velocity->y, "mobKing");
                            std::cout << "[CLIENT] Sending Mob King position: (" << transform->x << ", " << transform->y 
                                      << "), vel(" << velocity->x << ", " << velocity->y << ")" << std::endl;
                            mobKingPositionTimer = 0.0f;
                        }
                        
                        // Handle shooting separately
                        if (shooting)
                        {
                            networkSystem->sendMobKingInput(0, 0, shooting); // Only send shooting command
                        }
                    }
                    break;
                }
            }
            // (No debug output to avoid spam)
        }

        // Process incoming network messages
        while (networkSystem->hasIncomingMessages())
        {
            auto message = networkSystem->popIncomingMessage();

            // PLAYER_INPUT message removed - using ENTITY_POSITION_UPDATE instead
            // Client now receives position updates via MovementSystem::updateEntityFromNetwork()

            if (message.type == MessageType::MOB_KING_INPUT && isHost)
            {
                // Host receives mob king input from client (only for shooting now)
                MobKingInputData inputData;
                memcpy(&inputData, message.data, sizeof(MobKingInputData));

                std::cout << "[HOST] Received Mob King shooting input: shoot=" << inputData.shooting << std::endl;

                // Apply shooting only (position is handled via ENTITY_POSITION_UPDATE)
                auto &mobKingEntities = ecs.getComponents<MobKing>();
                for (auto &[entityID, mobKing] : mobKingEntities)
                {
                    // Handle shooting only
                    if (inputData.shooting)
                    {
                        auto *weapon = ecs.getComponent<Weapon>(entityID);
                        if (weapon)
                        {
                            weapon->canFire = true; // Enable shooting
                            std::cout << "[HOST] Mob King shooting enabled!" << std::endl;
                        }
                    }
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
