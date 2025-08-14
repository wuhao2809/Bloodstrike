#pragma once
#include <iostream>

// Forward declare GameSettings to avoid circular dependency
class GameSettings;

class GameManager
{
public:
    enum GameState
    {
        MENU,
        PLAYING,
        LEVEL_COMPLETE,
        GAME_OVER
    };

    enum GameMode
    {
        SINGLE_PLAYER,
        DUAL_PLAYER_LOCAL,
        MULTIPLAYER_ONLINE
    };

    enum Winner
    {
        NONE,
        PLAYER,
        MOB_KING,
        TIME_UP
    };

    GameState currentState = MENU;
    GameMode currentGameMode = SINGLE_PLAYER;
    Winner gameWinner = NONE;
    int score = 0;
    float gameTime = 0.0f;
    float accumulatedScore = 0.0f; // Track fractional score accumulation
    bool needsPlayerReset = false; // Flag to indicate player state should be reset

    // Level system
    int currentLevel = 1;
    float levelTime = 0.0f;     // Time spent in current level
    float levelDuration = 5.0f; // 5 seconds per level (default, can be modified for multiplayer)
    const int maxLevel = 4;

    // Screen bounds
    float screenWidth = 480;
    float screenHeight = 720;

    // Game settings (will be loaded from JSON later)
    float mobSpawnInterval = 0.5f;
    float scorePerSecond = 10.0f;

    void reset()
    {
        score = 0;
        gameTime = 0.0f;
        accumulatedScore = 0.0f;
        currentLevel = 1;
        levelTime = 0.0f;
        currentState = MENU;
        currentGameMode = SINGLE_PLAYER;
        gameWinner = NONE;
    }

    void startGame()
    {
        currentState = PLAYING;
        currentGameMode = SINGLE_PLAYER;
        score = 0;
        gameTime = 0.0f;
        accumulatedScore = 0.0f;
        currentLevel = 1;
        levelTime = 0.0f;
        gameWinner = NONE;
        needsPlayerReset = true; // Request player state reset
    }

    void startDualPlayerGame();

    void startNetworkedMultiplayerGame();

    void gameOver(Winner winner = NONE)
    {
        currentState = GAME_OVER;
        gameWinner = winner;
    }

    void continueToNextLevel()
    {
        if (currentState == LEVEL_COMPLETE && currentLevel < maxLevel)
        {
            currentLevel++;
            levelTime = 0.0f;
            currentState = PLAYING;
            needsPlayerReset = true; // Clear any remaining projectiles and reset player
            std::cout << "Advanced to Level " << currentLevel << std::endl;
        }
    }

    void updateGameTime(float deltaTime)
    {
        if (currentState == PLAYING)
        {
            gameTime += deltaTime;
            levelTime += deltaTime;

            // Special handling for dual player and multiplayer modes
            if (currentGameMode == DUAL_PLAYER_LOCAL || currentGameMode == MULTIPLAYER_ONLINE)
            {
                // Dual/Multiplayer: 120-second countdown, game ends when time is up
                if (levelTime >= levelDuration)
                {
                    std::cout << "Time up! Player Wins!" << std::endl;
                    gameOver(PLAYER); // Player wins when time runs out
                    return;
                }
            }
            else
            {
                // Original single/dual player logic
                if (levelTime >= levelDuration)
                {
                    if (currentLevel < maxLevel)
                    {
                        // Show level complete screen
                        currentState = LEVEL_COMPLETE;
                        std::cout << "Level " << currentLevel << " Complete! Press SPACE to continue or R to restart" << std::endl;
                    }
                    else
                    {
                        // Completed all levels - victory!
                        std::cout << "Congratulations! You completed all levels!" << std::endl;
                        gameOver();
                    }
                }
            }

            int previousScore = score;

            // Accumulate fractional score to avoid truncation
            accumulatedScore += scorePerSecond * deltaTime;
            score = static_cast<int>(accumulatedScore);

            // Debug output every second
            static float debugTimer = 0.0f;
            debugTimer += deltaTime;
            if (debugTimer >= 1.0f)
            {
                std::cout << "Level " << currentLevel << " - Time: " << (int)(levelDuration - levelTime)
                          << "s remaining - Score: " << score << std::endl;
                debugTimer = 0.0f;
            }
        }
    }

    // Get level-specific mob spawn interval
    float getLevelSpawnInterval() const;

    // Get level-specific mob speed multiplier
    float getLevelSpeedMultiplier() const
    {
        if (isDualPlayer())
        {
            // Use dynamic speed multiplier for dual/multiplayer modes
            return getMobSpeedMultiplier();
        }

        // Original level-based system for single player
        switch (currentLevel)
        {
        case 1:
            return 0.7f; // Level 1: 70% normal speed
        case 2:
            return 1.0f; // Level 2: Normal speed
        case 3:
            return 1.5f; // Level 3: 150% speed
        case 4:
            return 1.3f; // Level 4: 130% speed + shooting
        default:
            return 1.0f;
        }
    }

    // Check if mobs can shoot in current level
    bool canMobsShoot() const;

    // Get remaining time in current level
    float getLevelTimeRemaining() const
    {
        return levelDuration - levelTime;
    }

    // Game mode helpers
    bool isSinglePlayer() const { return currentGameMode == SINGLE_PLAYER; }
    bool isDualPlayer() const { return currentGameMode == DUAL_PLAYER_LOCAL || currentGameMode == MULTIPLAYER_ONLINE; }
    bool isMultiplayer() const { return currentGameMode == MULTIPLAYER_ONLINE; }

    // Dual player specific settings
    bool shouldSpawnMobKing() const
    {
        if (currentGameMode == DUAL_PLAYER_LOCAL || currentGameMode == MULTIPLAYER_ONLINE)
        {
            // Dual/Multiplayer: Mob King spawns immediately
            return true;
        }
        return false; // No Mob King in single player mode
    }

    // Get mob spawn settings based on game mode
    float getGameModeSpawnInterval() const
    {
        if (isDualPlayer())
        {
            // Dual/Multiplayer: dynamic spawn interval that gets faster over time
            return getDynamicSpawnInterval();
        }
        return getLevelSpawnInterval();
    }

    // Get dynamic spawn interval for dual/multiplayer modes
    float getDynamicSpawnInterval() const;

    // Get dynamic mob speed multiplier for dual/multiplayer modes
    float getMobSpeedMultiplier() const;
};