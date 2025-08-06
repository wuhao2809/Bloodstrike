#include "AimingSystem.h"
#include "../components/Components.h"
#include <iostream>

AimingSystem::AimingSystem() : mouseX(0), mouseY(0), mousePressed(false)
{
}

AimingSystem::~AimingSystem()
{
}

void AimingSystem::update(ECS &ecs, GameManager &gameManager, float deltaTime)
{
    updateMouseInput();
    updateMouseTarget(ecs, gameManager);
    calculateAimingLine(ecs);
}

void AimingSystem::updateMouseInput()
{
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
    mousePressed = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
}

void AimingSystem::updateMouseTarget(ECS &ecs, GameManager &gameManager)
{
    // Get all MouseTarget components
    auto &mouseTargets = ecs.getComponents<MouseTarget>();

    for (auto &[entityID, mouseTarget] : mouseTargets)
    {
        // Convert screen coordinates to world coordinates
        mouseTarget.x = static_cast<float>(mouseX);
        mouseTarget.y = static_cast<float>(mouseY);

        // For now, mouse is always valid (later we can add range checks)
        mouseTarget.isValid = true;
    }
}

void AimingSystem::calculateAimingLine(ECS &ecs)
{
    // Find player entity (has PlayerTag)
    Transform *playerTransform = nullptr;
    auto &playerTags = ecs.getComponents<PlayerTag>();

    for (auto &[entityID, playerTag] : playerTags)
    {
        playerTransform = ecs.getComponent<Transform>(entityID);
        break; // Assume only one player
    }

    if (!playerTransform)
        return;

    // Update all aiming lines
    auto &aimingLines = ecs.getComponents<AimingLine>();

    for (auto &[entityID, aimingLine] : aimingLines)
    {
        MouseTarget *mouseTarget = ecs.getComponent<MouseTarget>(entityID);

        if (mouseTarget)
        {
            // Set start position to player position
            aimingLine.startX = playerTransform->x;
            aimingLine.startY = playerTransform->y;

            // Set end position to mouse position
            aimingLine.endX = mouseTarget->x;
            aimingLine.endY = mouseTarget->y;

            // Calculate distance and check if within range
            float distance = calculateDistance(aimingLine.startX, aimingLine.startY,
                                               aimingLine.endX, aimingLine.endY);

            // Show line if mouse is being tracked
            aimingLine.showLine = mouseTarget->isValid;

            // Clamp to max range if needed
            if (distance > aimingLine.maxRange)
            {
                float ratio = aimingLine.maxRange / distance;
                float dirX = aimingLine.endX - aimingLine.startX;
                float dirY = aimingLine.endY - aimingLine.startY;

                aimingLine.endX = aimingLine.startX + dirX * ratio;
                aimingLine.endY = aimingLine.startY + dirY * ratio;
            }
        }
    }
}

float AimingSystem::calculateDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

void AimingSystem::normalizeDirection(float &dirX, float &dirY)
{
    float length = std::sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0f)
    {
        dirX /= length;
        dirY /= length;
    }
}
