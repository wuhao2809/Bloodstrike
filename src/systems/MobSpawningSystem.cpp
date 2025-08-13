#include "MobSpawningSystem.h"
#include "../components/Components.h"
#include <iostream>
#include <chrono>

MobSpawningSystem::MobSpawningSystem(EntityFactory *factory, float screenW, float screenH)
    : entityFactory(factory), timeSinceLastSpawn(0.0f), spawnInterval(0.5f),
      screenWidth(screenW), screenHeight(screenH),
      randomGenerator(std::chrono::steady_clock::now().time_since_epoch().count()),
      mobTypeDistribution(0, 2), // 0-2 for 3 mob types
      positionDistribution(0.0f, 1.0f),
      speedDistribution(0.0f, 1.0f)
{
}

void MobSpawningSystem::update(ECS &ecs, GameManager &gameManager, float deltaTime)
{
    // Only spawn mobs during gameplay
    if (gameManager.currentState != GameManager::PLAYING)
    {
        return;
    }

    // Handle dual player mode - spawn Mob King if needed
    if (gameManager.isDualPlayer() && gameManager.shouldSpawnMobKing() && !mobKingSpawned)
    {
        spawnMobKing(ecs, gameManager);
        mobKingSpawned = true;
    }

    // Update spawn timer
    timeSinceLastSpawn += deltaTime;

    // Get level-based spawn interval (modified for game mode)
    float currentSpawnInterval = gameManager.getGameModeSpawnInterval();

    // Check if it's time to spawn a new mob
    if (timeSinceLastSpawn >= currentSpawnInterval)
    {
        spawnMob(ecs, gameManager);
        timeSinceLastSpawn = 0.0f;
    }
}

void MobSpawningSystem::spawnMob(ECS &ecs, GameManager &gameManager)
{
    // Choose random mob type
    int mobTypeIndex = mobTypeDistribution(randomGenerator);
    std::string mobType = mobTypes[mobTypeIndex];

    // Get mob configuration
    json mobConfig = entityFactory->getEntityConfig()["mobs"][mobType];

    // Create mob entity
    EntityID mobEntity = ecs.createEntity();

    // Add MobTag component
    ecs.addComponent(mobEntity, MobTag{});

    // Add EntityType component for texture identification
    ecs.addComponent(mobEntity, EntityType{mobType});

    // Determine spawn edge and direction randomly
    int edge = std::uniform_int_distribution<int>(0, 3)(randomGenerator); // 0=right, 1=left, 2=top, 3=bottom
    float spawnX, spawnY;
    Velocity velocity;
    MovementDirection::Direction facingDirection;

    switch (edge)
    {
    case 0: // Spawn from right edge, move left
        spawnX = screenWidth + 50.0f;
        spawnY = positionDistribution(randomGenerator) * (screenHeight - 100.0f) + 50.0f;
        velocity.x = -1.0f;
        velocity.y = 0.0f;
        facingDirection = MovementDirection::HORIZONTAL;
        break;
    case 1: // Spawn from left edge, move right
        spawnX = -50.0f;
        spawnY = positionDistribution(randomGenerator) * (screenHeight - 100.0f) + 50.0f;
        velocity.x = 1.0f;
        velocity.y = 0.0f;
        facingDirection = MovementDirection::HORIZONTAL;
        break;
    case 2: // Spawn from top edge, move down
        spawnX = positionDistribution(randomGenerator) * (screenWidth - 100.0f) + 50.0f;
        spawnY = -50.0f;
        velocity.x = 0.0f;
        velocity.y = 1.0f;
        facingDirection = MovementDirection::VERTICAL;
        break;
    case 3: // Spawn from bottom edge, move up
        spawnX = positionDistribution(randomGenerator) * (screenWidth - 100.0f) + 50.0f;
        spawnY = screenHeight + 50.0f;
        velocity.x = 0.0f;
        velocity.y = -1.0f;
        facingDirection = MovementDirection::VERTICAL;
        break;
    }

    // Create Transform component
    Transform transform;
    transform.x = spawnX;
    transform.y = spawnY;
    transform.rotation = 0.0f;
    ecs.addComponent(mobEntity, transform);

    // Add MovementDirection component for proper sprite orientation
    ecs.addComponent(mobEntity, MovementDirection(facingDirection));

    // Create Sprite component
    json spriteConfig = mobConfig["sprite"];
    Sprite sprite;
    sprite.texture = nullptr; // Will be loaded by ResourceManager in RenderSystem
    sprite.width = spriteConfig["width"].get<int>();
    sprite.height = spriteConfig["height"].get<int>();
    sprite.frameCount = spriteConfig["frameCount"].get<int>();
    sprite.frameTime = spriteConfig["frameTime"].get<float>();
    sprite.animated = spriteConfig["animated"].get<bool>();
    ecs.addComponent(mobEntity, sprite);

    // Create Animation component if animated
    if (spriteConfig["animated"].get<bool>())
    {
        Animation animation;
        animation.currentFrame = 0;
        animation.animationTimer = 0.0f;
        ecs.addComponent(mobEntity, animation);
    } // Create Collider component
    json colliderConfig = mobConfig["collider"];
    Collider collider;
    collider.width = colliderConfig["width"].get<float>();
    collider.height = colliderConfig["height"].get<float>();
    collider.isTrigger = colliderConfig["isTrigger"].get<bool>();
    ecs.addComponent(mobEntity, collider);

    // Use the calculated velocity from spawn logic
    ecs.addComponent(mobEntity, velocity);

    // Create Speed component (random speed within range, modified by level)
    json speedRange = mobConfig["speedRange"];
    float minSpeed = speedRange["min"].get<float>();
    float maxSpeed = speedRange["max"].get<float>();
    float baseSpeed = minSpeed + speedDistribution(randomGenerator) * (maxSpeed - minSpeed);

    // Apply level speed multiplier
    float levelSpeedMultiplier = gameManager.getLevelSpeedMultiplier();
    float finalSpeed = baseSpeed * levelSpeedMultiplier;

    Speed speedComponent;
    speedComponent.value = finalSpeed;
    ecs.addComponent(mobEntity, speedComponent);

    // Regular mobs can shoot based on game mode and conditions
    bool shouldHaveWeapon = gameManager.canMobsShoot();

    // Add weapon component if conditions are met
    if (shouldHaveWeapon)
    {
        // Load combat config from JSON
        json combatConfig = mobConfig.contains("combat") ? mobConfig["combat"] : entityFactory->getEntityConfig()["defaultMobCombat"];

        // Add weapon component
        Weapon weapon;
        weapon.damage = combatConfig["damage"].get<float>();
        weapon.range = combatConfig["range"].get<float>();
        weapon.fireRate = combatConfig["fireRate"].get<float>();
        weapon.fireTimer = 0.0f;
        weapon.canFire = true;
        weapon.ammoCount = 999; // Unlimited ammo for mobs
        weapon.maxAmmo = 999;
        ecs.addComponent(mobEntity, weapon);

        std::cout << "Spawned " << mobType << " mob WITH WEAPON at (" << spawnX << ", " << spawnY
                  << ") with speed " << finalSpeed << " (base: " << baseSpeed
                  << ", multiplier: " << levelSpeedMultiplier << ") - CAN SHOOT!" << std::endl;
    }
    else
    {
        std::cout << "Spawned " << mobType << " mob at (" << spawnX << ", " << spawnY
                  << ") with speed " << finalSpeed << " (base: " << baseSpeed
                  << ", multiplier: " << levelSpeedMultiplier << ") - spawn interval: "
                  << gameManager.getGameModeSpawnInterval() << "s" << std::endl;
    }
}

void MobSpawningSystem::spawnMobKing(ECS &ecs, GameManager &gameManager)
{
    std::cout << "Spawning Mob King!" << std::endl;

    // Get mob king configuration
    json mobKingConfig = entityFactory->getEntityConfig()["mobs"]["mobKing"];

    // Create mob king entity
    EntityID mobKingEntity = ecs.createEntity();

    // Add MobKing component (special marker for the boss)
    ecs.addComponent(mobKingEntity, MobKing{});

    // Add MobTag component for general mob behavior
    ecs.addComponent(mobKingEntity, MobTag{});

    // Add EntityType component for texture identification
    ecs.addComponent(mobKingEntity, EntityType{"mobKing"});

    // Set spawn position from config (right side of screen)
    json startPos = mobKingConfig["startPosition"];
    Transform transform;
    transform.x = startPos["x"].get<float>();
    transform.y = startPos["y"].get<float>();
    transform.rotation = 0.0f;
    ecs.addComponent(mobKingEntity, transform);

    // Add MovementDirection component
    ecs.addComponent(mobKingEntity, MovementDirection(MovementDirection::HORIZONTAL));

    // Create Sprite component
    json spriteConfig = mobKingConfig["sprite"];
    Sprite sprite;
    sprite.texture = nullptr; // Will be loaded by ResourceManager in RenderSystem
    sprite.width = spriteConfig["width"].get<int>();
    sprite.height = spriteConfig["height"].get<int>();
    sprite.frameCount = spriteConfig["frameCount"].get<int>();
    sprite.frameTime = spriteConfig["frameTime"].get<float>();
    sprite.animated = spriteConfig["animated"].get<bool>();
    ecs.addComponent(mobKingEntity, sprite);

    // Create Animation component if animated
    if (spriteConfig["animated"].get<bool>())
    {
        Animation animation;
        animation.currentFrame = 0;
        animation.animationTimer = 0.0f;
        ecs.addComponent(mobKingEntity, animation);
    }

    // Create Collider component
    json colliderConfig = mobKingConfig["collider"];
    Collider collider;
    collider.width = colliderConfig["width"].get<float>();
    collider.height = colliderConfig["height"].get<float>();
    collider.isTrigger = colliderConfig["isTrigger"].get<bool>();
    ecs.addComponent(mobKingEntity, collider);

    // Initial velocity (player-controlled, starts stationary)
    Velocity velocity;
    velocity.x = 0.0f; // Player-controlled movement
    velocity.y = 0.0f;
    ecs.addComponent(mobKingEntity, velocity);

    // Create Speed component
    json speedRange = mobKingConfig["speedRange"];
    float speed = speedRange["min"].get<float>(); // Use minimum speed for more controlled movement
    Speed speedComponent;
    speedComponent.value = speed;
    ecs.addComponent(mobKingEntity, speedComponent);

    // Add Health component (boss has health)
    Health health;
    health.currentHealth = mobKingConfig["combat"]["health"].get<float>();
    health.maxHealth = health.currentHealth;
    ecs.addComponent(mobKingEntity, health);

    // Add weapon component (boss can always shoot)
    json combatConfig = mobKingConfig["combat"];
    Weapon weapon;
    weapon.damage = combatConfig["damage"].get<float>();
    weapon.range = combatConfig["range"].get<float>();
    weapon.fireRate = combatConfig["fireRate"].get<float>();
    weapon.fireTimer = 0.0f;
    weapon.canFire = true;
    weapon.ammoCount = 999; // Unlimited ammo
    weapon.maxAmmo = 999;
    ecs.addComponent(mobKingEntity, weapon);

    std::cout << "Mob King spawned at (" << transform.x << ", " << transform.y
              << ") with " << health.currentHealth << "/" << health.maxHealth << " health and combat abilities!" << std::endl;
    std::cout << "Mob King stats: Damage=" << weapon.damage << ", Range=" << weapon.range
              << ", Fire Rate=" << weapon.fireRate << std::endl;

    // Create health UI for Mob King
    createMobKingHealthUI(ecs, mobKingEntity, health);
}

void MobSpawningSystem::createMobKingHealthUI(ECS &ecs, EntityID mobKingEntity, const Health &health)
{
    // Create health UI entity
    EntityID healthUIEntity = ecs.createEntity();

    // Position in top-right corner (adjust based on screen size)
    // Screen width is typically 480, so position at x=280 (right side with some margin)
    UIPosition uiPos(280.0f, 10.0f); // Top-right corner with 10px margin
    ecs.addComponent(healthUIEntity, uiPos);

    // Create health text
    std::string healthText = "Mob King: " + std::to_string((int)health.currentHealth) + "/" + std::to_string((int)health.maxHealth);
    UIText uiText(healthText, "fonts/Xolonium-Regular.ttf", 20, {255, 255, 255, 255}, true);
    ecs.addComponent(healthUIEntity, uiText);

    // Add component to track which Mob King this UI belongs to
    MobKingHealthUI healthUITracker(mobKingEntity);
    ecs.addComponent(healthUIEntity, healthUITracker);

    std::cout << "Created Mob King health UI: " << healthText << std::endl;
}
