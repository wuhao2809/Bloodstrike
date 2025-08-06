#pragma once
#include "System.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>

class ResourceManager; // Forward declaration

class RenderSystem : public System
{
private:
    SDL_Renderer *renderer;
    ResourceManager *resourceManager;

public:
    RenderSystem(SDL_Renderer *renderer, ResourceManager *rm);
    void update(ECS &ecs, GameManager &gameManager, float fps);

private:
    void renderSprites(ECS &ecs);
    void renderUI(ECS &ecs, GameManager &gameManager, float fps);
    void renderAimingLines(ECS &ecs);
    void renderProjectiles(ECS &ecs);
    void renderCrosshair(ECS &ecs);
    std::vector<std::string> wrapText(const std::string &text, TTF_Font *font, int maxWidth);
};
