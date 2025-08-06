# 🎯 Bloodstrike 2D - Immediate Development Plan

## 🎮 Project Overview

**Bloodstrike** is a 2D top-down shooter game built with C++ and SDL2, featuring both single-player (vs AI) and peer-to-peer multiplayer modes.

### 🏗️ Current Architecture

- **Engine**: C++ with ECS (Entity Component System)
- **Graphics**: SDL2, SDL2_image, SDL2_ttf
- **Audio**: SDL2_mixer
- **Networking**: SDL_net (TCP peer-to-peer) - Future phase
- **Build System**: CMake
- **Platform**: Cross-platform (primary: macOS)

---

## 📅 Immediate Development Plan

### 🟢 **Phase 1: Project Rebranding** (Today - Step 1)

**Goal**: Change project name from "Dodge the Creeps" to "Bloodstrike"

#### Tasks:

- [ ] Update `CMakeLists.txt` project name from "DodgeTheCreeps" to "Bloodstrike"
- [ ] Update `run.sh` script text and executable name
- [ ] Update any hardcoded strings in source files
- [ ] Update README.md if exists
- [ ] Test build system works with new name

#### Files to Modify:

- `CMakeLists.txt` - Change project name
- `run.sh` - Update script descriptions and executable path
- `src/core/Game.cpp` - Update window title
- Any other files with "Dodge the Creeps" references

---

### 🟡 **Phase 2: Larger Window Size** (Today - Step 2)

**Goal**: Increase game window dimensions for better gameplay

#### Current Status Check:

Need to find current window size settings in:

- Game initialization code
- Configuration files
- Constants or defines

#### Proposed Changes:

- **Current**: Likely 800x600 or 1024x768
- **New**: 1280x720 (720p) or 1920x1080 (1080p)
- Make window size configurable via constants

#### Tasks:

- [ ] Locate window creation code in `Game.cpp`
- [ ] Update window dimensions
- [ ] Ensure UI elements scale properly
- [ ] Update any hardcoded position calculations
- [ ] Test rendering performance

---

### 🟠 **Phase 3: Aiming & Shooting System** (Today - Step 3)

**Goal**: Implement mouse-based aiming with visual feedback

#### New Components Needed:

```cpp
struct MouseTarget {
    float x, y;           // Mouse position in world coordinates
    bool isValid;         // Whether mouse is in valid range
};

struct AimingLine {
    float startX, startY; // Player position
    float endX, endY;     // Target position
    float maxRange;       // Maximum aiming distance
    bool showLine;        // Whether to render the line
    int dotCount;         // Number of dots in the line
    float dotSpacing;     // Distance between dots
};

struct Weapon {
    float damage;
    float fireRate;       // shots per second
    int ammoCount;
    int maxAmmo;
    float range;          // Maximum shooting range
    float fireTimer;      // Time since last shot
    bool canFire;
};

struct Projectile {
    float speed;
    float damage;
    float lifetime;
    EntityID owner;
    float directionX, directionY; // Normalized direction vector
};
```

#### New Systems Needed:

```cpp
class AimingSystem : public System {
public:
    void update(ECS& ecs, GameManager& gameManager, float deltaTime) override;
private:
    void updateMouseTarget(ECS& ecs);
    void calculateAimingLine(ECS& ecs);
    void handleMouseInput(ECS& ecs);
};

class WeaponSystem : public System {
public:
    void update(ECS& ecs, GameManager& gameManager, float deltaTime) override;
private:
    void handleShooting(ECS& ecs, float deltaTime);
    void updateWeaponTimers(ECS& ecs, float deltaTime);
    EntityID createProjectile(ECS& ecs, float startX, float startY, float dirX, float dirY, const Weapon& weapon);
};

class ProjectileSystem : public System {
public:
    void update(ECS& ecs, GameManager& gameManager, float deltaTime) override;
private:
    void moveProjectiles(ECS& ecs, float deltaTime);
    void checkProjectileLifetime(ECS& ecs, float deltaTime);
    void handleProjectileCollisions(ECS& ecs);
};
```

#### Rendering Updates:

```cpp
// In RenderSystem, add:
void renderAimingLine(SDL_Renderer* renderer, const AimingLine& line);
void renderCrosshair(SDL_Renderer* renderer, float mouseX, float mouseY);
```

#### Input System Updates:

- Track mouse position and convert to world coordinates
- Handle left mouse button for shooting
- Handle right mouse button for aiming mode (optional)

#### Tasks:

- [ ] Add new components to `Components.h`
- [ ] Create `AimingSystem.h/.cpp`
- [ ] Create `WeaponSystem.h/.cpp`
- [ ] Create `ProjectileSystem.h/.cpp`
- [ ] Update `InputSystem` for mouse tracking
- [ ] Update `RenderSystem` for aiming line and crosshair
- [ ] Add player weapon component initialization
- [ ] Test mouse aiming and shooting

#### Visual Features:

1. **Aiming Line**: Dotted line from player to mouse cursor

   - White/yellow dots when in range
   - Red dots when out of range
   - Line fades with distance

2. **Crosshair**: Small crosshair at mouse position

   - Changes color based on target validity
   - Different styles for different weapons

3. **Projectiles**: Visible bullets/projectiles
   - Move from player toward target
   - Collision detection with enemies
   - Visual effects on impact

---

## 🛠️ Implementation Details

### Phase 1: Project Name Changes

```cmake
# CMakeLists.txt
project(Bloodstrike)  # Change from DodgeTheCreeps

# Also update executable name and paths
```

```bash
# run.sh
echo "🎮 Bloodstrike 2D - C++ ECS Shooter"
./build/Bloodstrike  # Change from DodgeTheCreeps
```

### Phase 2: Window Size Configuration

```cpp
// In Game.h or constants file
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr const char* WINDOW_TITLE = "Bloodstrike 2D";

// In Game.cpp initialization
window = SDL_CreateWindow(WINDOW_TITLE,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    WINDOW_WIDTH, WINDOW_HEIGHT,
    SDL_WINDOW_SHOWN);
```

### Phase 3: Aiming System Flow

1. **Mouse Input**: Track mouse position every frame
2. **World Coordinates**: Convert mouse screen coordinates to world coordinates
3. **Range Check**: Determine if target is within weapon range
4. **Line Calculation**: Calculate dotted line points from player to target
5. **Rendering**: Draw aiming line and crosshair
6. **Shooting**: On left click, create projectile toward target
7. **Projectile Update**: Move projectiles and handle collisions

---

## 🎯 Today's Step-by-Step Plan

### Step 1: Project Rebranding (30 minutes)

1. Update CMakeLists.txt project name
2. Update run.sh script
3. Find and update window title in Game.cpp
4. Build and test

### Step 2: Window Resize (30 minutes)

1. Locate window creation code
2. Update dimensions to 1280x720
3. Test rendering and UI scaling
4. Adjust any hardcoded positions

### Step 3: Aiming System (2-3 hours)

1. Add new components (MouseTarget, AimingLine, Weapon, Projectile)
2. Create AimingSystem - mouse tracking and line calculation
3. Update InputSystem for mouse input
4. Update RenderSystem for aiming line rendering
5. Basic shooting - create projectiles on mouse click
6. ProjectileSystem - move and manage projectile lifecycle

---

## 🚀 Getting Started

Let's start with Phase 1 immediately:

```bash
# Test current build
./run.sh

# Then we'll update the project name and rebuild
```

## 🎮 New Controls (After Phase 3)

- **WASD**: Movement (existing)
- **Mouse**: Aiming direction
- **Left Click**: Shoot
- **Right Click**: Precise aim mode (optional)
- **ESC**: Menu/Quit

---

## 📝 Success Criteria

**Phase 1 Complete**: Game builds and runs with "Bloodstrike" name
**Phase 2 Complete**: Game window is larger and properly scaled  
**Phase 3 Complete**: Player can aim with mouse and shoot projectiles with visual aiming line

Let's get started! 🔫
