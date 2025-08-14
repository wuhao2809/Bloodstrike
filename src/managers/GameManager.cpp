#include "GameManager.h"
#include "GameSettings.h"

void GameManager::startDualPlayerGame()
{
    currentState = PLAYING;
    currentGameMode = DUAL_PLAYER_LOCAL;
    score = 0;
    gameTime = 0.0f;
    accumulatedScore = 0.0f;
    currentLevel = 1;
    levelTime = 0.0f;
    gameWinner = NONE;

    // Get duration from GameSettings
    const GameSettings &settings = GameSettings::getInstance();
    levelDuration = settings.getDualPlayerLevelDuration();

    needsPlayerReset = true; // Request player state reset
    std::cout << "Starting Dual Player Mode - " << levelDuration << "s battle!" << std::endl;
}

void GameManager::startNetworkedMultiplayerGame()
{
    currentState = PLAYING;
    currentGameMode = MULTIPLAYER_ONLINE;
    score = 0;
    gameTime = 0.0f;
    accumulatedScore = 0.0f;
    currentLevel = 1;
    levelTime = 0.0f;

    // Get duration from GameSettings - use multiplayer-specific settings
    const GameSettings &settings = GameSettings::getInstance();
    levelDuration = settings.getMultiplayerLevelDuration();

    needsPlayerReset = true; // Request player state reset
    std::cout << "Starting Networked Multiplayer Mode - " << levelDuration << "s battle!" << std::endl;
}

float GameManager::getMobSpeedMultiplier() const
{
    const GameSettings &settings = GameSettings::getInstance();

    if (currentGameMode == DUAL_PLAYER_LOCAL)
    {
        // Dual Player: Speed increases from start to end over the level duration
        float progress = levelTime / levelDuration; // 0.0 to 1.0
        float startSpeed = settings.getDualPlayerStartSpeedMultiplier();
        float endSpeed = settings.getDualPlayerEndSpeedMultiplier();
        return startSpeed + (progress * (endSpeed - startSpeed));
    }
    else if (currentGameMode == MULTIPLAYER_ONLINE)
    {
        // Multiplayer: Speed increases from start to end over the level duration
        float progress = levelTime / levelDuration; // 0.0 to 1.0
        float startSpeed = settings.getMultiplayerStartSpeedMultiplier();
        float endSpeed = settings.getMultiplayerEndSpeedMultiplier();
        return startSpeed + (progress * (endSpeed - startSpeed));
    }

    // Single player mode - get speed for current level
    return settings.getLevelMobSpeedMultiplier(currentLevel);
}

bool GameManager::canMobsShoot() const
{
    const GameSettings &settings = GameSettings::getInstance();

    if (currentGameMode == DUAL_PLAYER_LOCAL)
    {
        // Dual Player: mobs can shoot in last X seconds (from dual player settings)
        return (levelDuration - levelTime) <= settings.getDualPlayerMobShootingLastSeconds();
    }
    else if (currentGameMode == MULTIPLAYER_ONLINE)
    {
        // Multiplayer: mobs can shoot in last X seconds (from multiplayer settings)
        return (levelDuration - levelTime) <= settings.getMultiplayerMobShootingLastSeconds();
    }

    return settings.canMobsShootAtLevel(currentLevel);
}

float GameManager::getLevelSpawnInterval() const
{
    const GameSettings &settings = GameSettings::getInstance();
    return settings.getLevelSpawnInterval(currentLevel);
}

float GameManager::getDynamicSpawnInterval() const
{
    const GameSettings &settings = GameSettings::getInstance();

    if (currentGameMode == DUAL_PLAYER_LOCAL)
    {
        // Dual Player: Spawn interval decreases from start to end over the level duration
        float progress = levelTime / levelDuration; // 0.0 to 1.0
        float startInterval = settings.getDualPlayerStartSpawnInterval();
        float endInterval = settings.getDualPlayerEndSpawnInterval();
        return startInterval - (progress * (startInterval - endInterval));
    }
    else if (currentGameMode == MULTIPLAYER_ONLINE)
    {
        // Multiplayer: Spawn interval decreases from start to end over the level duration
        float progress = levelTime / levelDuration; // 0.0 to 1.0
        float startInterval = settings.getMultiplayerStartSpawnInterval();
        float endInterval = settings.getMultiplayerEndSpawnInterval();
        return startInterval - (progress * (startInterval - endInterval));
    }

    return 0.5f; // Default interval for single player
}
