#include "AudioSystem.h"
#include <iostream>

AudioSystem::AudioSystem() {}

AudioSystem::~AudioSystem()
{
    cleanup();
}

bool AudioSystem::initialize()
{
    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    // Set initial volumes
    Mix_VolumeMusic(musicVolume);
    Mix_Volume(-1, sfxVolume); // -1 sets volume for all channels

    return true;
}

bool AudioSystem::loadSoundEffect(const std::string &name, const std::string &filePath)
{
    // Construct full path to asset
    std::string fullPath = std::string(ASSET_PATH) + filePath;

    Mix_Chunk *sound = Mix_LoadWAV(fullPath.c_str());
    if (!sound)
    {
        std::cerr << "Failed to load sound effect '" << name << "' from '" << fullPath << "': " << Mix_GetError() << std::endl;
        return false;
    }

    // Free existing sound if it exists
    auto it = soundEffects.find(name);
    if (it != soundEffects.end())
    {
        Mix_FreeChunk(it->second);
    }

    soundEffects[name] = sound;
    return true;
}

bool AudioSystem::loadMusic(const std::string &name, const std::string &filePath)
{
    // Construct full path to asset
    std::string fullPath = std::string(ASSET_PATH) + filePath;

    Mix_Music *music = Mix_LoadMUS(fullPath.c_str());
    if (!music)
    {
        std::cerr << "Failed to load music '" << name << "' from '" << fullPath << "': " << Mix_GetError() << std::endl;
        return false;
    }

    // Free existing music if it exists
    auto it = backgroundMusic.find(name);
    if (it != backgroundMusic.end())
    {
        Mix_FreeMusic(it->second);
    }

    backgroundMusic[name] = music;
    return true;
}

void AudioSystem::playSound(const std::string &name)
{
    auto it = soundEffects.find(name);
    if (it != soundEffects.end())
    {
        Mix_PlayChannel(-1, it->second, 0); // -1 = first available channel, 0 = play once
    }
    else
    {
        std::cerr << "Sound effect '" << name << "' not found!" << std::endl;
    }
}

void AudioSystem::playMusic(const std::string &name, bool loop)
{
    auto it = backgroundMusic.find(name);
    if (it != backgroundMusic.end())
    {
        // Stop current music if playing
        if (musicPlaying)
        {
            Mix_HaltMusic();
        }

        // Play new music
        int loops = loop ? -1 : 0; // -1 = infinite loop, 0 = play once
        if (Mix_PlayMusic(it->second, loops) == 0)
        {
            currentMusic = it->second;
            musicPlaying = true;
        }
        else
        {
            std::cerr << "Failed to play music '" << name << "': " << Mix_GetError() << std::endl;
        }
    }
    else
    {
        std::cerr << "Music '" << name << "' not found!" << std::endl;
    }
}

void AudioSystem::stopMusic()
{
    if (musicPlaying)
    {
        Mix_HaltMusic();
        musicPlaying = false;
        currentMusic = nullptr;
    }
}

void AudioSystem::pauseMusic()
{
    if (musicPlaying)
    {
        Mix_PauseMusic();
    }
}

void AudioSystem::resumeMusic()
{
    if (musicPlaying && Mix_PausedMusic())
    {
        Mix_ResumeMusic();
    }
}

void AudioSystem::setMusicVolume(int volume)
{
    musicVolume = std::max(0, std::min(128, volume));
    Mix_VolumeMusic(musicVolume);
}

void AudioSystem::setSFXVolume(int volume)
{
    sfxVolume = std::max(0, std::min(128, volume));
    Mix_Volume(-1, sfxVolume);
}

void AudioSystem::update(ECS &ecs, GameManager &gameManager, float deltaTime)
{
    static GameManager::GameState lastState = GameManager::MENU;

    // Handle music changes based on game state
    if (gameManager.currentState != lastState)
    {
        handleGameStateMusic(gameManager.currentState);
        lastState = gameManager.currentState;
    }
}

void AudioSystem::handleGameStateMusic(GameManager::GameState state)
{
    switch (state)
    {
    case GameManager::MENU:
        // Play menu music or stop music
        stopMusic();
        break;

    case GameManager::COUNTDOWN:
        // Play background music during countdown
        playMusic("background", true);
        break;

    case GameManager::PLAYING:
        // Play background music during gameplay
        playMusic("background", true);
        break;

    case GameManager::GAME_OVER:
        // Stop background music and play game over sound
        stopMusic();
        playSound("gameover");
        break;
    }
}

void AudioSystem::cleanup()
{
    // Stop all audio
    Mix_HaltMusic();
    Mix_HaltChannel(-1);

    // Free sound effects
    for (auto &[name, sound] : soundEffects)
    {
        Mix_FreeChunk(sound);
    }
    soundEffects.clear();

    // Free music
    for (auto &[name, music] : backgroundMusic)
    {
        Mix_FreeMusic(music);
    }
    backgroundMusic.clear();

    // Close SDL_mixer
    Mix_CloseAudio();
}
