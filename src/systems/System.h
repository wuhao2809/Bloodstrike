#pragma once
#include "../core/ECS.h"
#include "../managers/GameManager.h"
#include <SDL2/SDL.h>
#include <chrono>

// Base System class
class System
{
public:
    virtual ~System() = default;
    virtual void update(ECS &ecs, float deltaTime) {}
    virtual void update(ECS &ecs, GameManager &gameManager) {}
    virtual void update(ECS &ecs, GameManager &gameManager, float deltaTime) {}
};
