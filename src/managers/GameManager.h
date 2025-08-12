#pragma once
#include <iostream>

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

    GameState currentState = MENU;
    GameMode currentGameMode = SINGLE_PLAYER;
    int score = 0;
    float gameTime = 0.0f;
    float accumulatedScore = 0.0f; // Track fractional score accumulation
    bool needsPlayerReset = false; // Flag to indicate player state should be reset

    // Level system
    int currentLevel = 1;
    float levelTime = 0.0f;           // Time spent in current level
    const float levelDuration = 5.0f; // 10 seconds per level
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
        needsPlayerReset = true; // Request player state reset
    }

    void startDualPlayerGame()
    {
        currentState = PLAYING;
        currentGameMode = DUAL_PLAYER_LOCAL;
        score = 0;
        gameTime = 0.0f;
        accumulatedScore = 0.0f;
        currentLevel = 1;
        levelTime = 0.0f;
        needsPlayerReset = true; // Request player state reset
        std::cout << "Starting Dual Player Mode - Player vs Mob King!" << std::endl;
    }

    void gameOver()
    {
        currentState = GAME_OVER;
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

            // Check if level is complete (10 seconds)
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
    float getLevelSpawnInterval() const
    {
        switch (currentLevel)
        {
        case 1:
            return 1.0f; // Level 1: Slower spawning (tutorial)
        case 2:
            return 0.4f; // Level 2: Faster spawning
        case 3:
            return 0.4f; // Level 3: Same as level 2
        case 4:
            return 0.3f; // Level 4: Very fast spawning + shooting mobs
        default:
            return 0.5f;
        }
    }

    // Get level-specific mob speed multiplier
    float getLevelSpeedMultiplier() const
    {
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
    bool canMobsShoot() const
    {
        return currentLevel >= 4;
    }

    // Get remaining time in current level
    float getLevelTimeRemaining() const
    {
        return levelDuration - levelTime;
    }

    // Game mode helpers
    bool isSinglePlayer() const { return currentGameMode == SINGLE_PLAYER; }
    bool isDualPlayer() const { return currentGameMode == DUAL_PLAYER_LOCAL; }
    bool isMultiplayer() const { return currentGameMode == MULTIPLAYER_ONLINE; }

    // Dual player specific settings
    bool shouldSpawnMobKing() const
    {
        return isDualPlayer() && currentLevel >= 2; // Mob King appears from level 2 in dual player
    }

    // Get mob spawn settings based on game mode
    float getGameModeSpawnInterval() const
    {
        if (isDualPlayer())
        {
            // Dual player: fewer regular mobs since we have Mob King
            return getLevelSpawnInterval() * 2.0f;
        }
        return getLevelSpawnInterval();
    }
};