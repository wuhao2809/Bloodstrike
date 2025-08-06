#pragma once
#include "../core/ECS.h"
#include "../components/Components.h"
#include <nlohmann/json.hpp>
#include <string>

class ResourceManager; // Forward declaration

using json = nlohmann::json;

class EntityFactory
{
private:
    json entityConfig;
    ResourceManager *resourceManager;

public:
    EntityFactory(ResourceManager *rm) : resourceManager(rm) {}

    // Load entity configuration from JSON file
    bool loadConfig(const std::string &configFile);

    // Create entities from JSON configuration
    EntityID createPlayer(ECS &ecs);
    EntityID createMob(ECS &ecs, const std::string &mobType);
    EntityID createUIElement(ECS &ecs, const std::string &uiType);

    // Get game settings from JSON
    json getGameSettings() const { return entityConfig["gameSettings"]; }

    // Get full entity configuration
    json getEntityConfig() const { return entityConfig; }

private:
    // Helper methods for creating components from JSON
    Transform createTransformFromJSON(const json &config, const json &positionOverride = json::object());
    Sprite createSpriteFromJSON(const json &config);
    Collider createColliderFromJSON(const json &config);
    Speed createSpeedFromJSON(const json &config);
    UIText createUITextFromJSON(const json &config);
    UIPosition createUIPositionFromJSON(const json &config);
};
