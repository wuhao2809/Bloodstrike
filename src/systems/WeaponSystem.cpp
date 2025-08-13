#include "WeaponSystem.h"
#include "NetworkSystem.h"
#include "../components/Components.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>

WeaponSystem::WeaponSystem(EntityFactory *factory, AudioSystem *audio) : entityFactory(factory), audioSystem(audio)
{
}

WeaponSystem::~WeaponSystem()
{
}

void WeaponSystem::update(ECS &ecs, GameManager &gameManager, float deltaTime)
{
    updateWeaponTimers(ecs, deltaTime);

    // In multiplayer mode, shooting logic depends on role:
    // - Host: Handles all shooting (player + mobs) based on inputs
    // - Client: Only handles player input (which gets sent to Host via InputSystem)
    if (gameManager.isMultiplayer() && networkSystem)
    {
        if (networkSystem->isHosting())
        {
            // Host handles all shooting
            handlePlayerShooting(ecs, gameManager, deltaTime);
            handleMobShooting(ecs, gameManager, deltaTime);
        }
        else
        {
            // Client: Only handle player shooting (input will be networked)
            handlePlayerShooting(ecs, gameManager, deltaTime);
            // Mobs are controlled by Host, don't handle mob shooting on Client
        }
    }
    else
    {
        // Single player or dual player: handle all shooting locally
        handlePlayerShooting(ecs, gameManager, deltaTime);
        handleMobShooting(ecs, gameManager, deltaTime);
    }
}

void WeaponSystem::updateWeaponTimers(ECS &ecs, float deltaTime)
{
    // Update fire timers for all weapons
    auto &weapons = ecs.getComponents<Weapon>();

    for (auto &[entityID, weapon] : weapons)
    {
        if (weapon.fireTimer > 0.0f)
        {
            weapon.fireTimer -= deltaTime;
            if (weapon.fireTimer <= 0.0f)
            {
                weapon.canFire = true;
            }
        }
    }
}

void WeaponSystem::handlePlayerShooting(ECS &ecs, GameManager &gameManager, float deltaTime)
{
    if (!isMousePressed())
        return;

    // Find player entities with weapon
    auto &playerTags = ecs.getComponents<PlayerTag>();

    for (auto &[entityID, playerTag] : playerTags)
    {
        Weapon *weapon = ecs.getComponent<Weapon>(entityID);
        Transform *transform = ecs.getComponent<Transform>(entityID);
        MouseTarget *mouseTarget = ecs.getComponent<MouseTarget>(entityID);

        if (!weapon || !transform || !mouseTarget)
            continue;
        if (!weapon->canFire || weapon->ammoCount <= 0)
            continue;

        // Calculate direction to mouse
        float dirX = mouseTarget->x - transform->x;
        float dirY = mouseTarget->y - transform->y;

        // Normalize direction
        float length = std::sqrt(dirX * dirX + dirY * dirY);
        if (length > 0.0f)
        {
            dirX /= length;
            dirY /= length;
        }

        // Create projectile
        EntityID projectileEntity = createProjectile(ecs, gameManager, transform->x, transform->y,
                                                     dirX, dirY, *weapon, entityID, 500.0f, true);

        // Play gun shot sound
        if (audioSystem)
        {
            audioSystem->playSound("gunshot");
        }

        // Update weapon state
        weapon->ammoCount--;
        weapon->fireTimer = 1.0f / weapon->fireRate; // Time between shots
        weapon->canFire = false;

        std::cout << "Player fired! Ammo remaining: " << weapon->ammoCount << std::endl;

        break; // Only one player can shoot
    }
}

EntityID WeaponSystem::createProjectile(ECS &ecs, GameManager &gameManager, float startX, float startY,
                                        float dirX, float dirY, const Weapon &weapon, EntityID owner, float projectileSpeed, bool isPlayerProjectile)
{
    EntityID projectileEntity = ecs.createEntity();

    // Add components to projectile
    ecs.addComponent(projectileEntity, Transform(startX, startY, 0.0f));

    // Add velocity (direction * speed)
    ecs.addComponent(projectileEntity, Velocity(dirX * projectileSpeed, dirY * projectileSpeed));

    // Add projectile component with damage and lifetime
    ecs.addComponent(projectileEntity, Projectile(projectileSpeed, weapon.damage, 3.0f, owner, dirX, dirY));

    // Add projectile tag for identification
    ecs.addComponent(projectileEntity, ProjectileTag{});

    if (isPlayerProjectile)
    {
        // Player projectiles: small, white/yellow
        ecs.addComponent(projectileEntity, Sprite(nullptr, 4, 4, 1, 0.0f));
        ecs.addComponent(projectileEntity, ProjectileColor(SDL_Color{255, 255, 0, 255})); // Yellow
        ecs.addComponent(projectileEntity, Collider(4.0f, 4.0f, false));
    }
    else
    {
        // Mob projectiles: larger, red
        ecs.addComponent(projectileEntity, Sprite(nullptr, 8, 8, 1, 0.0f));
        ecs.addComponent(projectileEntity, ProjectileColor(SDL_Color{255, 0, 0, 255})); // Red
        ecs.addComponent(projectileEntity, Collider(8.0f, 8.0f, false));
    }

    // Send projectile data over network for multiplayer synchronization
    if (networkSystem && gameManager.isMultiplayer())
    {
        networkSystem->sendProjectileCreate(projectileEntity, owner, startX, startY,
                                            dirX * projectileSpeed, dirY * projectileSpeed,
                                            weapon.damage, isPlayerProjectile);
    }

    return projectileEntity;
}

void WeaponSystem::handleMobShooting(ECS &ecs, GameManager &gameManager, float deltaTime)
{
    // Handle different shooting behaviors based on game mode
    if (gameManager.currentState != GameManager::PLAYING)
        return;

    // Handle Mob King shooting (player-controlled in dual player mode)
    if (gameManager.isDualPlayer())
    {
        handleMobKingShooting(ecs, gameManager, deltaTime);
    }

    // Handle regular mob shooting (at level 4 in single player, or in last 15s of dual/multiplayer)
    if ((gameManager.isSinglePlayer() && gameManager.canMobsShoot()) ||
        (gameManager.isDualPlayer() && gameManager.canMobsShoot()))
    {
        handleRegularMobShooting(ecs, deltaTime, gameManager);
    }
}

void WeaponSystem::handleMobKingShooting(ECS &ecs, GameManager &gameManager, float deltaTime)
{
    // Find Mob King entities
    auto &mobKingEntities = ecs.getComponents<MobKing>();

    for (auto &[mobKingEntityID, mobKing] : mobKingEntities)
    {
        Weapon *weapon = ecs.getComponent<Weapon>(mobKingEntityID);
        Transform *transform = ecs.getComponent<Transform>(mobKingEntityID);
        MovementDirection *movementDir = ecs.getComponent<MovementDirection>(mobKingEntityID);

        if (!weapon || !transform || !movementDir)
            continue;
        if (!weapon->canFire)
            continue;

        // Get keyboard state to check for P key
        const Uint8 *keyboardState = SDL_GetKeyboardState(nullptr);
        if (!keyboardState[SDL_SCANCODE_P])
            continue;

        // Determine shooting direction based on movement direction
        float dirX = 0.0f, dirY = 0.0f;

        if (movementDir->direction == MovementDirection::HORIZONTAL)
        {
            // Check last movement for horizontal direction
            Velocity *velocity = ecs.getComponent<Velocity>(mobKingEntityID);
            if (velocity)
            {
                if (velocity->x > 0)
                    dirX = 1.0f; // Moving right, shoot right
                else if (velocity->x < 0)
                    dirX = -1.0f; // Moving left, shoot left
                else
                    dirX = -1.0f; // Default to shooting left if stationary
            }
        }
        else // VERTICAL
        {
            // Check last movement for vertical direction
            Velocity *velocity = ecs.getComponent<Velocity>(mobKingEntityID);
            if (velocity)
            {
                if (velocity->y > 0)
                    dirY = 1.0f; // Moving down, shoot down
                else if (velocity->y < 0)
                    dirY = -1.0f; // Moving up, shoot up
                else
                    dirY = -1.0f; // Default to shooting up if stationary
            }
        }

        // Create projectile in facing direction
        EntityID projectileEntity = createProjectile(ecs, gameManager, transform->x, transform->y,
                                                     dirX, dirY, *weapon, mobKingEntityID, 400.0f, false);

        // Update weapon state
        weapon->fireTimer = 1.0f / weapon->fireRate;
        weapon->canFire = false;

        std::cout << "Mob King fired! Direction: (" << dirX << ", " << dirY << ")" << std::endl;
    }
}

void WeaponSystem::handleRegularMobShooting(ECS &ecs, float deltaTime, GameManager &gameManager)
{
    // Find player position for targeting
    float playerX = 0.0f, playerY = 0.0f;
    bool playerFound = false;

    auto &playerTags = ecs.getComponents<PlayerTag>();
    for (auto &[playerEntityID, playerTag] : playerTags)
    {
        Transform *playerTransform = ecs.getComponent<Transform>(playerEntityID);
        if (playerTransform)
        {
            playerX = playerTransform->x;
            playerY = playerTransform->y;
            playerFound = true;
            break;
        }
    }

    if (!playerFound)
        return;

    // Check all mobs with weapons (excluding Mob King)
    auto &mobTags = ecs.getComponents<MobTag>();
    for (auto &[mobEntityID, mobTag] : mobTags)
    {
        // Skip if this is a Mob King (they have separate handling)
        if (ecs.getComponent<MobKing>(mobEntityID))
            continue;

        Weapon *weapon = ecs.getComponent<Weapon>(mobEntityID);
        Transform *transform = ecs.getComponent<Transform>(mobEntityID);

        if (!weapon || !transform)
            continue;
        if (!weapon->canFire)
            continue;

        // Calculate distance to player
        float distX = playerX - transform->x;
        float distY = playerY - transform->y;
        float distance = std::sqrt(distX * distX + distY * distY);

        // Only shoot if player is within range
        if (distance > weapon->range)
            continue;

        // Calculate direction to player
        float dirX = distX / distance;
        float dirY = distY / distance;

        // Create projectile targeting player
        EntityID projectileEntity = createProjectile(ecs, gameManager, transform->x, transform->y,
                                                     dirX, dirY, *weapon, mobEntityID, 300.0f, false);

        // Update weapon state - use different fire rates for dual/multiplayer
        if (gameManager.isDualPlayer())
        {
            weapon->fireTimer = 1.0f; // 1 second between shots in dual/multiplayer (faster shooting)
        }
        else
        {
            weapon->fireTimer = 1.0f / weapon->fireRate; // Original fire rate for single player
        }
        weapon->canFire = false;

        std::cout << "Regular mob fired at player! Distance: " << distance << std::endl;
    }
}

bool WeaponSystem::isMousePressed()
{
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
    return (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
}

EntityID WeaponSystem::createProjectileFromNetwork(ECS &ecs, uint32_t projectileID, uint32_t shooterID, float x, float y, float velocityX, float velocityY, float damage, bool fromPlayer)
{
    std::cout << "Creating projectile from network: ID=" << projectileID << " from shooter=" << shooterID << " at (" << x << ", " << y << ") velocity=(" << velocityX << ", " << velocityY << ") damage=" << damage << " fromPlayer=" << fromPlayer << std::endl;

    EntityID projectileEntity = ecs.createEntity();

    // Add components to projectile
    ecs.addComponent(projectileEntity, Transform(x, y, 0.0f));

    // Add velocity from network data
    ecs.addComponent(projectileEntity, Velocity(velocityX, velocityY));

    // Calculate direction from velocity
    float speed = std::sqrt(velocityX * velocityX + velocityY * velocityY);
    float dirX = speed > 0 ? velocityX / speed : 0.0f;
    float dirY = speed > 0 ? velocityY / speed : 0.0f;

    // Add projectile component with damage and lifetime
    ecs.addComponent(projectileEntity, Projectile(speed, damage, 3.0f, shooterID, dirX, dirY));

    // Add projectile tag for identification
    ecs.addComponent(projectileEntity, ProjectileTag{});

    if (fromPlayer)
    {
        // Player projectiles: small, white/yellow
        ecs.addComponent(projectileEntity, Sprite(nullptr, 4, 4, 1, 0.0f));
        ecs.addComponent(projectileEntity, ProjectileColor(SDL_Color{255, 255, 0, 255})); // Yellow
        ecs.addComponent(projectileEntity, Collider(4.0f, 4.0f, false));
    }
    else
    {
        // Mob projectiles: larger, red
        ecs.addComponent(projectileEntity, Sprite(nullptr, 8, 8, 1, 0.0f));
        ecs.addComponent(projectileEntity, ProjectileColor(SDL_Color{255, 0, 0, 255})); // Red
        ecs.addComponent(projectileEntity, Collider(8.0f, 8.0f, false));
    }

    std::cout << "Network projectile created successfully!" << std::endl;
    return projectileEntity;
}
