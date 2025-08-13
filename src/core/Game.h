#pragma once
#include "ECS.h"
#include "../managers/GameManager.h"
#include "../systems/Systems.h"
#include "../managers/EntityFactory.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <memory>

class ResourceManager; // Forward declaration

class Game
{
private:
    // Core systems
    ECS ecs;
    GameManager gameManager;

    // SDL components
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool running;

    // Resource management
    std::unique_ptr<ResourceManager> resourceManager;
    std::unique_ptr<EntityFactory> entityFactory;

    // Systems (order matters for execution)
    std::unique_ptr<MenuSystem> menuSystem;
    std::unique_ptr<TimingSystem> timingSystem;
    std::unique_ptr<InputSystem> inputSystem;
    std::unique_ptr<MovementSystem> movementSystem;
    std::unique_ptr<AnimationSystem> animationSystem;
    std::unique_ptr<AudioSystem> audioSystem;
    std::unique_ptr<MobSpawningSystem> mobSpawningSystem;
    std::unique_ptr<CollisionSystem> collisionSystem;
    std::unique_ptr<BoundarySystem> boundarySystem;
    std::unique_ptr<RenderSystem> renderSystem;

    // Bloodstrike 2D Combat Systems
    std::unique_ptr<AimingSystem> aimingSystem;
    std::unique_ptr<WeaponSystem> weaponSystem;
    std::unique_ptr<ProjectileSystem> projectileSystem;

    // Bloodstrike 2D Networking Systems
    std::unique_ptr<NetworkSystem> networkSystem;
    
    // Bloodstrike 2D UI Systems
    std::unique_ptr<HealthUISystem> healthUISystem;

    // TODO: Systems to be implemented in Phase 4
    // std::unique_ptr<HudSystem> hudSystem;
    // std::unique_ptr<CleanupSystem> cleanupSystem;    // Entity IDs
    EntityID playerEntityID;

public:
    Game();
    ~Game();

    bool initialize();
    void run();
    void shutdown();

private:
    bool loadAssets();
    bool loadAudioAssets();
    void createInitialEntities();
    void gameLoop();
    void handleEvents();
    void updateUI();
    void resetPlayerState();
};
