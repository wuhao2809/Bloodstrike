#include "WeaponSystem.h"
#include "../components/Components.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>

WeaponSystem::WeaponSystem(EntityFactory *factory) : entityFactory(factory)
{
}

WeaponSystem::~WeaponSystem()
{
}

void WeaponSystem::update(ECS &ecs, GameManager &gameManager, float deltaTime)
{
    updateWeaponTimers(ecs, deltaTime);
    handleShooting(ecs, deltaTime);
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

void WeaponSystem::handleShooting(ECS &ecs, float deltaTime)
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
                                                     dirX, dirY, *weapon, entityID);

        // Update weapon state
        weapon->ammoCount--;
        weapon->fireTimer = 1.0f / weapon->fireRate; // Time between shots
        weapon->canFire = false;

        std::cout << "Player fired! Ammo remaining: " << weapon->ammoCount << std::endl;

        break; // Only one player can shoot
    }
}

EntityID WeaponSystem::createProjectile(ECS &ecs, float startX, float startY,
                                        float dirX, float dirY, const Weapon &weapon, EntityID owner)
{
    EntityID projectileEntity = ecs.createEntity();

    // Projectile speed (pixels per second)
    const float projectileSpeed = 500.0f;

    // Add components to projectile
    ecs.addComponent(projectileEntity, Transform(startX, startY, 0.0f));

    // Add velocity (direction * speed)
    ecs.addComponent(projectileEntity, Velocity(dirX * projectileSpeed, dirY * projectileSpeed));

    // Add projectile component with damage and lifetime
    ecs.addComponent(projectileEntity, Projectile(projectileSpeed, weapon.damage, 3.0f, owner, dirX, dirY));

    // Add projectile tag for identification
    ecs.addComponent(projectileEntity, ProjectileTag{});

    // Add sprite for rendering (small bullet)
    ecs.addComponent(projectileEntity, Sprite(nullptr, 4, 4, 1, 0.0f));

    // Add collider for hit detection
    ecs.addComponent(projectileEntity, Collider(4.0f, 4.0f, false));

    return projectileEntity;
}

bool WeaponSystem::isMousePressed()
{
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
    return (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
}
