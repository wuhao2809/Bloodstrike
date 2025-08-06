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
    if (!initializeSDL())
    {
        return false;
    }

    // Initialize resource manager
    resourceManager = std::make_unique<ResourceManager>(renderer);

    // Initialize entity factory
    entityFactory = std::make_unique<EntityFactory>(resourceManager.get());

    // Load entity configuration
    if (!entityFactory->loadConfig("entities.json"))
    {
        std::cerr << "Failed to load entity configuration" << std::endl;
        return false;
    }

    // Load game settings from JSON
    json gameSettings = entityFactory->getGameSettings();
    gameManager.screenWidth = gameSettings["screenSize"]["width"].get<float>();
    gameManager.screenHeight = gameSettings["screenSize"]["height"].get<float>();
    gameManager.mobSpawnInterval = gameSettings["mobSpawnInterval"].get<float>();
    gameManager.scorePerSecond = gameSettings["scorePerSecond"].get<float>();

    // Initialize systems
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

bool Game::initializeSDL()
{
    // Initialize SDL
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

    // Create window
    window = SDL_CreateWindow("Dodge the Creeps",
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
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
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
    // Create player entity
    playerEntityID = entityFactory->createPlayer(ecs);

    // Create UI entities
    entityFactory->createUIElement(ecs, "scoreDisplay");
    entityFactory->createUIElement(ecs, "fpsDisplay");
    entityFactory->createUIElement(ecs, "gameMessage");
}

void Game::gameLoop()
{
    // 1. Update timing and calculate delta time
    float deltaTime = timingSystem->update();

    // 2. Handle input
    inputSystem->update(ecs, gameManager, deltaTime);

    // 3. Update game logic (only if playing)
    if (gameManager.currentState == GameManager::PLAYING)
    {
        movementSystem->update(ecs, deltaTime);
        animationSystem->update(ecs, deltaTime);
        audioSystem->update(ecs, gameManager, deltaTime);

        // Update game time and score
        gameManager.updateGameTime(deltaTime);

        mobSpawningSystem->update(ecs, gameManager, deltaTime);
        collisionSystem->update(ecs, gameManager, deltaTime);
        boundarySystem->update(ecs, gameManager, deltaTime);
    }

    // 4. Update UI (update text content)
    updateUI();

    // 5. Render everything
    renderSystem->update(ecs, gameManager, timingSystem->getFPS());

    // 6. Frame limiting to maintain 60 FPS
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
        else if (entityType->type == "gameMessage")
        {
            switch (gameManager.currentState)
            {
            case GameManager::MENU:
                uiText.content = "Dodge the Creeps! Press SPACE to start";
                uiText.visible = true;
                break;
            case GameManager::PLAYING:
                uiText.visible = false;
                break;
            case GameManager::GAME_OVER:
                uiText.content = "Game Over! Press SPACE to restart";
                uiText.visible = true;
                break;
            }
        }
    }
}
