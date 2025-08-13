#include "HealthUISystem.h"
#include "../components/Components.h"
#include <iostream>
#include <sstream>

HealthUISystem::HealthUISystem(EntityFactory *factory) : entityFactory(factory)
{
}

HealthUISystem::~HealthUISystem()
{
}

void HealthUISystem::update(ECS &ecs, GameManager &gameManager, float deltaTime)
{
    // Only update during dual/multiplayer gameplay
    if (gameManager.currentState != GameManager::PLAYING || !gameManager.isDualPlayer())
    {
        return;
    }

    // Check if we need to create health UI for newly spawned Mob King
    auto &mobKings = ecs.getComponents<MobKing>();
    for (auto &[mobKingID, mobKing] : mobKings)
    {
        // Check if this Mob King already has a health UI
        bool hasHealthUI = false;
        auto &healthUIComponents = ecs.getComponents<MobKingHealthUI>();
        for (auto &[uiEntityID, healthUI] : healthUIComponents)
        {
            if (healthUI.mobKingEntity == mobKingID)
            {
                hasHealthUI = true;
                break;
            }
        }

        // Create health UI if it doesn't exist
        if (!hasHealthUI)
        {
            Health *health = ecs.getComponent<Health>(mobKingID);
            if (health)
            {
                createMobKingHealthUI(ecs, mobKingID, *health);
            }
        }
    }

    // Update existing health UI
    updateMobKingHealthUI(ecs);
}

void HealthUISystem::updateMobKingHealthUI(ECS &ecs)
{
    auto &healthUIComponents = ecs.getComponents<MobKingHealthUI>();
    std::vector<EntityID> uiToRemove;

    for (auto &[uiEntityID, healthUI] : healthUIComponents)
    {
        // Check if the associated Mob King still exists
        Health *mobHealth = ecs.getComponent<Health>(healthUI.mobKingEntity);
        if (!mobHealth || !ecs.getComponent<MobKing>(healthUI.mobKingEntity))
        {
            // Mob King is dead or removed, mark UI for removal
            uiToRemove.push_back(uiEntityID);
            continue;
        }

        // Update the health display
        UIText *uiText = ecs.getComponent<UIText>(uiEntityID);
        if (uiText)
        {
            // Get UI configuration
            json uiConfig = entityFactory->getEntityConfig()["ui"]["mobKingHealth"];
            
            // Update health text
            std::string format = uiConfig["text"]["format"].get<std::string>();
            std::string healthText = format;
            
            // Replace placeholders
            size_t currentPos = healthText.find("{current}");
            if (currentPos != std::string::npos)
            {
                healthText.replace(currentPos, 9, std::to_string((int)mobHealth->currentHealth));
            }
            
            size_t maxPos = healthText.find("{max}");
            if (maxPos != std::string::npos)
            {
                healthText.replace(maxPos, 5, std::to_string((int)mobHealth->maxHealth));
            }
            
            uiText->content = healthText;
            
            // Update color based on health percentage
            float healthPercent = mobHealth->currentHealth / mobHealth->maxHealth;
            uiText->color = getHealthColor(healthPercent, uiConfig);
        }
    }

    // Remove dead Mob King UIs
    for (EntityID uiID : uiToRemove)
    {
        ecs.removeEntity(uiID);
        std::cout << "Removed Mob King health UI (Mob King defeated)" << std::endl;
    }
}

void HealthUISystem::createMobKingHealthUI(ECS &ecs, EntityID mobKingEntity, const Health &health)
{
    // Get UI configuration from JSON
    json uiConfig = entityFactory->getEntityConfig()["ui"]["mobKingHealth"];
    
    // Create health UI entity
    EntityID healthUIEntity = ecs.createEntity();

    // Set position
    json position = uiConfig["position"];
    UIPosition uiPos(position["x"].get<float>(), position["y"].get<float>());
    ecs.addComponent(healthUIEntity, uiPos);

    // Create health text
    json textConfig = uiConfig["text"];
    std::string format = textConfig["format"].get<std::string>();
    std::string healthText = format;
    
    // Replace placeholders
    size_t currentPos = healthText.find("{current}");
    if (currentPos != std::string::npos)
    {
        healthText.replace(currentPos, 9, std::to_string((int)health.currentHealth));
    }
    
    size_t maxPos = healthText.find("{max}");
    if (maxPos != std::string::npos)
    {
        healthText.replace(maxPos, 5, std::to_string((int)health.maxHealth));
    }

    // Get initial color
    float healthPercent = health.currentHealth / health.maxHealth;
    SDL_Color color = getHealthColor(healthPercent, uiConfig);

    UIText uiText(healthText, 
                  textConfig["font"].get<std::string>(), 
                  textConfig["fontSize"].get<int>(), 
                  color, 
                  true);
    ecs.addComponent(healthUIEntity, uiText);

    // Add component to track which Mob King this UI belongs to
    MobKingHealthUI healthUITracker(mobKingEntity);
    ecs.addComponent(healthUIEntity, healthUITracker);

    std::cout << "Created Mob King health UI: " << healthText << std::endl;
}

void HealthUISystem::removeMobKingHealthUI(ECS &ecs, EntityID mobKingEntity)
{
    auto &healthUIComponents = ecs.getComponents<MobKingHealthUI>();
    std::vector<EntityID> uiToRemove;
    
    for (auto &[uiEntityID, healthUI] : healthUIComponents)
    {
        if (healthUI.mobKingEntity == mobKingEntity)
        {
            uiToRemove.push_back(uiEntityID);
        }
    }
    
    for (EntityID uiEntityID : uiToRemove)
    {
        ecs.removeEntity(uiEntityID);
        std::cout << "Removed Mob King health UI" << std::endl;
    }
}

void HealthUISystem::removeAllHealthUI(ECS &ecs)
{
    auto &healthUIComponents = ecs.getComponents<MobKingHealthUI>();
    std::vector<EntityID> uiToRemove;
    
    for (auto &[uiEntityID, healthUI] : healthUIComponents)
    {
        uiToRemove.push_back(uiEntityID);
    }
    
    for (EntityID uiEntityID : uiToRemove)
    {
        ecs.removeEntity(uiEntityID);
    }
}

SDL_Color HealthUISystem::getHealthColor(float healthPercent, const json &config)
{
    json colors = config["colors"];
    json thresholds = config["thresholds"];
    
    if (healthPercent > thresholds["warning"].get<float>())
    {
        // Healthy - white
        json color = colors["healthy"];
        return {(Uint8)color["r"].get<int>(), (Uint8)color["g"].get<int>(), 
                (Uint8)color["b"].get<int>(), (Uint8)color["a"].get<int>()};
    }
    else if (healthPercent > thresholds["critical"].get<float>())
    {
        // Warning - yellow
        json color = colors["warning"];
        return {(Uint8)color["r"].get<int>(), (Uint8)color["g"].get<int>(), 
                (Uint8)color["b"].get<int>(), (Uint8)color["a"].get<int>()};
    }
    else
    {
        // Critical - red
        json color = colors["critical"];
        return {(Uint8)color["r"].get<int>(), (Uint8)color["g"].get<int>(), 
                (Uint8)color["b"].get<int>(), (Uint8)color["a"].get<int>()};
    }
}
