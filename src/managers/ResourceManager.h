#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <unordered_map>
#include <string>

class ResourceManager {
private:
    SDL_Renderer* renderer;
    std::unordered_map<std::string, SDL_Texture*> textures;
    std::unordered_map<std::string, TTF_Font*> fonts;

public:
    ResourceManager(SDL_Renderer* renderer);
    ~ResourceManager();
    
    // Texture management
    SDL_Texture* loadTexture(const std::string& path);
    SDL_Texture* getTexture(const std::string& path);
    void unloadTexture(const std::string& path);
    
    // Font management
    TTF_Font* loadFont(const std::string& path, int fontSize);
    TTF_Font* getFont(const std::string& path, int fontSize);
    void unloadFont(const std::string& path, int fontSize);
    
    // Create text texture from font
    SDL_Texture* createTextTexture(const std::string& text, TTF_Font* font, SDL_Color color);
    
    // Cleanup
    void cleanup();

private:
    std::string getFontKey(const std::string& path, int fontSize);
};
