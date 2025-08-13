#pragma once
#include "System.h"
#include "../core/ECS.h"
#include "../managers/GameManager.h"
#include "../managers/EntityFactory.h"
#include "../components/Components.h"

class HealthUISystem : public System
{
private:
    EntityFactory *entityFactory;

public:
    HealthUISystem(EntityFactory *factory);
    ~HealthUISystem();

    void update(ECS &ecs, GameManager &gameManager, float deltaTime) override;

private:
    void updateMobKingHealthUI(ECS &ecs);
    void createMobKingHealthUI(ECS &ecs, EntityID mobKingEntity, const Health &health);
    void removeMobKingHealthUI(ECS &ecs, EntityID mobKingEntity);
    void removeAllHealthUI(ECS &ecs);
    SDL_Color getHealthColor(float healthPercent, const json &config);
};
