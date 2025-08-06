#include "AnimationSystem.h"
#include "../components/Components.h"

void AnimationSystem::update(ECS &ecs, float deltaTime)
{
    auto &animations = ecs.getComponents<Animation>();

    for (auto &[entityID, animation] : animations)
    {
        auto *sprite = ecs.getComponent<Sprite>(entityID);

        if (sprite && sprite->animated && sprite->frameCount > 1)
        {
            animation.animationTimer += deltaTime;

            if (animation.animationTimer >= sprite->frameTime)
            {
                animation.currentFrame = (animation.currentFrame + 1) % sprite->frameCount;
                animation.animationTimer = 0.0f;
            }
        }
    }
}
