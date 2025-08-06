#pragma once
#include "System.h"
#include "../core/ECS.h"
#include "../managers/GameManager.h"
#include <SDL2/SDL.h>
#include <cmath>

class AimingSystem : public System
{
private:
    int mouseX, mouseY;
    bool mousePressed;

public:
    AimingSystem();
    ~AimingSystem();

    void update(ECS &ecs, GameManager &gameManager, float deltaTime) override;

private:
    void updateMouseInput();
    void updateMouseTarget(ECS &ecs, GameManager &gameManager);
    void calculateAimingLine(ECS &ecs);
    float calculateDistance(float x1, float y1, float x2, float y2);
    void normalizeDirection(float &dirX, float &dirY);
};
