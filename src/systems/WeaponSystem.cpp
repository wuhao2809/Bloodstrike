#include "WeaponSystem.h"
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
    handlePlayerShooting(ecs, deltaTime);
    handleMobShooting(ecs, gameManager, deltaTime);
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

void WeaponSystem::handlePlayerShooting(ECS &ecs, float deltaTime)
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
        EntityID projectileEntity = createProjectile(ecs, transform->x, transform->y,
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

EntityID WeaponSystem::createProjectile(ECS &ecs, float startX, float startY,
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

    return projectileEntity;
}

void WeaponSystem::handleMobShooting(ECS &ecs, GameManager &gameManager, float deltaTime)
{
    // Only allow mob shooting during gameplay and at level 4
    if (gameManager.currentState != GameManager::PLAYING || !gameManager.canMobsShoot())
        return;

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

    // Check all mobs with weapons
    auto &mobTags = ecs.getComponents<MobTag>();
    for (auto &[mobEntityID, mobTag] : mobTags)
    {
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
        EntityID projectileEntity = createProjectile(ecs, transform->x, transform->y,
                                                     dirX, dirY, *weapon, mobEntityID, 300.0f, false);

        // Update weapon state
        weapon->fireTimer = 1.0f / weapon->fireRate;
        weapon->canFire = false;

        std::cout << "Mob fired at player! Distance: " << distance << std::endl;
    }
}

bool WeaponSystem::isMousePressed()
{
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
    return (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
}
