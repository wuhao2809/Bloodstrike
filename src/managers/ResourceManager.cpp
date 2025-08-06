#include "ResourceManager.h"
#include <iostream>

ResourceManager::ResourceManager(SDL_Renderer *renderer) : renderer(renderer) {}

ResourceManager::~ResourceManager()
{
    cleanup();
}

SDL_Texture *ResourceManager::loadTexture(const std::string &path)
{
    // Check if texture is already loaded
    auto it = textures.find(path);
    if (it != textures.end())
    {
        return it->second;
    }

    // Construct full path to asset
    std::string fullPath = std::string(ASSET_PATH) + path;

    // Load new texture
    SDL_Texture *texture = IMG_LoadTexture(renderer, fullPath.c_str());
    if (!texture)
    {
        std::cerr << "Failed to load texture: " << fullPath << " - " << IMG_GetError() << std::endl;
        return nullptr;
    }

    textures[path] = texture;
    return texture;
}

SDL_Texture *ResourceManager::getTexture(const std::string &path)
{
    auto it = textures.find(path);
    return it != textures.end() ? it->second : nullptr;
}

void ResourceManager::unloadTexture(const std::string &path)
{
    auto it = textures.find(path);
    if (it != textures.end())
    {
        SDL_DestroyTexture(it->second);
        textures.erase(it);
    }
}

TTF_Font *ResourceManager::loadFont(const std::string &path, int fontSize)
{
    std::string key = getFontKey(path, fontSize);

    // Check if font is already loaded
    auto it = fonts.find(key);
    if (it != fonts.end())
    {
        return it->second;
    }

    // Load new font
    std::string fullPath = std::string(ASSET_PATH) + path;
    TTF_Font *font = TTF_OpenFont(fullPath.c_str(), fontSize);
    if (!font)
    {
        std::cerr << "Failed to load font: " << fullPath << " - " << TTF_GetError() << std::endl;
        return nullptr;
    }

    fonts[key] = font;
    return font;
}

TTF_Font *ResourceManager::getFont(const std::string &path, int fontSize)
{
    std::string key = getFontKey(path, fontSize);
    auto it = fonts.find(key);
    return it != fonts.end() ? it->second : nullptr;
}

void ResourceManager::unloadFont(const std::string &path, int fontSize)
{
    std::string key = getFontKey(path, fontSize);
    auto it = fonts.find(key);
    if (it != fonts.end())
    {
        TTF_CloseFont(it->second);
        fonts.erase(it);
    }
}

SDL_Texture *ResourceManager::createTextTexture(const std::string &text, TTF_Font *font, SDL_Color color)
{
    if (!font)
        return nullptr;

    SDL_Surface *surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface)
    {
        std::cerr << "Failed to create text surface: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture)
    {
        std::cerr << "Failed to create text texture: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    return texture;
}

void ResourceManager::cleanup()
{
    // Clean up textures
    for (auto &[path, texture] : textures)
    {
        SDL_DestroyTexture(texture);
    }
    textures.clear();

    // Clean up fonts
    for (auto &[key, font] : fonts)
    {
        TTF_CloseFont(font);
    }
    fonts.clear();
}

std::string ResourceManager::getFontKey(const std::string &path, int fontSize)
{
    return path + "_" + std::to_string(fontSize);
}
