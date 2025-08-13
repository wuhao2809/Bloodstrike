#pragma once
#include "System.h"
#include "../managers/GameManager.h"
#include <SDL2/SDL.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

using json = nlohmann::json;

enum class MenuState
{
    MAIN_MENU,
    MULTIPLAYER_MENU,
    LOBBY_WAITING,   // Waiting for connection
    LOBBY_CONNECTED, // Connected, showing ready states
    LOBBY_COUNTDOWN  // Both ready, countdown to start
};

class MenuSystem : public System
{
private:
    const Uint8 *keyboardState;
    bool keyPressed = false;

    // Menu state management
    MenuState currentMenuState = MenuState::MAIN_MENU;
    int selectedOption = 0;
    std::vector<std::string> menuOptions;
    std::vector<std::string> menuActions;
    std::vector<EntityID> menuEntityIDs;

    // Network state tracking
    int lastNetworkState = 0; // Store as int to avoid forward declaration issues
    bool menuEntitiesCreated = false;
    bool localPlayerReady = false;

    // Menu configuration
    json menuConfig;
    json allMenuConfigs;

    // Network reference (will be set by Game)
    class NetworkSystem *networkSystem = nullptr;

public:
    MenuSystem();
    ~MenuSystem();

    void update(ECS &ecs, GameManager &gameManager, float deltaTime) override;
    void loadMenuConfig(const json &config);
    void cleanupMenuEntities(ECS &ecs);
    void setNetworkSystem(NetworkSystem *network) { networkSystem = network; }

private:
    void handleInput(GameManager &gameManager);
    void createMenuEntities(ECS &ecs);
    void updateMenuDisplay(ECS &ecs);
    void executeMenuAction(const std::string &action, GameManager &gameManager);
    bool isKeyPressed(SDL_Scancode key);
    void loadCurrentMenuConfig();

    // Menu state management
    void switchToMainMenu(ECS &ecs);
    void switchToMultiplayerMenu(ECS &ecs);
    void switchToLobbyWaiting(ECS &ecs);
    void switchToLobbyConnected(ECS &ecs);
    void updateLobbyStatus(ECS &ecs);
    void createLobbyUI(ECS &ecs);
    void updateLobbyUI(ECS &ecs);
};
