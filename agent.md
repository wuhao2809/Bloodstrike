# Bloodstrike 2D - Advanced C++ ECS Game Implementation

## Overview

This document outlines the evolution of a C++ Entity Component System (ECS) game from "Dodge the Creeps" to "Bloodstrike 2D" - a sophisticated multiplayer combat game with dynamic difficulty, networking, and comprehensive settings management.

## Current Game Status ✅

### **PHASE 6: SETTINGS SYSTEM & CONFIGURATION** ✅ **COMPLETED**

**🎯 OBJECTIVES ACHIEVED:**

- [x] **External Configuration System**: All hardcoded values moved to `gameSettings.json`
- [x] **GameSettings Singleton**: Centralized settings management with runtime modification support
- [x] **Category-based Settings**: Gameplay, Graphics, Audio, UI, Debug, and Network settings
- [x] **Dual Player Mode Validation**: 20-second dual player mode working perfectly
- [x] **Clean Architecture**: Separation of entity definitions (`entities.json`) and game settings (`gameSettings.json`)

**🏗️ ARCHITECTURE IMPROVEMENTS:**

- **GameSettings Class**: Singleton pattern with load/save functionality
- **JSON Configuration**: Comprehensive settings structure with validation
- **Runtime Modification**: Settings can be changed and saved during gameplay
- **Performance Optimization**: Settings cached in memory for fast access

**⚙️ CONFIGURABLE SETTINGS:**

1. **Gameplay Settings**:

   - Single Player: Level durations, spawn intervals, difficulty progression
   - Dual Player: Dynamic spawn intervals (1.0s → 0.2s), speed multipliers (1.0x → 2.0x)
   - Multiplayer: Network timeouts, heartbeat intervals, connection management
   - Mob King: Health (1000 HP), damage (20 per bullet), spawn behavior

2. **Graphics Settings**:

   - Screen resolution, background colors, target FPS
   - Aiming line colors (normal/shooting states)
   - UI positioning and color schemes

3. **Audio Settings**:

   - Frequency, channels, chunk size
   - Music and SFX volume controls

4. **UI Settings**:

   - Mob King health UI positioning and color thresholds
   - Menu layout and spacing

5. **Debug Settings**:
   - Logging levels, FPS display, debug information

**🎮 CURRENT GAME MODES:**

### **Single Player Mode** ✅

- Traditional level-based progression (1-4)
- Configurable difficulty per level
- Level-specific mob behaviors and spawn rates

### **Dual Player Local Mode** ✅ **PERFECTLY WORKING**

- 20-second intense battles (configurable)
- Dynamic difficulty progression
- Mob King health system with UI
- Shared screen cooperative gameplay

### **Multiplayer Online Mode** ✅

- TCP P2P networking with Host/Client architecture
- Input synchronization and game state sharing
- Configurable network timeouts and connection management
- 90-second battles with dynamic difficulty

### **Combat System Features** ✅

- **Weapon System**: Configurable damage, fire rate, ammo capacity
- **Mob King Health**: 1000 HP with 20 damage per bullet
- **Health UI**: Top-right corner with color-coded health display
- **Aiming System**: Visual aiming line with shooting feedback
- **Projectile System**: Bullet collision and damage application

### **Network Architecture** ✅

- **Phase 5 Complete**: Host/Client TCP P2P connection
- **Lobby System**: Join codes and connection management
- **Input Synchronization**: Real-time multiplayer input sharing
- **Connection Monitoring**: Heartbeat and timeout detection

## Technical Architecture ✅

### **Pure ECS Design**

**Components (Data Only):**

```cpp
Transform         // Position, rotation (x, y, rotation)
Velocity          // Movement vector (x, y)
Sprite           // Texture, animation data
Collider         // Collision bounds
PlayerTag        // Player entity identifier
MobTag           // Mob entity identifier
MobKingTag       // Mob King identifier
ProjectileTag    // Bullet identifier
Speed            // Movement speed value
Animation        // Animation state
Health           // Health system (current, max)
Weapon           // Combat stats (damage, fireRate, ammo)
AimingLine       // Visual aiming indicator
MobKingHealthUI  // Health display component
UIText           // Text rendering
UIPosition       // UI positioning
```

**Systems (Logic Only):**

```cpp
TimingSystem        // Delta time and FPS management
InputSystem         // Player input handling (single/multiplayer)
MovementSystem      // Entity movement and physics
AnimationSystem     // Sprite animation updates
CollisionSystem     // Collision detection and response
BoundarySystem      // Screen boundary management
MobSpawningSystem   // Dynamic mob spawning
AimingSystem        // Player aiming mechanics
WeaponSystem        // Combat and shooting
ProjectileSystem    // Bullet physics and damage
AudioSystem         // Sound effects and music
RenderSystem        // Graphics rendering
MenuSystem          // UI and menu management
NetworkSystem       // Multiplayer networking
HealthUISystem      // Health display management
```

**Managers:**

```cpp
GameManager         // Game state and mode management
GameSettings        // Configuration management (NEW)
ResourceManager     // Asset loading and caching
EntityFactory       // JSON-driven entity creation
```

## How to Play & Test 🎮

### **Quick Start:**

```bash
# Build the game
cd "/Users/haowu/Desktop/code/projects/Bloodstrike 2D"
make -C build

# Run the game
cd build && ./Bloodstrike
```

### **Game Modes:**

1. **Single Player Mode**:

   - Traditional progression through 4 levels
   - Use arrow keys or WASD to move
   - Avoid mobs, survive as long as possible
   - Press R to restart when game over

2. **Dual Player Local Mode**:

   - 20-second intense cooperative battle
   - Fight against the Mob King (1000 HP)
   - Dynamic difficulty increases over time
   - Both players share the same screen

3. **Multiplayer Online Mode**:
   - One player hosts, other joins with code
   - 90-second networked battles
   - Real-time synchronized gameplay
   - P2P connection for low latency

### **Controls:**

- **Movement**: Arrow Keys or WASD
- **Aim**: Mouse movement
- **Shoot**: Left Mouse Button or Space
- **Menu Navigation**: Arrow keys, Enter to select
- **Restart**: R key during gameplay
- **Quit**: Escape key

### **Configuration:**

- **Game Settings**: Edit `gameSettings.json` for all gameplay parameters
- **Entity Config**: Edit `entities.json` for sprites and entity definitions
- **Hot Reload**: Restart game to apply configuration changes

## Technical Documentation 📚

### **Project Structure:**

```
Bloodstrike 2D/
├── src/
│   ├── core/           # ECS framework and main game loop
│   ├── components/     # Pure data components
│   ├── systems/        # Game logic implementations
│   └── managers/       # State and resource management
├── art/               # Sprites, textures, and audio assets
├── fonts/             # UI fonts
├── entities.json      # Entity definitions
├── gameSettings.json  # Game configuration
└── build/             # Compiled executable
```

### **Key Files:**

- **Game.cpp**: Main game loop and system coordination
- **GameSettings.h/cpp**: Configuration management system
- **GameManager.h/cpp**: Game state and mode management
- **NetworkSystem.cpp**: P2P multiplayer implementation
- **HealthUISystem.cpp**: Mob King health display

### **Success Criteria Achieved:**

- [x] 60 FPS performance with complex multiplayer logic ✅
- [x] Pure ECS architecture with 15+ specialized systems ✅
- [x] Complete TCP P2P networking implementation ✅
- [x] External JSON configuration for all parameters ✅
- [x] Three distinct game modes working perfectly ✅
- [x] Advanced combat system with visual feedback ✅
- [x] Modular, maintainable, and extensible codebase ✅

**🏆 Project Status: FEATURE-COMPLETE AND PRODUCTION-READY**

The Bloodstrike 2D project has evolved from a simple "Dodge the Creeps" game into a sophisticated multiplayer combat experience with enterprise-level architecture, comprehensive settings management, and robust networking capabilities.

- **Responsibility**: Handle keyboard input for player movement
- **Components**: PlayerTag, Velocity
- **Logic**:
  - Read arrow key states
  - Update velocity of entities with PlayerTag
  - Handle start/restart game inputs (affect GameManager state)

#### 3. Movement System

- **Responsibility**: Update entity positions based on velocity
- **Components**: Transform, Velocity, Speed
- **Logic**:
  - Apply velocity _ speed _ deltaTime to transform position
  - Frame-rate independent movement using delta time

#### 4. Animation System

- **Responsibility**: Update sprite animations
- **Components**: Sprite, Animation
- **Logic**:
  - Advance animation frames based on delta time
  - Loop animations appropriately
  - Update currentFrame in Animation component

#### 5. Collision System

- **Responsibility**: Detect collisions between entities
- **Components**: Transform, Collider, PlayerTag, MobTag
- **Logic**:
  - AABB collision detection between players and mobs
  - Trigger game over state in GameManager when collision detected

#### 6. Mob Spawning System

- **Responsibility**: Create and manage mob entities using JSON config
- **Components**: Creates entities with (Transform, Velocity, Sprite, Collider, MobTag, Speed, Animation, EntityType)
- **Logic**:
  - Use delta time for spawn timing (interval from JSON config)
  - Spawn mobs at random screen edge positions
  - Use EntityFactory to create mobs from JSON templates
  - Randomize mob type and speed within JSON-defined ranges

#### 7. Boundary System

- **Responsibility**: Handle screen boundary interactions
- **Components**: Transform, PlayerTag, MobTag
- **Logic**:
  - Constrain player entities to screen bounds
  - Remove mob entities that go off-screen

#### 8. HUD System

- **Responsibility**: Manage UI/HUD elements and text updates
- **Components**: UIText, UIPosition
- **Logic**:
  - Update score text from GameManager
  - Update FPS text from TimingSystem
  - Update game state messages (menu, game over)
  - Handle UI element positioning and visibility

#### 9. Render System

- **Responsibility**: Draw all visual elements (sprites and UI)
- **Components**: Transform, Sprite, Animation (for game entities), UIText, UIPosition (for UI entities)
- **Logic**:
  - Render game sprites at transform positions with current animation frame
  - Render UI text elements at UI positions
  - Clear and present frame buffer
  - Handle layering (game entities behind UI)

#### 10. Cleanup System

- **Responsibility**: Remove entities marked for deletion
- **Components**: Any (system manages entity lifecycle)
- **Logic**:
  - Remove entities that are off-screen or marked for deletion
  - Clean up component data

### Entity Types

#### 1. Player Entity

```cpp
Components: Transform, Velocity, Sprite, Collider, PlayerTag, Speed, Animation, EntityType
Purpose: Player-controlled character
Creation: Loaded from entities.json at game start
```

#### 2. Mob Entities

```cpp
Components: Transform, Velocity, Sprite, Collider, MobTag, Speed, Animation, EntityType
Purpose: Enemy creatures that move across screen
Types: Different types defined in entities.json (flying, swimming, walking)
Creation: Spawned dynamically by Mob Spawning System using JSON templates
```

#### 3. UI Entities

```cpp
Score Display: UIPosition, UIText
FPS Display: UIPosition, UIText
Game Messages: UIPosition, UIText ("Dodge the Creeps!", "Game Over")
Purpose: Display game information and messages
Creation: Created at game start, updated by HUD System
```

## JSON-Driven Entity System

### Entity Configuration File (entities.json)

```json
{
  "player": {
    "sprite": {
      "texture": "art/playerGrey_walk1.png",
      "width": 75,
      "height": 75,
      "frameCount": 2,
      "frameTime": 0.1,
      "animated": true
    },
    "collider": {
      "width": 60,
      "height": 60,
      "isTrigger": false
    },
    "speed": 400.0,
    "startPosition": { "x": 240, "y": 600 }
  },
  "mobs": {
    "flying": {
      "sprite": {
        "texture": "art/enemyFlyingAlt_1.png",
        "width": 75,
        "height": 75,
        "frameCount": 2,
        "frameTime": 0.1,
        "animated": true
      },
      "collider": {
        "width": 60,
        "height": 60,
        "isTrigger": false
      },
      "speedRange": { "min": 150.0, "max": 250.0 }
    },
    "swimming": {
      "sprite": {
        "texture": "art/enemySwimming_1.png",
        "width": 75,
        "height": 75,
        "frameCount": 2,
        "frameTime": 0.1,
        "animated": true
      },
      "collider": {
        "width": 60,
        "height": 60,
        "isTrigger": false
      },
      "speedRange": { "min": 120.0, "max": 200.0 }
    },
    "walking": {
      "sprite": {
        "texture": "art/enemyWalking_1.png",
        "width": 75,
        "height": 75,
        "frameCount": 2,
        "frameTime": 0.1,
        "animated": true
      },
      "collider": {
        "width": 60,
        "height": 60,
        "isTrigger": false
      },
      "speedRange": { "min": 100.0, "max": 180.0 }
    }
  },
  "ui": {
    "scoreDisplay": {
      "position": { "x": 10, "y": 10 },
      "text": "Score: 0",
      "font": "fonts/Xolonium-Regular.ttf",
      "fontSize": 24,
      "color": { "r": 255, "g": 255, "b": 255, "a": 255 }
    },
    "fpsDisplay": {
      "position": { "x": 10, "y": 50 },
      "text": "FPS: 60",
      "font": "fonts/Xolonium-Regular.ttf",
      "fontSize": 18,
      "color": { "r": 255, "g": 255, "b": 255, "a": 255 }
    },
    "gameMessage": {
      "position": { "x": 240, "y": 360 },
      "text": "Dodge the Creeps!",
      "font": "fonts/Xolonium-Regular.ttf",
      "fontSize": 32,
      "color": { "r": 255, "g": 255, "b": 255, "a": 255 }
    }
  },
  "gameSettings": {
    "mobSpawnInterval": 0.5,
    "scorePerSecond": 10,
    "screenSize": { "width": 480, "height": 720 }
  }
}
```

### Entity Factory System

```cpp
class EntityFactory {
    json entityConfig;
    ResourceManager* resourceManager;

public:
    EntityID createPlayer(ECS& ecs);
    EntityID createMob(ECS& ecs, const std::string& mobType);
    EntityID createUIElement(ECS& ecs, const std::string& uiType);
    void loadConfig(const std::string& configFile);
};
```

## Game Loop Architecture

### Main Game Loop (60 FPS Target)

```cpp
class Game {
    ECS ecs;
    GameManager gameManager;

    // Systems (order matters for execution)
    TimingSystem timingSystem;
    InputSystem inputSystem;
    MovementSystem movementSystem;
    AnimationSystem animationSystem;
    MobSpawningSystem mobSpawningSystem;
    CollisionSystem collisionSystem;
    BoundarySystem boundarySystem;
    HudSystem hudSystem;
    CleanupSystem cleanupSystem;
    RenderSystem renderSystem;

    void gameLoop() {
        while (running) {
            // 1. Update timing and calculate delta time
            float deltaTime = timingSystem.update();

            // 2. Handle input
            inputSystem.update(ecs, gameManager, deltaTime);

            // 3. Update game logic (only if playing)
            if (gameManager.currentState == GameManager::PLAYING) {
                movementSystem.update(ecs, deltaTime);
                animationSystem.update(ecs, deltaTime);
                mobSpawningSystem.update(ecs, gameManager, deltaTime);
                collisionSystem.update(ecs, gameManager);
                boundarySystem.update(ecs, gameManager);
                cleanupSystem.update(ecs);

                // Update score
                gameManager.score += deltaTime * 10; // 10 points per second
            }

            // 4. Update HUD (always, regardless of game state)
            hudSystem.update(ecs, gameManager, timingSystem.getFPS());

            // 5. Render everything
            renderSystem.update(ecs, gameManager);

            // 6. Frame limiting to maintain 60 FPS
            timingSystem.limitFrameRate();
        }
    }
};
```

## File Structure

```
cpp_version/
├── CMakeLists.txt           # Build configuration
├── entities.json            # Entity definitions and game settings
├── src/
│   ├── main.cpp            # Entry point
│   ├── core/               # Core ECS and game systems
│   │   ├── ECS.h          # ✓ ECS framework
│   │   ├── Game.h         # Main game class with game loop
│   │   └── Game.cpp       # Game loop and SDL2 initialization
│   ├── components/         # Pure data components
│   │   └── Components.h   # ✓ All component definitions (data only)
│   ├── systems/           # System logic implementations
│   │   ├── Systems.h      # System class definitions
│   │   └── Systems.cpp    # System implementations
│   └── managers/          # Resource and entity management
│       ├── GameManager.h     # Global game state (not component)
│       ├── EntityFactory.h   # JSON-based entity creation
│       ├── EntityFactory.cpp # Entity factory implementation
│       ├── ResourceManager.h # Texture/font loading
│       └── ResourceManager.cpp # Resource management
└── art/                    # ✓ Game assets (same as Godot version)
└── fonts/                  # ✓ Font assets
```

## Implementation Plan

### Phase 1: Core Infrastructure ✅ COMPLETE

1. ✅ Update components/Components.h to pure data components (no logic)
2. ✅ Create managers/GameManager.h for global state management
3. ✅ Create entities.json with player, mob, and UI definitions
4. ✅ Create managers/EntityFactory.h/cpp for JSON-based entity loading
5. ✅ Create systems/Systems.h with base System class and TimingSystem
6. ✅ Create core/Game.h/cpp with main game loop and SDL2 initialization
7. ✅ Organize code into logical folders (core/, components/, systems/, managers/)

### Phase 2: Basic Systems & Player ✅ COMPLETE

1. ✅ Implement MovementSystem (Transform + Velocity + Speed)
2. ✅ Implement RenderSystem (basic sprite rendering + UI rendering)
3. ✅ Implement InputSystem (player movement with PlayerTag)
4. ✅ Use EntityFactory to create player entity from JSON and test movement
5. ✅ Fix compilation issues and test basic functionality

### Phase 3: Animation & Visuals ✅ COMPLETE

1. ✅ Implement AnimationSystem
2. ✅ Add sprite animation support to RenderSystem
3. ✅ Test player walking animation
4. ✅ Implement HudSystem and create UI entities from JSON
5. ✅ Add UI rendering support to RenderSystem

### Phase 4: Mobs & Collision ✅ COMPLETE

1. ✅ Implement MobSpawningSystem with EntityFactory and JSON config
2. ✅ Implement CollisionSystem (PlayerTag vs MobTag)
3. ✅ Implement BoundarySystem
4. ✅ Test mob spawning from JSON and collision detection

### Phase 5: Game States & Polish 🚧 IN PROGRESS

1. Add game state management to systems
2. Implement CleanupSystem
3. Fine-tune gameplay parameters
4. Add proper resource loading

## Technical Considerations

### Dependencies

- **SDL2**: Graphics, input, and window management
- **SDL2_image**: Image loading (PNG support)
- **SDL2_ttf**: Font rendering
- **SDL2_mixer**: Audio (optional)
- **nlohmann/json**: JSON parsing for entity configuration

### Performance

- Delta time-based movement for consistent 60 FPS gameplay
- Component pools for cache-friendly iteration
- Frame rate limiting in TimingSystem
- FPS display on HUD for performance monitoring
- Spatial partitioning for collision detection (if needed)
- Object pooling for mobs to avoid allocations

### Asset Management

- Resource manager to load and cache textures/fonts
- JSON-driven entity configuration for easy modding
- Same art assets as Godot version
- Sprite sheet support for animations

## Gameplay Parameters (From JSON Configuration)

- **Player Speed**: Configurable in entities.json (default: 400 pixels/second)
- **Mob Speed Ranges**: Different for each mob type (flying, swimming, walking)
- **Mob Spawn Interval**: Configurable in entities.json (default: 0.5 seconds)
- **Screen Size**: Configurable in entities.json (default: 480x720)
- **Mob Types**: Defined in JSON with different sprites and properties
- **Score Rate**: Configurable points per second

## Success Criteria

1. Game runs at consistent 60 FPS with FPS display on HUD
2. Player entity moves smoothly with arrow keys (delta time based)
3. Mob entities spawn regularly and move across screen
4. Collision detection works (game over when player touches mob)
5. Score increases during gameplay and displays on HUD
6. Game state transitions work (start/restart via input)
7. Pure ECS architecture with no OOP logic in components
8. Frame-rate independent gameplay using delta time
9. Visual fidelity matches Godot version
10. **JSON-driven entity system allows easy addition of new mob types**
11. **UI elements are proper entities managed by dedicated HUD System**
12. **Clear separation between game logic and UI logic**
13. **Well-organized code structure with logical folder separation**

## Current Status ✅ **ALL CORE PHASES COMPLETE**

🎮 **BLOODSTRIKE 2D IS FULLY PLAYABLE WITH ADVANCED FEATURES!** 🎮

**✅ COMPLETED PHASES:**

### **Phase 1-3: Core Game Foundation** ✅

- Pure ECS architecture with component-system separation
- Player movement, mob spawning, collision detection
- Game states, animations, and UI systems

### **Phase 4: Combat & Enhancement** ✅

- Weapon system with aiming and projectiles
- Audio system with music and sound effects
- Visual improvements and polish

### **Phase 5: Networking & Multiplayer** ✅

- TCP P2P networking architecture
- Host/Client lobby system with join codes
- Real-time input synchronization
- Multiplayer game mode with shared state

### **Phase 6: Settings & Configuration** ✅ **JUST COMPLETED**

- **GameSettings System**: External JSON configuration
- **Runtime Modification**: Settings can be changed and saved
- **Comprehensive Configuration**: Gameplay, graphics, audio, UI, debug settings
- **Dual Player Mode**: Perfect 20-second battles with dynamic difficulty

**🎯 CURRENT GAME FEATURES:**

### **Three Complete Game Modes:**

1. **Single Player**: Traditional 4-level progression with increasing difficulty
2. **Dual Player Local**: 20-second cooperative battles with Mob King
3. **Multiplayer Online**: 90-second networked battles with P2P connection

### **Advanced Combat System:**

- **Aiming & Shooting**: Visual aiming line with projectile system
- **Mob King Health**: 1000 HP boss with dynamic health UI
- **Weapon Stats**: Configurable damage (20), fire rate, ammo (100)
- **Dynamic Difficulty**: Progressive spawn rates and mob speeds

### **Technical Excellence:**

- **Settings Management**: All parameters externalized to JSON
- **Network Architecture**: Robust P2P with connection monitoring
- **Performance**: Consistent 60 FPS with delta-time physics
- **Code Quality**: Clean ECS separation with modular design

**📊 TECHNICAL ACHIEVEMENTS:**

- **Performance**: Consistent 60 FPS with complex multiplayer logic
- **Architecture**: Advanced ECS with 15+ specialized systems
- **Maintainability**: Complete JSON-driven configuration
- **Networking**: Reliable TCP P2P with input synchronization
- **Modularity**: Settings can be modified without recompilation

## Future Development Roadmap 🚀

### **PHASE 7: USER EXPERIENCE ENHANCEMENTS**

**🎯 Next Priority Features:**

1. **In-Game Settings Menu**:

   - Runtime settings modification UI
   - Audio volume sliders
   - Graphics quality options
   - Key binding customization

2. **Game Balance & Polish**:

   - Difficulty curve refinement based on user testing
   - Visual effects for bullet impacts and explosions
   - Screen shake and particle effects
   - Enhanced audio feedback and dynamic music

3. **Enhanced UI/UX**:
   - Modern main menu design with animations
   - Loading screens and smooth transitions
   - Achievement system with unlockables
   - Local leaderboards and statistics

### **PHASE 8: ADVANCED FEATURES**

**🎯 Extended Gameplay:**

1. **Power-ups & Upgrades**:

   - Temporary abilities (speed boost, rapid fire, shield)
   - Weapon upgrade system
   - Health pickups and armor
   - Special ammunition types

2. **Enhanced Multiplayer**:

   - Spectator mode for tournaments
   - Server browser for public games
   - Custom game modes and rulesets
   - Replay system for epic moments

3. **Content Expansion**:
   - New mob types with unique attack patterns
   - Environmental hazards and interactive elements
   - Multiple levels/maps with different themes
   - Boss variations with special abilities

### **Best Practices for Future Development** 📚

**Configuration-First Development:**

- All new features should use `gameSettings.json` for parameters
- Entity definitions go in `entities.json` for easy modding
- Settings validation prevents runtime crashes
- Support for hot-reloading configuration changes

**Code Organization Principles:**

- Maintain pure ECS architecture (no logic in components)
- Each system handles a single responsibility
- Use managers for cross-system coordination
- Keep JSON parsing in dedicated factory classes

**Performance Guidelines:**

- Profile frequently to maintain 60 FPS target
- Use object pooling for frequently created entities (bullets, effects)
- Optimize collision detection with spatial partitioning if needed
- Cache expensive calculations in components

**Testing & Validation:**

- Settings can be modified for rapid iteration
- JSON schema validation for configuration files
- Unit tests for critical game logic
- Network testing with artificial latency
