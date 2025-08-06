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

// ========== BLOODSTRIKE 2D COMBAT COMPONENTS ==========

struct MouseTarget
{
    float x, y;   // Mouse position in world coordinates
    bool isValid; // Whether mouse is in valid range

    MouseTarget(float x = 0.0f, float y = 0.0f, bool valid = false)
        : x(x), y(y), isValid(valid) {}
};

struct AimingLine
{
    float startX, startY; // Player position
    float endX, endY;     // Target position
    float maxRange;       // Maximum aiming distance
    bool showLine;        // Whether to render the line
    int dotCount;         // Number of dots in the line
    float dotSpacing;     // Distance between dots

    AimingLine(float maxRange = 300.0f, int dots = 20, float spacing = 15.0f)
        : startX(0), startY(0), endX(0), endY(0), maxRange(maxRange),
          showLine(false), dotCount(dots), dotSpacing(spacing) {}
};

struct Weapon
{
    float damage;
    float fireRate; // shots per second
    int ammoCount;
    int maxAmmo;
    float range;     // Maximum shooting range
    float fireTimer; // Time since last shot
    bool canFire;

    Weapon(float dmg = 25.0f, float rate = 5.0f, int ammo = 30, int maxAmmo = 30, float range = 300.0f)
        : damage(dmg), fireRate(rate), ammoCount(ammo), maxAmmo(maxAmmo),
          range(range), fireTimer(0.0f), canFire(true) {}
};

struct Projectile
{
    float speed;
    float damage;
    float lifetime;
    float timer;
    EntityID owner;
    float directionX, directionY; // Normalized direction vector

    Projectile(float spd = 500.0f, float dmg = 25.0f, float life = 3.0f,
               EntityID ownerId = 0, float dirX = 0.0f, float dirY = 0.0f)
        : speed(spd), damage(dmg), lifetime(life), timer(0.0f),
          owner(ownerId), directionX(dirX), directionY(dirY) {}
};

// Additional tag components
struct ProjectileTag
{
};
struct WeaponTag
{
};
