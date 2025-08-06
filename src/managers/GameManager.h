#pragma once
#include <iostream>

class GameManager
{
public:
    enum GameState
    {
        MENU,
        PLAYING,
        GAME_OVER
    };

    GameState currentState = MENU;
    int score = 0;
    float gameTime = 0.0f;
    float accumulatedScore = 0.0f; // Track fractional score accumulation
    bool needsPlayerReset = false; // Flag to indicate player state should be reset

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
        currentState = MENU;
    }

    void startGame()
    {
        currentState = PLAYING;
        score = 0;
        gameTime = 0.0f;
        accumulatedScore = 0.0f;
        needsPlayerReset = true; // Request player state reset
    }

    void gameOver()
    {
        currentState = GAME_OVER;
    }

    void updateGameTime(float deltaTime)
    {
        if (currentState == PLAYING)
        {
            gameTime += deltaTime;
            int previousScore = score;

            // Accumulate fractional score to avoid truncation
            accumulatedScore += scorePerSecond * deltaTime;
            score = static_cast<int>(accumulatedScore);

            // Debug output every second
            static float debugTimer = 0.0f;
            debugTimer += deltaTime;
            if (debugTimer >= 1.0f)
            {
                std::cout << "Score update: " << previousScore << " -> " << score
                          << " (deltaTime: " << deltaTime << ", scorePerSecond: " << scorePerSecond
                          << ", accumulatedScore: " << accumulatedScore << ")" << std::endl;
                debugTimer = 0.0f;
            }
        }
    }
};
