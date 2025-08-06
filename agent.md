# Dodge the Creeps - C++ ECS Implementation Design

## Overview

This document outlines the design for converting the Godot "Dodge the Creeps" game to a C++ Entity Component System (ECS) architecture using SDL2 for rendering and input.

## Game Analysis (From Godot Version)

### Core Game Mechanics

1. **Player Movement**: Arrow keys control player character within screen bounds
2. **Mob Spawning**: Enemies spawn at random locations along screen edges at regular intervals
3. **Collision Detection**: Game ends when player touches any mob
4. **Scoring System**: Score increases over time while playing
5. **Game States**: Menu → Playing → Game Over → Menu loop

### Godot Structure Analysis

- **Main Scene**: Game manager, handles spawning, scoring, game states
- **Player Scene**: Character controller with animation and area detection
- **Mob Scene**: Enemy with random movement and self-cleanup
- **HUD Scene**: UI for score display and game messages

## ECS Architecture Design

### Pure ECS Components (Data Only)

```cpp
Transform     // Position, rotation (x, y, rotation)
Velocity      // Movement vector (x, y)
Sprite        // Texture, animation data (texture*, width, height, frames, etc.)
Collider      // Collision bounds (width, height, isTrigger)
PlayerTag     // Empty tag to identify player entities
MobTag        // Empty tag to identify mob entities
Speed         // Movement speed value
Animation     // Animation state (currentFrame, frameTime, etc.)
EntityType    // String identifier for entity type (for JSON loading)
UIText        // Text content, font, color for UI elements
UIPosition    // Screen position for UI elements (different from Transform)
```

### Global Game State (Not Components)

```cpp
class GameManager {
    enum GameState { MENU, PLAYING, GAME_OVER };
    GameState currentState = MENU;
    int score = 0;
    float gameTime = 0.0f;

    // Screen bounds
    float screenWidth = 480;
    float screenHeight = 720;
};
```

### Systems Architecture

#### 1. Timing System

- **Responsibility**: Manage frame timing and delta time
- **Components**: None (system-level timing)
- **Logic**:
  - Calculate delta time between frames
  - Maintain 60 FPS target with frame limiting
  - Provide delta time to all other systems
  - Track FPS for display

#### 2. Input System

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

## Current Status ✅ Phases 1-4 Complete, 🚧 Phase 5 In Progress

🎮 **GAME IS FULLY PLAYABLE!** 🎮

The project has successfully implemented a complete C++ ECS version of "Dodge the Creeps" with:

**✅ COMPLETED FEATURES:**

- ✅ **Core ECS Framework**: Pure component architecture with efficient systems
- ✅ **Player Movement**: Smooth arrow key controls with directional sprites
- ✅ **Mob Spawning**: Random enemy spawns from screen edges with JSON-configured types
- ✅ **Collision Detection**: Player vs mob collision triggering game over
- ✅ **Boundary System**: Off-screen mob cleanup for performance
- ✅ **Animation System**: Sprite animations for player and mobs
- ✅ **Game States**: Menu → Playing → Game Over → Menu loop
- ✅ **HUD System**: Score display, FPS counter, game messages
- ✅ **Resource Management**: Texture and font loading/caching
- ✅ **JSON Configuration**: Data-driven entity definitions
- ✅ **Sky Blue Background**: Visual improvement from original
- ✅ **Directional Player Sprites**: Horizontal vs vertical movement sprites
- ✅ **Proper Sprite Scaling**: Optimized sprite sizes for gameplay

**🚧 CURRENT ENHANCEMENTS IN PROGRESS:**

- Audio system integration (music and sound effects)
- Final gameplay polish and parameter tuning
- Additional visual effects

**📊 TECHNICAL ACHIEVEMENTS:**

- **Performance**: Consistent 60 FPS with delta-time movement
- **Architecture**: Clean ECS separation (core/, components/, systems/, managers/)
- **Maintainability**: JSON-driven configuration for easy modding
- **Code Quality**: Well-organized, documented, and extensible codebase

**🎯 SUCCESS CRITERIA MET:**

- [x] Game runs at consistent 60 FPS ✅
- [x] Player moves smoothly with arrow keys ✅
- [x] Mob entities spawn and move across screen ✅
- [x] Collision detection works (game over) ✅
- [x] Score increases and displays on HUD ✅
- [x] Game state transitions work ✅
- [x] Pure ECS architecture ✅
- [x] Frame-rate independent gameplay ✅
- [x] Visual fidelity matches/exceeds Godot version ✅
- [x] JSON-driven entity system ✅
- [x] UI entities managed by HUD System ✅
- [x] Clean code organization ✅

**🎮 HOW TO PLAY:**

1. Run `./run.sh` from the cpp_version directory
2. Press SPACE to start the game
3. Use arrow keys (or WASD) to move the player
4. Avoid the enemy creatures (mobs) that spawn from the screen edges
5. Try to survive as long as possible to increase your score
6. When hit by a mob, press SPACE to restart

**🔧 TESTING CONFIRMED:**

- Mob spawning system: ✅ Spawns flying/swimming/walking mobs with random speeds
- Boundary system: ✅ Removes off-screen mobs efficiently
- Collision detection: ✅ Detects player-mob collisions accurately
- Movement system: ✅ Smooth directional movement with proper sprites
- Animation system: ✅ Sprite animations working
- Game state management: ✅ Menu → Playing → Game Over transitions

The project now has a clean, organized structure following pure ECS principles:

- **core/**: ECS framework and main game loop
- **components/**: Pure data components (no logic)
- **systems/**: All game logic implementations
- **managers/**: Resource and entity management

The HUD System properly manages UI entities separately from game entities, while the RenderSystem handles the actual drawing of both. The JSON-driven approach makes the game easily moddable and maintainable.
