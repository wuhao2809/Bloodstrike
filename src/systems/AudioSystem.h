#pragma once
#include "System.h"
#include "../managers/GameManager.h"
#include <SDL2/SDL_mixer.h>
#include <unordered_map>
#include <string>

class AudioSystem : public System
{
private:
    std::unordered_map<std::string, Mix_Chunk *> soundEffects;
    std::unordered_map<std::string, Mix_Music *> backgroundMusic;

    Mix_Music *currentMusic = nullptr;
    bool musicPlaying = false;

    // Volume settings (0-128)
    int musicVolume = 64;
    int sfxVolume = 64;

public:
    AudioSystem();
    ~AudioSystem();

    // Initialize SDL_mixer
    bool initialize();

    // Load audio assets
    bool loadSoundEffect(const std::string &name, const std::string &filePath);
    bool loadMusic(const std::string &name, const std::string &filePath);

    // Play audio
    void playSound(const std::string &name);
    void playMusic(const std::string &name, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();

    // Volume control
    void setMusicVolume(int volume); // 0-128
    void setSFXVolume(int volume);   // 0-128

    // System update - handles game state music changes
    void update(ECS &ecs, GameManager &gameManager, float deltaTime) override;

    // Cleanup
    void cleanup();

private:
    void handleGameStateMusic(GameManager::GameState state);
};
