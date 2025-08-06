#pragma once
#include "System.h"
#include "../core/ECS.h"
#include "../managers/GameManager.h"

class BoundarySystem : public System
{
private:
    float screenWidth;
    float screenHeight;

public:
    BoundarySystem(float screenW, float screenH)
        : screenWidth(screenW), screenHeight(screenH) {}

    void update(ECS &ecs, GameManager &gameManager, float deltaTime) override;

private:
    void keepPlayerInBounds(ECS &ecs);
    void removeOffScreenMobs(ECS &ecs);
};
