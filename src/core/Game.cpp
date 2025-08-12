#include "Game.h"
#include "../systems/Systems.h"
#include "../managers/ResourceManager.h"
#include "../managers/EntityFactory.h"
#include <iostream>

Game::Game()
    : window(nullptr), renderer(nullptr), running(false), playerEntityID(0) {}

Game::~Game()
{
    shutdown();
}

bool Game::initialize()
{
    // Initialize basic SDL first (without window)
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1)
    {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    // Load entity configuration FIRST to get screen size
    entityFactory = std::make_unique<EntityFactory>(nullptr); // Temporary without renderer
    if (!entityFactory->loadConfig("entities.json"))
    {
        std::cerr << "Failed to load entity configuration" << std::endl;
        return false;
    }

    // Load game settings from JSON BEFORE creating window
    json gameSettings = entityFactory->getGameSettings();
    gameManager.screenWidth = gameSettings["screenSize"]["width"].get<float>();
    gameManager.screenHeight = gameSettings["screenSize"]["height"].get<float>();
    gameManager.mobSpawnInterval = gameSettings["mobSpawnInterval"].get<float>();
    gameManager.scorePerSecond = gameSettings["scorePerSecond"].get<float>();

    // NOW create window with correct size
    window = SDL_CreateWindow("Bloodstrike 2D",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              static_cast<int>(gameManager.screenWidth),
                              static_cast<int>(gameManager.screenHeight),
                              SDL_WINDOW_SHOWN);

    if (!window)
    {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize resource manager
    resourceManager = std::make_unique<ResourceManager>(renderer);

    // Re-initialize entity factory with proper renderer
    entityFactory = std::make_unique<EntityFactory>(resourceManager.get());

    // Reload config with proper resource manager
    if (!entityFactory->loadConfig("entities.json"))
    {
        std::cerr << "Failed to reload entity configuration" << std::endl;
        return false;
    }

    // Initialize systems
    menuSystem = std::make_unique<MenuSystem>();
    timingSystem = std::make_unique<TimingSystem>();
    inputSystem = std::make_unique<InputSystem>();
    movementSystem = std::make_unique<MovementSystem>();
    animationSystem = std::make_unique<AnimationSystem>();
    audioSystem = std::make_unique<AudioSystem>();
    mobSpawningSystem = std::make_unique<MobSpawningSystem>(entityFactory.get(),
                                                            gameManager.screenWidth,
                                                            gameManager.screenHeight);
    collisionSystem = std::make_unique<CollisionSystem>(audioSystem.get());
    boundarySystem = std::make_unique<BoundarySystem>(gameManager.screenWidth,
                                                      gameManager.screenHeight);
    renderSystem = std::make_unique<RenderSystem>(renderer, resourceManager.get());

    // Load menu configuration for MenuSystem
    json fullConfig = entityFactory->getEntityConfig();
    menuSystem->loadMenuConfig(fullConfig);

    // Initialize Bloodstrike 2D combat systems
    aimingSystem = std::make_unique<AimingSystem>();
    weaponSystem = std::make_unique<WeaponSystem>(entityFactory.get(), audioSystem.get());
    projectileSystem = std::make_unique<ProjectileSystem>();

    // Initialize Bloodstrike 2D networking systems
    networkSystem = std::make_unique<NetworkSystem>();
    
    // Connect MenuSystem with NetworkSystem
    menuSystem->setNetworkSystem(networkSystem.get());

    // Initialize audio system
    if (!audioSystem->initialize())
    {
        std::cerr << "Failed to initialize audio system" << std::endl;
        return false;
    }

    // Load audio assets
    if (!loadAudioAssets())
    {
        std::cerr << "Failed to load audio assets" << std::endl;
        return false;
    }

    // Create initial entities
    createInitialEntities();

    running = true;
    return true;
}

void Game::run()
{
    while (running)
    {
        handleEvents();
        gameLoop();
    }
}

void Game::shutdown()
{
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

bool Game::loadAudioAssets()
{
    json fullConfig = entityFactory->getEntityConfig();
    if (!fullConfig.contains("audio"))
    {
        std::cerr << "No audio configuration found in entities.json" << std::endl;
        return true; // Not critical, continue without audio
    }

    json audio = fullConfig["audio"];

    // Load background music
    if (audio.contains("backgroundMusic"))
    {
        json bgMusic = audio["backgroundMusic"];
        std::string name = bgMusic["name"].get<std::string>();
        std::string file = bgMusic["file"].get<std::string>();

        if (!audioSystem->loadMusic(name, file))
        {
            std::cerr << "Failed to load background music: " << file << std::endl;
        }
    }

    // Load sound effects
    if (audio.contains("soundEffects"))
    {
        for (auto &[key, sfx] : audio["soundEffects"].items())
        {
            std::string name = sfx["name"].get<std::string>();
            std::string file = sfx["file"].get<std::string>();

            if (!audioSystem->loadSoundEffect(name, file))
            {
                std::cerr << "Failed to load sound effect: " << file << std::endl;
            }
        }
    }

    // Set volume levels
    if (audio.contains("settings"))
    {
        json settings = audio["settings"];
        if (settings.contains("musicVolume"))
        {
            audioSystem->setMusicVolume(settings["musicVolume"].get<int>());
        }
        if (settings.contains("sfxVolume"))
        {
            audioSystem->setSFXVolume(settings["sfxVolume"].get<int>());
        }
    }

    return true;
}

void Game::createInitialEntities()
{
    // Create player entity (combat components loaded from JSON)
    playerEntityID = entityFactory->createPlayer(ecs);

    // Create UI entities
    entityFactory->createUIElement(ecs, "scoreDisplay");
    entityFactory->createUIElement(ecs, "fpsDisplay");
    entityFactory->createUIElement(ecs, "ammoDisplay");
    entityFactory->createUIElement(ecs, "levelDisplay");
    entityFactory->createUIElement(ecs, "gameMessage");
}

void Game::gameLoop()
{
    // 1. Update timing and calculate delta time
    float deltaTime = timingSystem->update();

    // 2. Handle menu system (always active)
    menuSystem->update(ecs, gameManager, deltaTime);

    // 2.5. Handle networking system (always active)
    networkSystem->update(ecs, gameManager, deltaTime);

    // 3. Handle input
    inputSystem->update(ecs, gameManager, deltaTime);

    // 3.5. Check if player state needs to be reset (after game restart)
    if (gameManager.needsPlayerReset)
    {
        resetPlayerState();
        gameManager.needsPlayerReset = false; // Clear the flag
    }

    // 4. Update game logic (only if playing)
    if (gameManager.currentState == GameManager::PLAYING)
    {
        // Movement and animation
        movementSystem->update(ecs, deltaTime);
        animationSystem->update(ecs, deltaTime);
        audioSystem->update(ecs, gameManager, deltaTime);

        // Update game time and score
        gameManager.updateGameTime(deltaTime);

        // Combat systems
        aimingSystem->update(ecs, gameManager, deltaTime);
        weaponSystem->update(ecs, gameManager, deltaTime);
        projectileSystem->update(ecs, gameManager, deltaTime);

        // Mob and collision systems
        mobSpawningSystem->update(ecs, gameManager, deltaTime);
        collisionSystem->update(ecs, gameManager, deltaTime);
        boundarySystem->update(ecs, gameManager, deltaTime);
    }

    // 5. Update UI (update text content)
    updateUI();

    // 6. Render everything
    renderSystem->update(ecs, gameManager, timingSystem->getFPS());

    // 7. Frame limiting to maintain 60 FPS
    timingSystem->limitFrameRate();
}

void Game::handleEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
        {
            running = false;
        }

        // Handle escape key for quitting
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
        {
            running = false;
        }
    }
}

void Game::updateUI()
{
    // Update score display
    auto &uiTextComponents = ecs.getComponents<UIText>();
    for (auto &[entityID, uiText] : uiTextComponents)
    {
        auto *entityType = ecs.getComponent<EntityType>(entityID);
        if (!entityType)
            continue;

        if (entityType->type == "scoreDisplay")
        {
            uiText.content = "Score: " + std::to_string(gameManager.score);
        }
        else if (entityType->type == "fpsDisplay")
        {
            uiText.content = "FPS: " + std::to_string(static_cast<int>(timingSystem->getFPS()));
        }
        else if (entityType->type == "ammoDisplay")
        {
            // Find player's weapon to get ammo count
            int currentAmmo = 0;
            int maxAmmo = 30; // Default

            auto &playerTags = ecs.getComponents<PlayerTag>();
            for (auto &[playerEntityID, playerTag] : playerTags)
            {
                Weapon *weapon = ecs.getComponent<Weapon>(playerEntityID);
                if (weapon)
                {
                    currentAmmo = weapon->ammoCount;
                    maxAmmo = weapon->maxAmmo;
                    break;
                }
            }

            uiText.content = "Ammo: " + std::to_string(currentAmmo) + "/" + std::to_string(maxAmmo);
        }
        else if (entityType->type == "levelDisplay")
        {
            int remainingTime = static_cast<int>(gameManager.levelDuration - gameManager.levelTime);
            remainingTime = std::max(0, remainingTime); // Don't show negative time
            uiText.content = "Level: " + std::to_string(gameManager.currentLevel) +
                             " - Time: " + std::to_string(remainingTime) + "s";
        }
        else if (entityType->type == "gameMessage")
        {
            switch (gameManager.currentState)
            {
            case GameManager::MENU:
                // Hide game message during menu - MenuSystem handles the menu UI
                uiText.visible = false;
                break;
            case GameManager::PLAYING:
                uiText.visible = false;
                break;
            case GameManager::LEVEL_COMPLETE:
                uiText.content = "Level " + std::to_string(gameManager.currentLevel) + " Complete! SPACE: Continue | R: Restart";
                uiText.visible = true;
                break;
            case GameManager::GAME_OVER:
                uiText.content = "Game Over! Press SPACE to restart";
                uiText.visible = true;
                break;
            }
        }
    }
}

void Game::resetPlayerState()
{
    // Reset player's weapon ammo when game restarts
    auto &playerTags = ecs.getComponents<PlayerTag>();
    for (auto &[playerEntityID, playerTag] : playerTags)
    {
        Weapon *weapon = ecs.getComponent<Weapon>(playerEntityID);
        if (weapon)
        {
            weapon->ammoCount = weapon->maxAmmo; // Reset to full ammo
            weapon->fireTimer = 0.0f;            // Ready to fire immediately
            weapon->canFire = true;              // Enable firing
        }

        // Reset player position to start position
        Transform *transform = ecs.getComponent<Transform>(playerEntityID);
        if (transform)
        {
            // Get start position from JSON configuration
            json playerConfig = entityFactory->getEntityConfig()["player"];
            if (playerConfig.contains("startPosition"))
            {
                transform->x = playerConfig["startPosition"]["x"].get<float>();
                transform->y = playerConfig["startPosition"]["y"].get<float>();
            }
        }

        // Reset player velocity
        Velocity *velocity = ecs.getComponent<Velocity>(playerEntityID);
        if (velocity)
        {
            velocity->x = 0.0f;
            velocity->y = 0.0f;
        }
    }
}
