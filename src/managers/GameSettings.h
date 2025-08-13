#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class GameSettings
{
public:
    // Singleton pattern for global access
    static GameSettings &getInstance()
    {
        static GameSettings instance;
        return instance;
    }

    // Load settings from JSON file
    bool loadSettings(const std::string &filePath = "gameSettings.json");

    // Save current settings to JSON file
    bool saveSettings(const std::string &filePath = "gameSettings.json");

    // Getter methods for different setting categories

    // Single Player Settings
    float getSinglePlayerLevelDuration() const { return singlePlayerLevelDuration; }
    int getSinglePlayerMaxLevel() const { return singlePlayerMaxLevel; }
    float getSinglePlayerMobSpawnInterval() const { return singlePlayerMobSpawnInterval; }
    float getSinglePlayerScorePerSecond() const { return singlePlayerScorePerSecond; }

    // Level-specific settings for single player
    float getLevelMobSpeedMultiplier(int level) const;
    float getLevelSpawnInterval(int level) const;
    bool canMobsShootAtLevel(int level) const;

    // Dual Player / Multiplayer Settings
    float getDualPlayerLevelDuration() const { return dualPlayerLevelDuration; }
    float getDualPlayerStartSpawnInterval() const { return dualPlayerStartSpawnInterval; }
    float getDualPlayerEndSpawnInterval() const { return dualPlayerEndSpawnInterval; }
    float getDualPlayerStartSpeedMultiplier() const { return dualPlayerStartSpeedMultiplier; }
    float getDualPlayerEndSpeedMultiplier() const { return dualPlayerEndSpeedMultiplier; }
    float getDualPlayerMobShootingLastSeconds() const { return dualPlayerMobShootingLastSeconds; }
    float getDualPlayerScorePerSecond() const { return dualPlayerScorePerSecond; }

    // Multiplayer Settings (separate from dual player)
    float getMultiplayerLevelDuration() const { return multiplayerLevelDuration; }
    float getMultiplayerStartSpawnInterval() const { return multiplayerStartSpawnInterval; }
    float getMultiplayerEndSpawnInterval() const { return multiplayerEndSpawnInterval; }
    float getMultiplayerStartSpeedMultiplier() const { return multiplayerStartSpeedMultiplier; }
    float getMultiplayerEndSpeedMultiplier() const { return multiplayerEndSpeedMultiplier; }
    float getMultiplayerMobShootingLastSeconds() const { return multiplayerMobShootingLastSeconds; }
    float getMultiplayerScorePerSecond() const { return multiplayerScorePerSecond; }

    float getSpawnBoundaryPadding() const { return spawnBoundaryPadding; }

    // Mob King Settings
    float getMobKingHealth() const { return mobKingHealth; }
    float getMobKingDamagePerBullet() const { return mobKingDamagePerBullet; }
    bool shouldSpawnMobKingImmediately() const { return spawnMobKingImmediately; }

    // Network Settings
    int getHeartbeatIntervalMs() const { return heartbeatIntervalMs; }
    int getConnectionTimeoutMs() const { return connectionTimeoutMs; }
    int getPingIntervalMs() const { return pingIntervalMs; }

    // Graphics Settings
    float getScreenWidth() const { return screenWidth; }
    float getScreenHeight() const { return screenHeight; }
    int getTargetFPS() const { return targetFPS; }

    struct Color
    {
        int r, g, b, a = 255;
    };

    Color getBackgroundColor() const { return backgroundColor; }
    Color getAimingLineNormalColor() const { return aimingLineNormalColor; }
    Color getAimingLineShootingColor() const { return aimingLineShootingColor; }

    // Audio Settings
    int getAudioFrequency() const { return audioFrequency; }
    int getAudioChannels() const { return audioChannels; }
    int getAudioChunkSize() const { return audioChunkSize; }
    int getMusicVolume() const { return musicVolume; }
    int getSFXVolume() const { return sfxVolume; }

    // UI Settings
    struct Position
    {
        float x, y;
    };

    Position getMobKingHealthUIPosition() const { return mobKingHealthUIPosition; }
    Color getMobKingHealthHighColor() const { return mobKingHealthHighColor; }
    Color getMobKingHealthMediumColor() const { return mobKingHealthMediumColor; }
    Color getMobKingHealthLowColor() const { return mobKingHealthLowColor; }
    float getMobKingHealthMediumThreshold() const { return mobKingHealthMediumThreshold; }
    float getMobKingHealthLowThreshold() const { return mobKingHealthLowThreshold; }
    std::string getMobKingHealthTextFormat() const { return mobKingHealthTextFormat; }

    float getMenuTitlePositionY() const { return menuTitlePositionY; }
    float getMenuOptionSpacing() const { return menuOptionSpacing; }

    // Debug Settings
    bool isLoggingEnabled() const { return enableLogging; }
    bool shouldShowFPS() const { return showFPS; }
    bool shouldShowDebugInfo() const { return showDebugInfo; }

    // Setter methods for runtime modification
    void setMusicVolume(int volume) { musicVolume = volume; }
    void setSFXVolume(int volume) { sfxVolume = volume; }
    void setScreenSize(float width, float height)
    {
        screenWidth = width;
        screenHeight = height;
    }

private:
    GameSettings() = default;
    ~GameSettings() = default;
    GameSettings(const GameSettings &) = delete;
    GameSettings &operator=(const GameSettings &) = delete;

    // Helper methods
    Color parseColor(const json &colorJson) const;
    Position parsePosition(const json &positionJson) const;

    // Single Player Settings
    float singlePlayerLevelDuration = 5.0f;
    int singlePlayerMaxLevel = 4;
    float singlePlayerMobSpawnInterval = 0.5f;
    float singlePlayerScorePerSecond = 10.0f;

    // Level-specific settings (stored as arrays indexed by level-1)
    float levelMobSpeedMultipliers[4] = {0.8f, 1.0f, 1.5f, 1.3f};
    float levelSpawnIntervals[4] = {0.8f, 0.6f, 0.4f, 0.3f};
    bool levelCanMobsShoot[4] = {false, false, false, true};

    // Dual Player Settings
    float dualPlayerLevelDuration = 20.0f;
    float dualPlayerStartSpawnInterval = 1.0f;
    float dualPlayerEndSpawnInterval = 0.2f;
    float dualPlayerStartSpeedMultiplier = 1.0f;
    float dualPlayerEndSpeedMultiplier = 2.0f;
    float dualPlayerMobShootingLastSeconds = 15.0f;
    float dualPlayerScorePerSecond = 15.0f;

    // Multiplayer Settings (separate from dual player)
    float multiplayerLevelDuration = 90.0f;
    float multiplayerStartSpawnInterval = 1.0f;
    float multiplayerEndSpawnInterval = 0.2f;
    float multiplayerStartSpeedMultiplier = 1.0f;
    float multiplayerEndSpeedMultiplier = 2.0f;
    float multiplayerMobShootingLastSeconds = 15.0f;
    float multiplayerScorePerSecond = 15.0f;

    float spawnBoundaryPadding = 50.0f;

    // Mob King Settings
    float mobKingHealth = 1000.0f;
    float mobKingDamagePerBullet = 20.0f;
    bool spawnMobKingImmediately = true;

    // Network Settings
    int heartbeatIntervalMs = 2000;
    int connectionTimeoutMs = 10000;
    int pingIntervalMs = 1000;

    // Graphics Settings
    float screenWidth = 1280.0f;
    float screenHeight = 720.0f;
    int targetFPS = 60;
    Color backgroundColor = {135, 206, 235, 255};
    Color aimingLineNormalColor = {255, 255, 255, 200};
    Color aimingLineShootingColor = {255, 0, 0, 200};

    // Audio Settings
    int audioFrequency = 44100;
    int audioChannels = 2;
    int audioChunkSize = 2048;
    int musicVolume = 64;
    int sfxVolume = 128;

    // UI Settings
    Position mobKingHealthUIPosition = {280.0f, 10.0f};
    Color mobKingHealthHighColor = {0, 255, 0};
    Color mobKingHealthMediumColor = {255, 255, 0};
    Color mobKingHealthLowColor = {255, 0, 0};
    float mobKingHealthMediumThreshold = 50.0f;
    float mobKingHealthLowThreshold = 25.0f;
    std::string mobKingHealthTextFormat = "Mob King Health: {health}/{maxHealth}";

    float menuTitlePositionY = 150.0f;
    float menuOptionSpacing = 60.0f;

    // Debug Settings
    bool enableLogging = true;
    bool showFPS = true;
    bool showDebugInfo = false;
};
