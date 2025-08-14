#include "RenderSystem.h"
#include "../components/Components.h"
#include "../managers/ResourceManager.h"
#include <sstream>
#include <cmath>

RenderSystem::RenderSystem(SDL_Renderer *renderer, ResourceManager *rm)
    : renderer(renderer), resourceManager(rm) {}

void RenderSystem::update(ECS &ecs, GameManager &gameManager, float fps)
{
    // Clear screen with sky blue background (135, 206, 235)
    SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
    SDL_RenderClear(renderer);

    // Render game sprites
    renderSprites(ecs);

    // Render projectiles
    renderProjectiles(ecs);

    // Render aiming line
    renderAimingLines(ecs);

    // Render crosshair
    renderCrosshair(ecs);

    // Render UI
    renderUI(ecs, gameManager, fps);

    // Present frame
    SDL_RenderPresent(renderer);
}

void RenderSystem::renderSprites(ECS &ecs)
{
    auto &transforms = ecs.getComponents<Transform>();

    for (auto &[entityID, transform] : transforms)
    {
        auto *sprite = ecs.getComponent<Sprite>(entityID);
        auto *animation = ecs.getComponent<Animation>(entityID);
        auto *entityType = ecs.getComponent<EntityType>(entityID);

        if (sprite)
        {
            std::string texturePath;
            bool needsTextureReload = false;

            // Determine texture path based on entity type
            if (entityType && entityType->type == "player")
            {
                // For player entities, check movement direction and animation frame
                auto *movementDir = ecs.getComponent<MovementDirection>(entityID);
                auto *velocity = ecs.getComponent<Velocity>(entityID);
                int frameNumber = 1; // Default to frame 1

                // Get current animation frame
                if (animation && sprite->animated && sprite->frameCount > 1)
                {
                    frameNumber = (animation->currentFrame % sprite->frameCount) + 1;
                }

                if (movementDir && movementDir->direction == MovementDirection::VERTICAL)
                {
                    texturePath = "art/playerGrey_up" + std::to_string(frameNumber) + ".png";
                }
                else
                {
                    texturePath = "art/playerGrey_walk" + std::to_string(frameNumber) + ".png";
                }

                // Always reload texture for player to handle animation and direction changes
                needsTextureReload = true;
            }
            else if (entityType && (entityType->type == "flying" || entityType->type == "swimming" || entityType->type == "walking"))
            {
                // For mob entities, handle individual frame files and direction
                int frameNumber = 1; // Default to frame 1

                // Get current animation frame
                if (animation && sprite->animated && sprite->frameCount > 1)
                {
                    frameNumber = (animation->currentFrame % sprite->frameCount) + 1;
                }

                if (entityType->type == "flying")
                {
                    texturePath = "art/enemyFlyingAlt_" + std::to_string(frameNumber) + ".png";
                }
                else if (entityType->type == "swimming")
                {
                    texturePath = "art/enemySwimming_" + std::to_string(frameNumber) + ".png";
                }
                else if (entityType->type == "walking")
                {
                    texturePath = "art/enemyWalking_" + std::to_string(frameNumber) + ".png";
                }

                // Always reload texture to handle animation changes
                needsTextureReload = true;
            }
            else if (entityType && entityType->type == "mobKing")
            {
                // For Mob King, check movement direction and animation frame
                auto *movementDir = ecs.getComponent<MovementDirection>(entityID);
                int frameNumber = 1; // Default to frame 1

                // Get current animation frame
                if (animation && sprite->animated && sprite->frameCount > 1)
                {
                    frameNumber = (animation->currentFrame % sprite->frameCount) + 1;
                }

                if (movementDir && movementDir->direction == MovementDirection::VERTICAL)
                {
                    texturePath = "art/enemyFlyingUp_" + std::to_string(frameNumber) + ".png";
                }
                else
                {
                    texturePath = "art/enemyFlyingAlt_" + std::to_string(frameNumber) + ".png";
                }

                // Always reload texture to handle animation and direction changes
                needsTextureReload = true;
            } // Load or reload texture if needed
            if (needsTextureReload && !texturePath.empty())
            {
                sprite->texture = resourceManager->loadTexture(texturePath);
                sprite->currentTexturePath = texturePath;
            }

            if (sprite->texture)
            {
                // Get the actual texture dimensions
                int textureWidth, textureHeight;
                SDL_QueryTexture(sprite->texture, nullptr, nullptr, &textureWidth, &textureHeight);

                SDL_Rect destRect = {
                    static_cast<int>(transform.x - sprite->width / 2),
                    static_cast<int>(transform.y - sprite->height / 2),
                    sprite->width,
                    sprite->height};

                // Source rectangle should use the full texture
                // (all sprites are individual frame files, not sprite sheets)
                SDL_Rect srcRect = {0, 0, textureWidth, textureHeight};

                // Determine sprite flipping based on entity type and movement direction
                SDL_RendererFlip flipFlags = SDL_FLIP_NONE;
                auto *velocity = ecs.getComponent<Velocity>(entityID);

                if (entityType && entityType->type == "player" && velocity)
                {
                    // Flip player sprite vertically when moving down
                    auto *movementDir = ecs.getComponent<MovementDirection>(entityID);
                    if (movementDir && movementDir->direction == MovementDirection::VERTICAL && velocity->y > 0)
                    {
                        flipFlags = SDL_FLIP_VERTICAL;
                    }
                }
                else if (entityType && (entityType->type == "flying" || entityType->type == "swimming" || entityType->type == "walking" || entityType->type == "mobKing") && velocity)
                {
                    // Flip mob sprites and mob king based on movement direction
                    auto *movementDir = ecs.getComponent<MovementDirection>(entityID);
                    if (movementDir)
                    {
                        if (movementDir->direction == MovementDirection::HORIZONTAL && velocity->x < 0)
                        {
                            flipFlags = SDL_FLIP_HORIZONTAL; // Flip when moving left (since sprites face right by default)
                        }
                        else if (movementDir->direction == MovementDirection::VERTICAL && entityType->type == "mobKing")
                        {
                            if (velocity->y > 0)
                            {
                                flipFlags = SDL_FLIP_VERTICAL; // Flip when moving down (enemyFlyingUp sprites face up by default)
                            }
                            // When moving up (velocity->y < 0), use normal orientation (no flip)
                        }
                        else if (movementDir->direction == MovementDirection::VERTICAL && entityType->type != "mobKing")
                        {
                            // For regular mobs, keep the original logic
                            if (velocity->y < 0)
                            {
                                flipFlags = SDL_FLIP_VERTICAL; // Flip when moving up
                            }
                            // When moving down (velocity->y > 0), use normal orientation (no flip)
                        }
                    }
                }

                SDL_RenderCopyEx(renderer, sprite->texture, &srcRect, &destRect, 0.0, nullptr, flipFlags);
            }
        }
    }
}

void RenderSystem::renderUI(ECS &ecs, GameManager &gameManager, float fps)
{
    auto &uiPositions = ecs.getComponents<UIPosition>();

    for (auto &[entityID, uiPos] : uiPositions)
    {
        auto *uiText = ecs.getComponent<UIText>(entityID);
        if (!uiText || !uiText->visible)
            continue;

        // Load font if not already loaded
        TTF_Font *font = resourceManager->getFont(uiText->fontPath, uiText->fontSize);
        if (!font)
        {
            font = resourceManager->loadFont(uiText->fontPath, uiText->fontSize);
        }

        if (font)
        {
            // Check if this is the gameMessage and needs text wrapping
            auto *entityType = ecs.getComponent<EntityType>(entityID);
            if (entityType && entityType->type == "gameMessage")
            {
                // Use text wrapping for game message (max width: 400 pixels)
                std::vector<std::string> lines = wrapText(uiText->content, font, 400);

                // Calculate total height for centering
                int lineHeight;
                TTF_SizeText(font, "A", nullptr, &lineHeight);
                int totalHeight = lines.size() * lineHeight;

                // Start position (centered vertically)
                float startY = uiPos.y - totalHeight / 2.0f;

                for (size_t i = 0; i < lines.size(); ++i)
                {
                    SDL_Texture *lineTexture = resourceManager->createTextTexture(
                        lines[i], font, uiText->color);

                    if (lineTexture)
                    {
                        int lineWidth, lineHeightActual;
                        SDL_QueryTexture(lineTexture, nullptr, nullptr, &lineWidth, &lineHeightActual);

                        SDL_Rect destRect = {
                            static_cast<int>(uiPos.x - lineWidth / 2),
                            static_cast<int>(startY + i * lineHeight),
                            lineWidth,
                            lineHeightActual};

                        SDL_RenderCopy(renderer, lineTexture, nullptr, &destRect);
                        SDL_DestroyTexture(lineTexture);
                    }
                }
            }
            else
            {
                // Single line rendering for other UI elements
                SDL_Texture *textTexture = resourceManager->createTextTexture(
                    uiText->content, font, uiText->color);

                if (textTexture)
                {
                    int textWidth, textHeight;
                    SDL_QueryTexture(textTexture, nullptr, nullptr, &textWidth, &textHeight);

                    SDL_Rect destRect = {
                        static_cast<int>(uiPos.x),
                        static_cast<int>(uiPos.y),
                        textWidth,
                        textHeight};

                    SDL_RenderCopy(renderer, textTexture, nullptr, &destRect);
                    SDL_DestroyTexture(textTexture);
                }
            }
        }
    }
}

std::vector<std::string> RenderSystem::wrapText(const std::string &text, TTF_Font *font, int maxWidth)
{
    std::vector<std::string> lines;
    std::istringstream words(text);
    std::string word;
    std::string currentLine;

    while (words >> word)
    {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;

        int textWidth;
        TTF_SizeText(font, testLine.c_str(), &textWidth, nullptr);

        if (textWidth <= maxWidth)
        {
            currentLine = testLine;
        }
        else
        {
            if (!currentLine.empty())
            {
                lines.push_back(currentLine);
                currentLine = word;
            }
            else
            {
                // Single word is too long, add it anyway
                lines.push_back(word);
            }
        }
    }

    if (!currentLine.empty())
    {
        lines.push_back(currentLine);
    }

    return lines;
}

// ========== BLOODSTRIKE 2D RENDERING METHODS ==========

void RenderSystem::renderAimingLines(ECS &ecs)
{
    auto &aimingLines = ecs.getComponents<AimingLine>();

    for (auto &[entityID, aimingLine] : aimingLines)
    {
        if (!aimingLine.showLine)
            continue;

        // Calculate direction and length
        float dirX = aimingLine.endX - aimingLine.startX;
        float dirY = aimingLine.endY - aimingLine.startY;
        float length = std::sqrt(dirX * dirX + dirY * dirY);

        if (length <= 0)
            continue;

        // Normalize direction
        dirX /= length;
        dirY /= length;

        // Set color based on range (white if in range, red if out of range)
        if (length <= aimingLine.maxRange)
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200); // White
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 200); // Red
        }

        // Draw dotted line
        float currentDistance = 0.0f;
        while (currentDistance < length && currentDistance < aimingLine.maxRange)
        {
            float x = aimingLine.startX + dirX * currentDistance;
            float y = aimingLine.startY + dirY * currentDistance;

            // Draw a small circle (dot)
            SDL_Rect dotRect = {
                static_cast<int>(x - 2),
                static_cast<int>(y - 2),
                4, 4};
            SDL_RenderFillRect(renderer, &dotRect);

            currentDistance += aimingLine.dotSpacing;
        }
    }
}

void RenderSystem::renderProjectiles(ECS &ecs)
{
    auto &projectileTags = ecs.getComponents<ProjectileTag>();

    for (auto &[entityID, projectileTag] : projectileTags)
    {
        Transform *transform = ecs.getComponent<Transform>(entityID);
        Sprite *sprite = ecs.getComponent<Sprite>(entityID);
        ProjectileColor *projColor = ecs.getComponent<ProjectileColor>(entityID);

        if (transform && sprite)
        {
            // Set projectile color (use ProjectileColor component if available, default to yellow)
            if (projColor)
            {
                SDL_SetRenderDrawColor(renderer, projColor->color.r, projColor->color.g,
                                       projColor->color.b, projColor->color.a);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Default yellow
            }

            // Draw a small rectangle for the projectile
            SDL_Rect rect = {
                static_cast<int>(transform->x - sprite->width / 2),
                static_cast<int>(transform->y - sprite->height / 2),
                sprite->width,
                sprite->height};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void RenderSystem::renderCrosshair(ECS &ecs)
{
    auto &mouseTargets = ecs.getComponents<MouseTarget>();

    for (auto &[entityID, mouseTarget] : mouseTargets)
    {
        if (!mouseTarget.isValid)
            continue;

        // Set crosshair color (white)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        int x = static_cast<int>(mouseTarget.x);
        int y = static_cast<int>(mouseTarget.y);
        int size = 8;

        // Draw crosshair (horizontal and vertical lines)
        SDL_RenderDrawLine(renderer, x - size, y, x + size, y);
        SDL_RenderDrawLine(renderer, x, y - size, x, y + size);
    }
}
