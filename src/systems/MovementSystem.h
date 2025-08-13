#pragma once
#include "System.h"

class MovementSystem : public System
{
public:
    void update(ECS &ecs, float deltaTime) override;

    // Network synchronization
    void updateEntityFromNetwork(ECS &ecs, uint32_t entityID, float x, float y, float velocityX, float velocityY, const std::string &entityType);
};
