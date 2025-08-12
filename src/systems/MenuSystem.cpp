#include "MenuSystem.h"
#include "../components/Components.h"
#include <iostream>

MenuSystem::MenuSystem()
{
    keyboardState = SDL_GetKeyboardState(nullptr);
    std::cout << "MenuSystem initialized" << std::endl;
}

MenuSystem::~MenuSystem()
{
}

void MenuSystem::update(ECS &ecs, GameManager &gameManager, float deltaTime)
{
    // Handle menu when in menu state
    if (gameManager.currentState == GameManager::MENU)
    {
        // Create menu entities if they don't exist
        if (!menuEntitiesCreated)
        {
            createMenuEntities(ecs);
            menuEntitiesCreated = true;
        }
        
        handleInput(gameManager);
        updateMenuDisplay(ecs);
    }
    else
    {
        // Clean up menu entities when not in menu state
        if (menuEntitiesCreated)
        {
            cleanupMenuEntities(ecs);
            menuEntitiesCreated = false;
        }
    }
}

void MenuSystem::loadMenuConfig(const json &config)
{
    if (config.contains("menus") && config["menus"].contains("mainMenu"))
    {
        menuConfig = config["menus"]["mainMenu"];

        // Load menu options
        menuOptions.clear();
        menuActions.clear();

        if (menuConfig.contains("options"))
        {
            for (const auto &option : menuConfig["options"])
            {
                menuOptions.push_back(option["text"].get<std::string>());
                menuActions.push_back(option["action"].get<std::string>());
            }
        }

        std::cout << "Loaded " << menuOptions.size() << " menu options" << std::endl;
    }
    else
    {
        std::cerr << "Warning: No menu configuration found in JSON" << std::endl;

        // Default menu options
        menuOptions = {"Single Player", "Multiplayer", "Settings", "Quit"};
        menuActions = {"singleplayer", "multiplayer", "settings", "quit"};
    }
}

void MenuSystem::handleInput(GameManager &gameManager)
{
    // Handle menu navigation - support both arrow keys and WASD
    if ((isKeyPressed(SDL_SCANCODE_UP) || isKeyPressed(SDL_SCANCODE_W)) && !keyPressed)
    {
        selectedOption = (selectedOption - 1 + menuOptions.size()) % menuOptions.size();
        keyPressed = true;
        std::cout << "Menu selection: " << selectedOption << " (" << menuOptions[selectedOption] << ")" << std::endl;
    }
    else if ((isKeyPressed(SDL_SCANCODE_DOWN) || isKeyPressed(SDL_SCANCODE_S)) && !keyPressed)
    {
        selectedOption = (selectedOption + 1) % menuOptions.size();
        keyPressed = true;
        std::cout << "Menu selection: " << selectedOption << " (" << menuOptions[selectedOption] << ")" << std::endl;
    }
    else if (isKeyPressed(SDL_SCANCODE_RETURN) || isKeyPressed(SDL_SCANCODE_SPACE))
    {
        if (!keyPressed)
        {
            // Execute selected menu action
            if (selectedOption < menuActions.size())
            {
                executeMenuAction(menuActions[selectedOption], gameManager);
            }
            keyPressed = true;
        }
    }
    else
    {
        keyPressed = false;
    }
}

void MenuSystem::createMenuEntities(ECS &ecs)
{
    std::cout << "Creating menu entities..." << std::endl;

    // Create title entity
    EntityID titleEntity = ecs.createEntity();

    std::string titleText = "BLOODSTRIKE 2D";
    float titleX = 640.0f;  // Center X (1280/2)
    float titleY = 150.0f;  // Higher up to center the whole menu

    // Use config if available
    if (!menuConfig.empty())
    {
        if (menuConfig.contains("title"))
            titleText = menuConfig["title"].get<std::string>();
        if (menuConfig.contains("titlePosition"))
        {
            titleX = menuConfig["titlePosition"]["x"].get<float>();
            titleY = menuConfig["titlePosition"]["y"].get<float>();
        }
    }

    ecs.addComponent(titleEntity, UIText(titleText, "fonts/Xolonium-Regular.ttf", 48));
    ecs.addComponent(titleEntity, UIPosition(titleX, titleY));
    ecs.addComponent(titleEntity, EntityType("menuTitle"));

    menuEntityIDs.push_back(titleEntity);

    // Create menu option entities
    float startX = 640.0f;  // Center X (1280/2)
    float startY = 250.0f;  // Start options below title
    float spacing = 60.0f;

    // Use config if available
    if (!menuConfig.empty())
    {
        if (menuConfig.contains("startPosition"))
        {
            startX = menuConfig["startPosition"]["x"].get<float>();
            startY = menuConfig["startPosition"]["y"].get<float>();
        }
        if (menuConfig.contains("optionSpacing"))
        {
            spacing = menuConfig["optionSpacing"].get<float>();
        }
    }

    for (size_t i = 0; i < menuOptions.size(); i++)
    {
        EntityID optionEntity = ecs.createEntity();

        std::string optionText = menuOptions[i];
        if (i == selectedOption)
        {
            optionText = "> " + optionText + " <";
        }

        ecs.addComponent(optionEntity, UIText(optionText, "fonts/Xolonium-Regular.ttf", 32));
        ecs.addComponent(optionEntity, UIPosition(startX, startY + i * spacing));
        ecs.addComponent(optionEntity, EntityType("menuOption"));

        menuEntityIDs.push_back(optionEntity);
    }

    std::cout << "Created " << menuEntityIDs.size() << " menu entities" << std::endl;
}

void MenuSystem::updateMenuDisplay(ECS &ecs)
{
    // Update menu option highlighting
    static int lastSelectedOption = -1;

    if (lastSelectedOption != selectedOption)
    {
        // Find menu option entities and update their text
        auto &uiTextComponents = ecs.getComponents<UIText>();

        for (auto &[entityID, uiText] : uiTextComponents)
        {
            auto *entityType = ecs.getComponent<EntityType>(entityID);
            if (!entityType || entityType->type != "menuOption")
                continue;

            // Find which option this entity represents
            auto *uiPosition = ecs.getComponent<UIPosition>(entityID);
            if (!uiPosition)
                continue;

            float startY = 250.0f;  // Match the new default startY
            float spacing = 60.0f;

            if (!menuConfig.empty())
            {
                if (menuConfig.contains("startPosition"))
                    startY = menuConfig["startPosition"]["y"].get<float>();
                if (menuConfig.contains("optionSpacing"))
                    spacing = menuConfig["optionSpacing"].get<float>();
            }

            int optionIndex = static_cast<int>((uiPosition->y - startY) / spacing + 0.5f);

            if (optionIndex >= 0 && optionIndex < static_cast<int>(menuOptions.size()))
            {
                std::string optionText = menuOptions[optionIndex];
                if (optionIndex == selectedOption)
                {
                    optionText = "> " + optionText + " <";
                }
                uiText.content = optionText;
            }
        }

        lastSelectedOption = selectedOption;
    }
}

void MenuSystem::executeMenuAction(const std::string &action, GameManager &gameManager)
{
    std::cout << "Executing menu action: " << action << std::endl;

    if (action == "singleplayer")
    {
        std::cout << "Starting single player game..." << std::endl;
        gameManager.startGame();
    }
    else if (action == "multiplayer")
    {
        std::cout << "Multiplayer not yet implemented" << std::endl;
        // TODO: Implement multiplayer menu
    }
    else if (action == "settings")
    {
        std::cout << "Settings not yet implemented" << std::endl;
        // TODO: Implement settings menu
    }
    else if (action == "quit")
    {
        std::cout << "Quit game selected" << std::endl;
        // TODO: Set quit flag or handle game exit
    }
}

bool MenuSystem::isKeyPressed(SDL_Scancode key)
{
    return keyboardState[key] != 0;
}

void MenuSystem::cleanupMenuEntities(ECS &ecs)
{
    std::cout << "Cleaning up menu entities..." << std::endl;
    
    // Remove all menu entities
    for (EntityID entityID : menuEntityIDs)
    {
        ecs.removeEntity(entityID);
    }
    
    menuEntityIDs.clear();
    std::cout << "Menu entities cleaned up" << std::endl;
}
