#pragma once
#include "System.h"

class AnimationSystem : public System
{
public:
    void update(ECS &ecs, float deltaTime) override;
};
