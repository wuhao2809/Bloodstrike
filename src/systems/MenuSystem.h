#pragma once
#include "System.h"
#include "../managers/GameManager.h"
#include <SDL2/SDL.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

using json = nlohmann::json;

class MenuSystem : public System
{
private:
    const Uint8 *keyboardState;
    bool keyPressed = false;

    // Menu state
    int selectedOption = 0;
    std::vector<std::string> menuOptions;
    std::vector<std::string> menuActions;
    std::vector<EntityID> menuEntityIDs;
    bool menuEntitiesCreated = false;

    // Menu configuration
    json menuConfig;

public:
    MenuSystem();
    ~MenuSystem();

    void update(ECS &ecs, GameManager &gameManager, float deltaTime) override;
    void loadMenuConfig(const json &config);
    void cleanupMenuEntities(ECS &ecs);

private:
    void handleInput(GameManager &gameManager);
    void createMenuEntities(ECS &ecs);
    void updateMenuDisplay(ECS &ecs);
    void executeMenuAction(const std::string &action, GameManager &gameManager);
    bool isKeyPressed(SDL_Scancode key);
};
