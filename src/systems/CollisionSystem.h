#pragma once
#include "System.h"
#include "../core/ECS.h"
#include "../components/Components.h"
#include "../managers/GameManager.h"
#include "../systems/AudioSystem.h"

class CollisionSystem : public System
{
private:
    AudioSystem *audioSystem;

public:
    CollisionSystem(AudioSystem *audio) : audioSystem(audio) {}
    void update(ECS &ecs, GameManager &gameManager, float deltaTime) override;

private:
    bool checkCollision(const Transform &pos1, const Collider &col1,
                        const Transform &pos2, const Collider &col2);
    void handlePlayerMobCollision(ECS &ecs, GameManager &gameManager,
                                  EntityID playerEntity, EntityID mobEntity);
};
