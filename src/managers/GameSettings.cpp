#include "GameSettings.h"

bool GameSettings::loadSettings(const std::string &filePath)
{
    try
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            std::cerr << "Could not open settings file: " << filePath << std::endl;
            std::cerr << "Using default settings..." << std::endl;
            return false;
        }

        json settings;
        file >> settings;
        file.close();

        // Load Single Player Settings
        if (settings.contains("gameplay") && settings["gameplay"].contains("singlePlayer"))
        {
            auto sp = settings["gameplay"]["singlePlayer"];

            if (sp.contains("levelDuration"))
            {
                singlePlayerLevelDuration = sp["levelDuration"].get<float>();
            }
            if (sp.contains("maxLevel"))
            {
                singlePlayerMaxLevel = sp["maxLevel"].get<int>();
            }
            if (sp.contains("mobSpawnInterval"))
            {
                singlePlayerMobSpawnInterval = sp["mobSpawnInterval"].get<float>();
            }
            if (sp.contains("scorePerSecond"))
            {
                singlePlayerScorePerSecond = sp["scorePerSecond"].get<float>();
            }

            // Load level-specific settings
            if (sp.contains("levelSettings"))
            {
                auto levelSettings = sp["levelSettings"];
                for (int i = 1; i <= 4; i++)
                {
                    std::string levelKey = std::to_string(i);
                    if (levelSettings.contains(levelKey))
                    {
                        auto level = levelSettings[levelKey];
                        if (level.contains("mobSpeedMultiplier"))
                        {
                            levelMobSpeedMultipliers[i - 1] = level["mobSpeedMultiplier"].get<float>();
                        }
                        if (level.contains("spawnInterval"))
                        {
                            levelSpawnIntervals[i - 1] = level["spawnInterval"].get<float>();
                        }
                        if (level.contains("canMobsShoot"))
                        {
                            levelCanMobsShoot[i - 1] = level["canMobsShoot"].get<bool>();
                        }
                    }
                }
            }
        }

        // Load Dual Player Settings
        if (settings.contains("gameplay") && settings["gameplay"].contains("dualPlayer"))
        {
            auto dp = settings["gameplay"]["dualPlayer"];

            if (dp.contains("levelDuration"))
            {
                dualPlayerLevelDuration = dp["levelDuration"].get<float>();
            }
            if (dp.contains("mobSpawnInterval"))
            {
                auto spawnInterval = dp["mobSpawnInterval"];
                if (spawnInterval.contains("start"))
                {
                    dualPlayerStartSpawnInterval = spawnInterval["start"].get<float>();
                }
                if (spawnInterval.contains("end"))
                {
                    dualPlayerEndSpawnInterval = spawnInterval["end"].get<float>();
                }
            }
            if (dp.contains("mobSpeedMultiplier"))
            {
                auto speedMultiplier = dp["mobSpeedMultiplier"];
                if (speedMultiplier.contains("start"))
                {
                    dualPlayerStartSpeedMultiplier = speedMultiplier["start"].get<float>();
                }
                if (speedMultiplier.contains("end"))
                {
                    dualPlayerEndSpeedMultiplier = speedMultiplier["end"].get<float>();
                }
            }
            if (dp.contains("mobShootingSettings"))
            {
                auto shootingSettings = dp["mobShootingSettings"];
                if (shootingSettings.contains("enabledInLastSeconds"))
                {
                    dualPlayerMobShootingLastSeconds = shootingSettings["enabledInLastSeconds"].get<float>();
                }
            }
            if (dp.contains("scorePerSecond"))
            {
                dualPlayerScorePerSecond = dp["scorePerSecond"].get<float>();
            }
            if (dp.contains("spawnBoundaryPadding"))
            {
                spawnBoundaryPadding = dp["spawnBoundaryPadding"].get<float>();
            }
        }

        // Load Multiplayer Settings (separate from dual player)
        if (settings.contains("gameplay") && settings["gameplay"].contains("multiplayer"))
        {
            auto mp = settings["gameplay"]["multiplayer"];

            if (mp.contains("levelDuration"))
            {
                multiplayerLevelDuration = mp["levelDuration"].get<float>();
            }
            if (mp.contains("mobSpawnInterval"))
            {
                auto spawnInterval = mp["mobSpawnInterval"];
                if (spawnInterval.contains("start"))
                {
                    multiplayerStartSpawnInterval = spawnInterval["start"].get<float>();
                }
                if (spawnInterval.contains("end"))
                {
                    multiplayerEndSpawnInterval = spawnInterval["end"].get<float>();
                }
            }
            if (mp.contains("mobSpeedMultiplier"))
            {
                auto speedMultiplier = mp["mobSpeedMultiplier"];
                if (speedMultiplier.contains("start"))
                {
                    multiplayerStartSpeedMultiplier = speedMultiplier["start"].get<float>();
                }
                if (speedMultiplier.contains("end"))
                {
                    multiplayerEndSpeedMultiplier = speedMultiplier["end"].get<float>();
                }
            }
            if (mp.contains("mobShootingSettings"))
            {
                auto shootingSettings = mp["mobShootingSettings"];
                if (shootingSettings.contains("enabledInLastSeconds"))
                {
                    multiplayerMobShootingLastSeconds = shootingSettings["enabledInLastSeconds"].get<float>();
                }
            }
            if (mp.contains("scorePerSecond"))
            {
                multiplayerScorePerSecond = mp["scorePerSecond"].get<float>();
            }
        }

        // Load Multiplayer Network Settings
        if (settings.contains("gameplay") && settings["gameplay"].contains("multiplayer") &&
            settings["gameplay"]["multiplayer"].contains("networkSettings"))
        {
            auto networkSettings = settings["gameplay"]["multiplayer"]["networkSettings"];

            if (networkSettings.contains("heartbeatIntervalMs"))
            {
                heartbeatIntervalMs = networkSettings["heartbeatIntervalMs"].get<int>();
            }
            if (networkSettings.contains("connectionTimeoutMs"))
            {
                connectionTimeoutMs = networkSettings["connectionTimeoutMs"].get<int>();
            }
            if (networkSettings.contains("pingIntervalMs"))
            {
                pingIntervalMs = networkSettings["pingIntervalMs"].get<int>();
            }
        }

        // Load Mob King Settings
        if (settings.contains("gameplay") && settings["gameplay"].contains("mobKing"))
        {
            auto mobKing = settings["gameplay"]["mobKing"];

            if (mobKing.contains("health"))
            {
                mobKingHealth = mobKing["health"].get<float>();
            }
            if (mobKing.contains("damagePerBullet"))
            {
                mobKingDamagePerBullet = mobKing["damagePerBullet"].get<float>();
            }
            if (mobKing.contains("spawnImmediately"))
            {
                spawnMobKingImmediately = mobKing["spawnImmediately"].get<bool>();
            }
        }

        // Load Graphics Settings
        if (settings.contains("graphics"))
        {
            auto graphics = settings["graphics"];

            if (graphics.contains("screenSize"))
            {
                auto screenSize = graphics["screenSize"];
                if (screenSize.contains("width") && screenSize.contains("height"))
                {
                    screenWidth = screenSize["width"].get<float>();
                    screenHeight = screenSize["height"].get<float>();
                }
            }
            if (graphics.contains("targetFPS"))
            {
                targetFPS = graphics["targetFPS"].get<int>();
            }
            if (graphics.contains("backgroundColor"))
            {
                backgroundColor = parseColor(graphics["backgroundColor"]);
            }
            if (graphics.contains("aimingLine"))
            {
                auto aimingLine = graphics["aimingLine"];
                if (aimingLine.contains("normalColor"))
                {
                    aimingLineNormalColor = parseColor(aimingLine["normalColor"]);
                }
                if (aimingLine.contains("shootingColor"))
                {
                    aimingLineShootingColor = parseColor(aimingLine["shootingColor"]);
                }
            }
        }

        // Load Audio Settings
        if (settings.contains("audio") && settings["audio"].contains("settings"))
        {
            auto audioSettings = settings["audio"]["settings"];

            if (audioSettings.contains("frequency"))
            {
                audioFrequency = audioSettings["frequency"].get<int>();
            }
            if (audioSettings.contains("channels"))
            {
                audioChannels = audioSettings["channels"].get<int>();
            }
            if (audioSettings.contains("chunkSize"))
            {
                audioChunkSize = audioSettings["chunkSize"].get<int>();
            }
            if (audioSettings.contains("musicVolume"))
            {
                musicVolume = audioSettings["musicVolume"].get<int>();
            }
            if (audioSettings.contains("sfxVolume"))
            {
                sfxVolume = audioSettings["sfxVolume"].get<int>();
            }
        }

        // Load UI Settings
        if (settings.contains("ui"))
        {
            auto ui = settings["ui"];

            if (ui.contains("mobKingHealth"))
            {
                auto mobKingHealthUI = ui["mobKingHealth"];

                if (mobKingHealthUI.contains("position"))
                {
                    mobKingHealthUIPosition = parsePosition(mobKingHealthUI["position"]);
                }
                if (mobKingHealthUI.contains("colors"))
                {
                    auto colors = mobKingHealthUI["colors"];
                    if (colors.contains("high"))
                    {
                        mobKingHealthHighColor = parseColor(colors["high"]);
                    }
                    if (colors.contains("medium"))
                    {
                        mobKingHealthMediumColor = parseColor(colors["medium"]);
                    }
                    if (colors.contains("low"))
                    {
                        mobKingHealthLowColor = parseColor(colors["low"]);
                    }
                }
                if (mobKingHealthUI.contains("thresholds"))
                {
                    auto thresholds = mobKingHealthUI["thresholds"];
                    if (thresholds.contains("mediumHealthPercent"))
                    {
                        mobKingHealthMediumThreshold = thresholds["mediumHealthPercent"].get<float>();
                    }
                    if (thresholds.contains("lowHealthPercent"))
                    {
                        mobKingHealthLowThreshold = thresholds["lowHealthPercent"].get<float>();
                    }
                }
                if (mobKingHealthUI.contains("textFormat"))
                {
                    mobKingHealthTextFormat = mobKingHealthUI["textFormat"].get<std::string>();
                }
            }

            if (ui.contains("menu"))
            {
                auto menu = ui["menu"];
                if (menu.contains("titlePosition") && menu["titlePosition"].contains("y"))
                {
                    menuTitlePositionY = menu["titlePosition"]["y"].get<float>();
                }
                if (menu.contains("optionSpacing"))
                {
                    menuOptionSpacing = menu["optionSpacing"].get<float>();
                }
            }
        }

        // Load Debug Settings
        if (settings.contains("debug"))
        {
            auto debug = settings["debug"];

            if (debug.contains("enableLogging"))
            {
                enableLogging = debug["enableLogging"].get<bool>();
            }
            if (debug.contains("showFPS"))
            {
                showFPS = debug["showFPS"].get<bool>();
            }
            if (debug.contains("showDebugInfo"))
            {
                showDebugInfo = debug["showDebugInfo"].get<bool>();
            }
        }

        if (enableLogging)
        {
            std::cout << "GameSettings: Successfully loaded settings from " << filePath << std::endl;
        }
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading settings from " << filePath << ": " << e.what() << std::endl;
        std::cerr << "Using default settings..." << std::endl;
        return false;
    }
}

bool GameSettings::saveSettings(const std::string &filePath)
{
    try
    {
        json settings;

        // Save Single Player Settings
        settings["gameplay"]["singlePlayer"]["levelDuration"] = singlePlayerLevelDuration;
        settings["gameplay"]["singlePlayer"]["maxLevel"] = singlePlayerMaxLevel;
        settings["gameplay"]["singlePlayer"]["mobSpawnInterval"] = singlePlayerMobSpawnInterval;
        settings["gameplay"]["singlePlayer"]["scorePerSecond"] = singlePlayerScorePerSecond;

        // Save level-specific settings
        for (int i = 1; i <= 4; i++)
        {
            std::string levelKey = std::to_string(i);
            settings["gameplay"]["singlePlayer"]["levelSettings"][levelKey]["mobSpeedMultiplier"] = levelMobSpeedMultipliers[i - 1];
            settings["gameplay"]["singlePlayer"]["levelSettings"][levelKey]["spawnInterval"] = levelSpawnIntervals[i - 1];
            settings["gameplay"]["singlePlayer"]["levelSettings"][levelKey]["canMobsShoot"] = levelCanMobsShoot[i - 1];
        }

        // Save Dual Player Settings
        settings["gameplay"]["dualPlayer"]["levelDuration"] = dualPlayerLevelDuration;
        settings["gameplay"]["dualPlayer"]["mobSpawnInterval"]["start"] = dualPlayerStartSpawnInterval;
        settings["gameplay"]["dualPlayer"]["mobSpawnInterval"]["end"] = dualPlayerEndSpawnInterval;
        settings["gameplay"]["dualPlayer"]["mobSpeedMultiplier"]["start"] = dualPlayerStartSpeedMultiplier;
        settings["gameplay"]["dualPlayer"]["mobSpeedMultiplier"]["end"] = dualPlayerEndSpeedMultiplier;
        settings["gameplay"]["dualPlayer"]["mobShootingSettings"]["enabledInLastSeconds"] = dualPlayerMobShootingLastSeconds;
        settings["gameplay"]["dualPlayer"]["scorePerSecond"] = dualPlayerScorePerSecond;
        settings["gameplay"]["dualPlayer"]["spawnBoundaryPadding"] = spawnBoundaryPadding;

        // Save Graphics Settings
        settings["graphics"]["screenSize"]["width"] = screenWidth;
        settings["graphics"]["screenSize"]["height"] = screenHeight;
        settings["graphics"]["targetFPS"] = targetFPS;
        settings["graphics"]["backgroundColor"]["r"] = backgroundColor.r;
        settings["graphics"]["backgroundColor"]["g"] = backgroundColor.g;
        settings["graphics"]["backgroundColor"]["b"] = backgroundColor.b;
        settings["graphics"]["backgroundColor"]["a"] = backgroundColor.a;

        // Save Audio Settings
        settings["audio"]["settings"]["musicVolume"] = musicVolume;
        settings["audio"]["settings"]["sfxVolume"] = sfxVolume;

        std::ofstream file(filePath);
        if (!file.is_open())
        {
            std::cerr << "Could not open settings file for writing: " << filePath << std::endl;
            return false;
        }

        file << settings.dump(2); // Pretty print with 2 spaces
        file.close();

        if (enableLogging)
        {
            std::cout << "GameSettings: Successfully saved settings to " << filePath << std::endl;
        }
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error saving settings to " << filePath << ": " << e.what() << std::endl;
        return false;
    }
}

float GameSettings::getLevelMobSpeedMultiplier(int level) const
{
    if (level >= 1 && level <= 4)
    {
        return levelMobSpeedMultipliers[level - 1];
    }
    return 1.0f; // Default speed
}

float GameSettings::getLevelSpawnInterval(int level) const
{
    if (level >= 1 && level <= 4)
    {
        return levelSpawnIntervals[level - 1];
    }
    return 0.5f; // Default interval
}

bool GameSettings::canMobsShootAtLevel(int level) const
{
    if (level >= 1 && level <= 4)
    {
        return levelCanMobsShoot[level - 1];
    }
    return false; // Default: no shooting
}

GameSettings::Color GameSettings::parseColor(const json &colorJson) const
{
    Color color = {255, 255, 255, 255}; // Default white

    if (colorJson.contains("r"))
        color.r = colorJson["r"].get<int>();
    if (colorJson.contains("g"))
        color.g = colorJson["g"].get<int>();
    if (colorJson.contains("b"))
        color.b = colorJson["b"].get<int>();
    if (colorJson.contains("a"))
        color.a = colorJson["a"].get<int>();

    return color;
}

GameSettings::Position GameSettings::parsePosition(const json &positionJson) const
{
    Position position = {0.0f, 0.0f}; // Default origin

    if (positionJson.contains("x"))
        position.x = positionJson["x"].get<float>();
    if (positionJson.contains("y"))
        position.y = positionJson["y"].get<float>();

    return position;
}
