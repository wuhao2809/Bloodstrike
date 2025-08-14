# 🎮 Bloodstrike 2D - Engine vs Game Architecture

## 📋 Current Architecture Analysis

### 🔧 **Game Engine Components (Reusable)**

These parts should be **engine-agnostic** and reusable for any 2D game:

```
src/
├── engine/                    # 🎯 REUSABLE ENGINE
│   ├── core/
│   │   ├── ECS.h              ✅ Generic entity-component system
│   │   ├── Application.h      ❌ Currently Game.h (game-specific)
│   │   └── Engine.h           ❌ Missing - needs extraction
│   ├── systems/
│   │   ├── System.h           ✅ Generic system interface
│   │   ├── RenderSystem.*     ✅ Generic 2D rendering
│   │   ├── AudioSystem.*      ✅ Generic audio playback
│   │   ├── InputSystem.*      ✅ Generic input handling
│   │   ├── MovementSystem.*   ✅ Generic movement/physics
│   │   ├── AnimationSystem.*  ✅ Generic sprite animation
│   │   ├── NetworkSystem.*    ✅ Generic networking
│   │   ├── BoundarySystem.*   ✅ Generic boundary checks
│   │   └── TimingSystem.*     ✅ Generic timing/deltaTime
│   ├── components/
│   │   ├── Transform.*        ✅ Generic position/rotation
│   │   ├── Sprite.*           ✅ Generic sprite rendering
│   │   ├── Velocity.*         ✅ Generic movement
│   │   ├── Collider.*         ✅ Generic collision detection
│   │   ├── Health.*           ✅ Generic health system
│   │   └── Audio.*            ✅ Generic audio components
│   └── managers/
│       ├── ResourceManager.*  ✅ Generic asset loading
│       ├── ConfigManager.*    ✅ Generic configuration
│       └── SceneManager.*     ❌ Missing - needs creation
```

### 🎯 **Game-Specific Components (Bloodstrike 2D)**

These parts are **specific to your shooter game**:

```
src/
├── game/                      # 🎮 GAME-SPECIFIC
│   ├── BloodstrikeGame.*      ❌ Currently Game.* (mixed)
│   ├── systems/
│   │   ├── WeaponSystem.*     ❌ Shooter-specific
│   │   ├── ProjectileSystem.* ❌ Shooter-specific
│   │   ├── CollisionSystem.*  ❌ Game-specific collision rules
│   │   ├── MobSpawningSystem.*❌ Game-specific mob logic
│   │   ├── AimingSystem.*     ❌ Shooter-specific
│   │   ├── HealthUISystem.*   ❌ Game-specific UI
│   │   └── MenuSystem.*       ❌ Game-specific menus
│   ├── components/
│   │   ├── PlayerTag.*        ❌ Game-specific
│   │   ├── MobTag.*           ❌ Game-specific
│   │   ├── ProjectileTag.*    ❌ Game-specific
│   │   ├── Weapon.*           ❌ Shooter-specific
│   │   ├── Projectile.*       ❌ Shooter-specific
│   │   └── MobKing.*          ❌ Game-specific boss
│   ├── managers/
│   │   ├── GameManager.*      ❌ Game-specific logic
│   │   ├── EntityFactory.*    ❌ Game-specific entity creation
│   │   └── GameSettings.*     ❌ Game-specific settings
│   └── main.cpp               ❌ Game entry point
```

---

## 🔄 **Refactoring Plan: Engine Separation**

### **Step 1: Create Engine Namespace**

```cpp
// engine/core/Engine.h
namespace BloodstrikeEngine {
    class Application {
        virtual void initialize() = 0;
        virtual void update(float deltaTime) = 0;
        virtual void render() = 0;
        virtual void shutdown() = 0;
    };

    class Engine {
        void run(Application* app);
        ECS& getECS();
        ResourceManager& getResourceManager();
    };
}
```

### **Step 2: Game Implementation**

```cpp
// game/BloodstrikeGame.h
#include "engine/core/Engine.h"

class BloodstrikeGame : public BloodstrikeEngine::Application {
public:
    void initialize() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

// main.cpp
int main() {
    BloodstrikeEngine::Engine engine;
    BloodstrikeGame game;
    engine.run(&game);
    return 0;
}
```

---

## 📁 **Proposed Directory Structure**

```
src/
├── engine/                         # 🎯 REUSABLE ENGINE
│   ├── core/
│   │   ├── ECS.h                   # Entity-Component-System
│   │   ├── Engine.h                # Main engine class
│   │   ├── Application.h           # Game interface
│   │   └── Scene.h                 # Scene management
│   ├── systems/
│   │   ├── System.h                # Base system interface
│   │   ├── RenderSystem.*          # 2D rendering
│   │   ├── AudioSystem.*           # Audio playback
│   │   ├── InputSystem.*           # Input handling
│   │   ├── MovementSystem.*        # Physics/movement
│   │   ├── AnimationSystem.*       # Sprite animation
│   │   ├── NetworkSystem.*         # Multiplayer networking
│   │   ├── BoundarySystem.*        # Screen boundaries
│   │   └── TimingSystem.*          # Delta time management
│   ├── components/
│   │   ├── Transform.h             # Position, rotation, scale
│   │   ├── Sprite.h                # Sprite rendering
│   │   ├── Velocity.h              # Movement velocity
│   │   ├── Collider.h              # Collision detection
│   │   ├── Health.h                # Health/damage system
│   │   ├── AudioSource.h           # Audio playback
│   │   └── NetworkEntity.h         # Network synchronization
│   ├── managers/
│   │   ├── ResourceManager.*       # Asset loading (textures, sounds)
│   │   ├── ConfigManager.*         # Configuration files
│   │   ├── SceneManager.*          # Scene switching
│   │   └── AssetCache.*            # Asset caching
│   └── utils/
│       ├── Math.h                  # Vector math utilities
│       ├── Logger.h                # Logging system
│       └── Timer.h                 # High-resolution timing
│
├── game/                           # 🎮 BLOODSTRIKE-SPECIFIC
│   ├── BloodstrikeGame.*           # Main game class
│   ├── systems/
│   │   ├── WeaponSystem.*          # Weapon mechanics
│   │   ├── ProjectileSystem.*      # Bullet physics
│   │   ├── CombatSystem.*          # Combat logic
│   │   ├── MobSpawningSystem.*     # Enemy spawning
│   │   ├── AimingSystem.*          # Mouse aiming
│   │   ├── HealthUISystem.*        # Health bars, UI
│   │   ├── MenuSystem.*            # Game menus
│   │   └── GameplaySystem.*        # Score, levels, etc.
│   ├── components/
│   │   ├── GameTags.h              # PlayerTag, MobTag, etc.
│   │   ├── Weapon.h                # Weapon properties
│   │   ├── Projectile.h            # Bullet properties
│   │   ├── MobKing.h               # Boss mechanics
│   │   └── GameUI.h                # UI components
│   ├── managers/
│   │   ├── GameManager.*           # Game state, score
│   │   ├── EntityFactory.*         # Game entity creation
│   │   ├── GameSettings.*          # Game-specific settings
│   │   └── LevelManager.*          # Level progression
│   ├── scenes/
│   │   ├── MenuScene.*             # Main menu
│   │   ├── GameplayScene.*         # Core gameplay
│   │   ├── LobbyScene.*            # Multiplayer lobby
│   │   └── GameOverScene.*         # End game screen
│   └── data/
│       ├── weapons.json            # Weapon configurations
│       ├── levels.json             # Level definitions
│       └── enemies.json            # Enemy types
│
└── main.cpp                        # Game entry point
```

---

## 🎯 **Making the Engine Reusable**

### **1. Abstract Interfaces**

```cpp
// engine/core/Application.h
class Application {
public:
    virtual ~Application() = default;
    virtual bool initialize() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void handleEvent(const Event& event) = 0;
    virtual void shutdown() = 0;
};
```

### **2. Component Registration System**

```cpp
// engine/core/ComponentRegistry.h
template<typename T>
void registerComponent() {
    ComponentRegistry::instance().register<T>();
}

// In game code:
void BloodstrikeGame::initialize() {
    // Register game-specific components
    registerComponent<Weapon>();
    registerComponent<Projectile>();
    registerComponent<MobKing>();
}
```

### **3. System Factory Pattern**

```cpp
// engine/core/SystemFactory.h
class SystemFactory {
public:
    template<typename T>
    void registerSystem(const std::string& name) {
        creators[name] = []() { return std::make_unique<T>(); };
    }

    std::unique_ptr<System> createSystem(const std::string& name);
};

// In game configuration:
// systems.json
{
    "systems": [
        "RenderSystem",
        "AudioSystem",
        "WeaponSystem",      // Game-specific
        "ProjectileSystem"   // Game-specific
    ]
}
```

### **4. Event System**

```cpp
// engine/core/EventSystem.h
class EventSystem {
public:
    template<typename T>
    void subscribe(std::function<void(const T&)> handler);

    template<typename T>
    void publish(const T& event);
};

// Usage in game:
eventSystem.subscribe<ProjectileHitEvent>([](const auto& event) {
    // Handle projectile collision
});
```

---

## 🚀 **Benefits of Engine Separation**

### **Reusability:**

- ✅ Use engine for platformer games
- ✅ Use engine for RPGs
- ✅ Use engine for puzzle games
- ✅ Share networking code across projects

### **Maintainability:**

- ✅ Clear separation of concerns
- ✅ Engine bugs don't affect game logic
- ✅ Game changes don't break engine
- ✅ Easier unit testing

### **Extensibility:**

- ✅ Plugin system for new components
- ✅ Modular system architecture
- ✅ Easy to add new game types
- ✅ Configuration-driven setup

---

## 📋 **Migration Checklist**

### **Phase 1: Core Separation**

- [ ] Extract `Engine` class from `Game.cpp`
- [ ] Create `Application` interface
- [ ] Move generic systems to `engine/systems/`
- [ ] Move generic components to `engine/components/`

### **Phase 2: Component Registry**

- [ ] Implement component registration system
- [ ] Move game-specific components to `game/components/`
- [ ] Update component usage in systems

### **Phase 3: System Factory**

- [ ] Implement system factory pattern
- [ ] Create system configuration files
- [ ] Move game-specific systems to `game/systems/`

### **Phase 4: Scene Management**

- [ ] Implement scene system
- [ ] Create game-specific scenes
- [ ] Update main loop to use scenes

### **Phase 5: Configuration**

- [ ] Create engine configuration system
- [ ] Move game settings to configuration files
- [ ] Implement asset manifest system

---

**This refactoring will make your engine truly reusable while keeping Bloodstrike 2D as a clean example game built on top of it!** 🎮✨
