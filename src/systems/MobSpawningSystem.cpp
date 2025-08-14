#include "MobSpawningSystem.h"
#include "NetworkSystem.h"
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

    // Handle dual player mode - spawn Mob King if needed (ONLY HOST)
    if (gameManager.isDualPlayer() && gameManager.shouldSpawnMobKing() && !mobKingSpawned)
    {
        // STEP 1: Only Host creates Mob King, Client will receive via network
        bool shouldCreateMobKing = true;
        if (gameManager.isMultiplayer() && networkSystem)
        {
            shouldCreateMobKing = networkSystem->isHosting(); // Only Host creates
        }

        if (shouldCreateMobKing)
        {
            spawnMobKing(ecs, gameManager);
            mobKingSpawned = true;

            // For networked multiplayer, send Mob King to client
            if (gameManager.isMultiplayer() && networkSystem)
            {
                std::cout << "[HOST] Created Mob King entity - sending to client" << std::endl;

                // Get Mob King spawn position from config
                json mobKingConfig = entityFactory->getEntityConfig()["mobs"]["mobKing"];
                json startPos = mobKingConfig["startPosition"];
                float x = startPos["x"].get<float>();
                float y = startPos["y"].get<float>();

                // Send MOB_SPAWN message to client
                networkSystem->sendMobSpawn(999, x, y, 0.0f, 0.0f, "mobKing"); // Use ID 999 for Mob King
            }
        }
        else
        {
            // Client waits for Mob King from host
            std::cout << "[CLIENT] Waiting for Mob King from host..." << std::endl;
        }
    }

    // In multiplayer mode, only the Host should spawn regular mobs locally
    // Clients will receive mob spawn messages from the Host
    if (gameManager.isMultiplayer() && networkSystem && !networkSystem->isHosting())
    {
        return; // Client: don't spawn regular mobs locally, wait for network messages
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

    // Send network update if multiplayer
    if (networkSystem && gameManager.isMultiplayer())
    {
        std::cout << "Host sending MOB_SPAWN for " << mobType << " at (" << spawnX << ", " << spawnY << ")" << std::endl;
        networkSystem->sendMobSpawn(mobEntity, spawnX, spawnY, velocity.x * finalSpeed, velocity.y * finalSpeed, mobType);
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
}

EntityID MobSpawningSystem::createMobFromNetwork(ECS &ecs, uint32_t mobID, float x, float y, float velocityX, float velocityY, const std::string &mobType)
{
    std::cout << "Client creating mob from network: " << mobType << " at (" << x << ", " << y << ") with velocity (" << velocityX << ", " << velocityY << ")" << std::endl;

    // Get mob configuration
    json mobConfig = entityFactory->getEntityConfig()["mobs"][mobType];

    // Create mob entity with specified ID (if the ECS supports it) or create new one
    EntityID mobEntity = ecs.createEntity(); // Note: ideally we'd use the provided mobID for consistency

    // Special handling for Mob King
    if (mobType == "mobKing")
    {
        // Add MobKing component (special marker for the boss)
        ecs.addComponent(mobEntity, MobKing{});
    }

    // Add MobTag component
    ecs.addComponent(mobEntity, MobTag{});

    // Add EntityType component for texture identification
    ecs.addComponent(mobEntity, EntityType{mobType});

    // Set transform from network data
    Transform transform;
    transform.x = x;
    transform.y = y;
    transform.rotation = 0.0f;
    ecs.addComponent(mobEntity, transform);

    // Set velocity from network data (already scaled by speed on host)
    // We need to extract the actual speed and normalize the velocity
    float velocityMagnitude = std::sqrt(velocityX * velocityX + velocityY * velocityY);

    Velocity velocity;
    if (velocityMagnitude > 0.0f)
    {
        // Normalize velocity direction
        velocity.x = velocityX / velocityMagnitude;
        velocity.y = velocityY / velocityMagnitude;
    }
    else
    {
        velocity.x = 0.0f;
        velocity.y = 0.0f;
    }
    ecs.addComponent(mobEntity, velocity);

    // Determine facing direction based on velocity
    MovementDirection::Direction facingDirection;
    if (std::abs(velocity.x) > std::abs(velocity.y))
    {
        facingDirection = MovementDirection::HORIZONTAL;
    }
    else
    {
        facingDirection = MovementDirection::VERTICAL;
    }
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
    }

    // Create Collider component
    json colliderConfig = mobConfig["collider"];
    Collider collider;
    collider.width = colliderConfig["width"].get<float>();
    collider.height = colliderConfig["height"].get<float>();
    collider.isTrigger = colliderConfig["isTrigger"].get<bool>();
    ecs.addComponent(mobEntity, collider);

    // Speed component - use the magnitude from network data as the final speed
    Speed speedComponent;
    speedComponent.value = velocityMagnitude; // This is the actual final speed from host
    ecs.addComponent(mobEntity, speedComponent);

    // Special components for Mob King
    if (mobType == "mobKing")
    {
        // Add Health component (boss has health)
        Health health;
        health.currentHealth = mobConfig["combat"]["health"].get<float>();
        health.maxHealth = health.currentHealth;
        ecs.addComponent(mobEntity, health);

        // Add weapon component (boss can always shoot)
        json combatConfig = mobConfig["combat"];
        Weapon weapon;
        weapon.damage = combatConfig["damage"].get<float>();
        weapon.range = combatConfig["range"].get<float>();
        weapon.fireRate = combatConfig["fireRate"].get<float>();
        weapon.fireTimer = 0.0f;
        weapon.canFire = true;
        weapon.ammoCount = 999; // Unlimited ammo
        weapon.maxAmmo = 999;
        ecs.addComponent(mobEntity, weapon);

        std::cout << "[CLIENT] Created Mob King with " << health.currentHealth << "/" << health.maxHealth
                  << " health and combat abilities!" << std::endl;
    }

    // Note: Weapon components will be added based on game state timing by other systems

    std::cout << "Network mob created with speed=" << speedComponent.value
              << ", normalized velocity=(" << velocity.x << ", " << velocity.y << ")" << std::endl;

    return mobEntity;
}
