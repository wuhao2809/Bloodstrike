#pragma once
#include "System.h"
#include <chrono>

class TimingSystem : public System
{
private:
    std::chrono::high_resolution_clock::time_point lastTime;
    std::chrono::high_resolution_clock::time_point fpsCounterTime;
    int frameCount;
    float currentFPS;
    const float targetFPS = 60.0f;
    const float targetFrameTime = 1.0f / targetFPS;

public:
    TimingSystem();

    // Returns delta time in seconds
    float update();

    // Limit frame rate to 60 FPS
    void limitFrameRate();

    // Get current FPS for display
    float getFPS() const { return currentFPS; }

    // Get target frame time
    float getTargetFrameTime() const { return targetFrameTime; }
};
