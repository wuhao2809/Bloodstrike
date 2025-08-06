#include "MovementSystem.h"
#include "../components/Components.h"

void MovementSystem::update(ECS &ecs, float deltaTime)
{
    auto &transforms = ecs.getComponents<Transform>();

    for (auto &[entityID, transform] : transforms)
    {
        auto *velocity = ecs.getComponent<Velocity>(entityID);
        auto *speed = ecs.getComponent<Speed>(entityID);

        if (velocity && speed)
        {
            // Apply velocity * speed * deltaTime to position
            transform.x += velocity->x * speed->value * deltaTime;
            transform.y += velocity->y * speed->value * deltaTime;
        }
    }
}
