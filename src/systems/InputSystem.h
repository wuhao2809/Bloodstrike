#pragma once
#include "System.h"
#include "../managers/GameManager.h"
#include <SDL2/SDL.h>

class InputSystem : public System
{
private:
    const Uint8 *keyboardState;
    void clearAllMobs(ECS &ecs);
    void clearAllProjectiles(ECS &ecs);

public:
    InputSystem();
    void update(ECS &ecs, GameManager &gameManager, float deltaTime) override;
};
