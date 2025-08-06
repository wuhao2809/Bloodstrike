#include "EntityFactory.h"
#include "ResourceManager.h"
#include "../components/Components.h"
#include <fstream>
#include <iostream>
#include <random>

bool EntityFactory::loadConfig(const std::string &configFile)
{
    std::ifstream file(configFile);
    if (!file.is_open())
    {
        std::cerr << "Failed to open config file: " << configFile << std::endl;
        return false;
    }

    try
    {
        file >> entityConfig;
        return true;
    }
    catch (const json::exception &e)
    {
        std::cerr << "Failed to parse JSON config: " << e.what() << std::endl;
        return false;
    }
}

EntityID EntityFactory::createPlayer(ECS &ecs)
{
    if (!entityConfig.contains("player"))
    {
        std::cerr << "Player configuration not found in JSON" << std::endl;
        return 0;
    }

    EntityID playerID = ecs.createEntity();
    const json &playerConfig = entityConfig["player"];

    // Add Transform component with start position
    Transform transform = createTransformFromJSON(playerConfig, playerConfig["startPosition"]);
    ecs.addComponent(playerID, transform);

    // Add Velocity component (starts at zero)
    ecs.addComponent(playerID, Velocity(0, 0));

    // Add MovementDirection component (starts with horizontal)
    ecs.addComponent(playerID, MovementDirection(MovementDirection::HORIZONTAL));

    // Add Sprite component (start with horizontal sprite)
    Sprite sprite = createSpriteFromJSON(playerConfig["sprites"]["horizontal"]);
    ecs.addComponent(playerID, sprite);

    // Add Collider component
    Collider collider = createColliderFromJSON(playerConfig["collider"]);
    ecs.addComponent(playerID, collider);

    // Add Speed component
    Speed speed(playerConfig["speed"].get<float>());
    ecs.addComponent(playerID, speed);

    // Add Animation component
    ecs.addComponent(playerID, Animation());

    // Add PlayerTag
    ecs.addComponent(playerID, PlayerTag{});

    // Add EntityType
    ecs.addComponent(playerID, EntityType("player"));

    // Add combat components if available
    if (playerConfig.contains("combat"))
    {
        const json &combatConfig = playerConfig["combat"];

        // Add MouseTarget component
        ecs.addComponent(playerID, MouseTarget());

        // Add AimingLine component with configuration
        if (combatConfig.contains("aimingLine"))
        {
            const json &aimingConfig = combatConfig["aimingLine"];
            AimingLine aimingLine;
            aimingLine.maxRange = aimingConfig["maxRange"].get<float>();
            aimingLine.dotCount = aimingConfig["dotCount"].get<int>();
            aimingLine.dotSpacing = aimingConfig["dotSpacing"].get<float>();
            ecs.addComponent(playerID, aimingLine);
        }
        else
        {
            ecs.addComponent(playerID, AimingLine());
        }

        // Add Weapon component with configuration
        if (combatConfig.contains("weapon"))
        {
            const json &weaponConfig = combatConfig["weapon"];
            Weapon weapon;
            weapon.damage = weaponConfig["damage"].get<float>();
            weapon.fireRate = weaponConfig["fireRate"].get<float>();
            weapon.ammoCount = weaponConfig["ammoCount"].get<int>();
            weapon.maxAmmo = weaponConfig["maxAmmo"].get<int>();
            weapon.range = weaponConfig["range"].get<float>();
            weapon.fireTimer = 0.0f; // Ready to fire
            weapon.canFire = true;   // Ready to fire
            ecs.addComponent(playerID, weapon);
        }
        else
        {
            ecs.addComponent(playerID, Weapon()); // Default weapon
        }
    }

    return playerID;
}

EntityID EntityFactory::createMob(ECS &ecs, const std::string &mobType)
{
    if (!entityConfig.contains("mobs") || !entityConfig["mobs"].contains(mobType))
    {
        std::cerr << "Mob type '" << mobType << "' not found in JSON" << std::endl;
        return 0;
    }

    EntityID mobID = ecs.createEntity();
    const json &mobConfig = entityConfig["mobs"][mobType];

    // Add Transform component (position will be set by spawning system)
    ecs.addComponent(mobID, Transform());

    // Add Velocity component (will be set by spawning system)
    ecs.addComponent(mobID, Velocity());

    // Add Sprite component
    Sprite sprite = createSpriteFromJSON(mobConfig["sprite"]);
    ecs.addComponent(mobID, sprite);

    // Add Collider component
    Collider collider = createColliderFromJSON(mobConfig["collider"]);
    ecs.addComponent(mobID, collider);

    // Add Speed component (random within range)
    float minSpeed = mobConfig["speedRange"]["min"].get<float>();
    float maxSpeed = mobConfig["speedRange"]["max"].get<float>();

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> speedDist(minSpeed, maxSpeed);

    Speed speed(speedDist(gen));
    ecs.addComponent(mobID, speed);

    // Add Animation component
    ecs.addComponent(mobID, Animation());

    // Add MobTag
    ecs.addComponent(mobID, MobTag{});

    // Add EntityType
    ecs.addComponent(mobID, EntityType(mobType));

    return mobID;
}

EntityID EntityFactory::createUIElement(ECS &ecs, const std::string &uiType)
{
    if (!entityConfig.contains("ui") || !entityConfig["ui"].contains(uiType))
    {
        std::cerr << "UI element '" << uiType << "' not found in JSON" << std::endl;
        return 0;
    }

    EntityID uiID = ecs.createEntity();
    const json &uiConfig = entityConfig["ui"][uiType];

    // Add UIPosition component
    UIPosition uiPos = createUIPositionFromJSON(uiConfig);
    ecs.addComponent(uiID, uiPos);

    // Add UIText component
    UIText uiText = createUITextFromJSON(uiConfig);
    ecs.addComponent(uiID, uiText);

    // Add EntityType
    ecs.addComponent(uiID, EntityType(uiType));

    return uiID;
}

Transform EntityFactory::createTransformFromJSON(const json &config, const json &positionOverride)
{
    float x = 0, y = 0, rotation = 0;

    if (!positionOverride.is_null())
    {
        x = positionOverride["x"].get<float>();
        y = positionOverride["y"].get<float>();
    }
    else if (config.contains("position"))
    {
        x = config["position"]["x"].get<float>();
        y = config["position"]["y"].get<float>();
    }

    if (config.contains("rotation"))
    {
        rotation = config["rotation"].get<float>();
    }

    return Transform(x, y, rotation);
}

Sprite EntityFactory::createSpriteFromJSON(const json &config)
{
    std::string texturePath = config["texture"].get<std::string>();
    SDL_Texture *texture = resourceManager->loadTexture(texturePath);

    int width = config["width"].get<int>();
    int height = config["height"].get<int>();
    int frameCount = config.contains("frameCount") ? config["frameCount"].get<int>() : 1;
    float frameTime = config.contains("frameTime") ? config["frameTime"].get<float>() : 0.1f;

    return Sprite(texture, width, height, frameCount, frameTime);
}

Collider EntityFactory::createColliderFromJSON(const json &config)
{
    float width = config["width"].get<float>();
    float height = config["height"].get<float>();
    bool isTrigger = config.contains("isTrigger") ? config["isTrigger"].get<bool>() : false;

    return Collider(width, height, isTrigger);
}

Speed EntityFactory::createSpeedFromJSON(const json &config)
{
    float value = config.get<float>();
    return Speed(value);
}

UIText EntityFactory::createUITextFromJSON(const json &config)
{
    std::string text = config["text"].get<std::string>();
    std::string font = config["font"].get<std::string>();
    int fontSize = config["fontSize"].get<int>();

    SDL_Color color = {255, 255, 255, 255}; // Default white
    if (config.contains("color"))
    {
        color.r = config["color"]["r"].get<Uint8>();
        color.g = config["color"]["g"].get<Uint8>();
        color.b = config["color"]["b"].get<Uint8>();
        color.a = config["color"]["a"].get<Uint8>();
    }

    return UIText(text, font, fontSize, color, true);
}

UIPosition EntityFactory::createUIPositionFromJSON(const json &config)
{
    float x = config["position"]["x"].get<float>();
    float y = config["position"]["y"].get<float>();
    return UIPosition(x, y);
}
