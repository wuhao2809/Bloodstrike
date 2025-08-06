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

    // Update spawn timer
    timeSinceLastSpawn += deltaTime;

    // Check if it's time to spawn a new mob
    if (timeSinceLastSpawn >= spawnInterval)
    {
        spawnMob(ecs);
        timeSinceLastSpawn = 0.0f;

        // Gradually decrease spawn interval to increase difficulty
        if (spawnInterval > 0.2f)
        {
            spawnInterval = std::max(0.2f, spawnInterval - 0.01f);
        }
    }
}

void MobSpawningSystem::spawnMob(ECS &ecs)
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

    // Create Speed component (random speed within range)
    json speedRange = mobConfig["speedRange"];
    float minSpeed = speedRange["min"].get<float>();
    float maxSpeed = speedRange["max"].get<float>();
    float speed = minSpeed + speedDistribution(randomGenerator) * (maxSpeed - minSpeed);

    Speed speedComponent;
    speedComponent.value = speed;
    ecs.addComponent(mobEntity, speedComponent);

    std::cout << "Spawned " << mobType << " mob at (" << spawnX << ", " << spawnY << ") with speed " << speed << std::endl;
}
