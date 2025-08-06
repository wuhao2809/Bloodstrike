#pragma once
#include <SDL2/SDL.h>
#include <string>

// Pure ECS Components (Data Only)

struct Transform
{
    float x, y;
    float rotation;

    Transform(float x = 0, float y = 0, float rotation = 0)
        : x(x), y(y), rotation(rotation) {}
};

struct Velocity
{
    float x, y;

    Velocity(float x = 0, float y = 0) : x(x), y(y) {}
};

struct Sprite
{
    SDL_Texture *texture;
    int width, height;
    int frameCount;
    float frameTime;
    bool animated;
    std::string currentTexturePath; // Track currently loaded texture

    Sprite(SDL_Texture *tex = nullptr, int w = 0, int h = 0, int frames = 1, float fTime = 0.1f)
        : texture(tex), width(w), height(h), frameCount(frames), frameTime(fTime), animated(frames > 1), currentTexturePath("") {}
};

struct Collider
{
    float width, height;
    bool isTrigger;

    Collider(float w = 0, float h = 0, bool trigger = false)
        : width(w), height(h), isTrigger(trigger) {}
};

struct Speed
{
    float value;

    Speed(float v = 0.0f) : value(v) {}
};

struct Animation
{
    int currentFrame;
    float animationTimer;

    Animation(int frame = 0, float timer = 0.0f)
        : currentFrame(frame), animationTimer(timer) {}
};

struct EntityType
{
    std::string type;

    EntityType(const std::string &t = "") : type(t) {}
};

struct UIText
{
    std::string content;
    std::string fontPath;
    int fontSize;
    SDL_Color color;
    bool visible;

    UIText(const std::string &text = "", const std::string &font = "", int size = 24,
           SDL_Color col = {255, 255, 255, 255}, bool vis = true)
        : content(text), fontPath(font), fontSize(size), color(col), visible(vis) {}
};

struct UIPosition
{
    float x, y;

    UIPosition(float x = 0, float y = 0) : x(x), y(y) {}
};

// Tag Components (Empty structs for identification)
struct PlayerTag
{
};
struct MobTag
{
};

// Movement Direction Component for directional sprites
struct MovementDirection
{
    enum Direction
    {
        HORIZONTAL,
        VERTICAL
    } direction;

    MovementDirection(Direction dir = HORIZONTAL) : direction(dir) {}
};
