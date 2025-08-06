#pragma once
#include "System.h"

class MovementSystem : public System
{
public:
    void update(ECS &ecs, float deltaTime) override;
};
