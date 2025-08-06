#include "TimingSystem.h"
#include <thread>

TimingSystem::TimingSystem()
    : frameCount(0), currentFPS(60.0f)
{
    lastTime = std::chrono::high_resolution_clock::now();
    fpsCounterTime = lastTime;
}

float TimingSystem::update()
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;

    // Update FPS counter
    frameCount++;
    auto fpsElapsed = std::chrono::duration<float>(currentTime - fpsCounterTime).count();
    if (fpsElapsed >= 1.0f)
    {
        currentFPS = frameCount / fpsElapsed;
        frameCount = 0;
        fpsCounterTime = currentTime;
    }

    return deltaTime;
}

void TimingSystem::limitFrameRate()
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto frameTime = std::chrono::duration<float>(currentTime - lastTime).count();

    if (frameTime < targetFrameTime)
    {
        float sleepTime = targetFrameTime - frameTime;
        std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
    }
}
