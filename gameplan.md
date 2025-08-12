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

### ✅ **Phase 1: Project Rebranding** (COMPLETED)

**Goal**: Change project name from "Dodge the Creeps" to "Bloodstrike"

#### Tasks:

- [x] Update `CMakeLists.txt` project name from "DodgeTheCreeps" to "Bloodstrike"
- [x] Update `run.sh` script text and executable name
- [x] Update any hardcoded strings in source files
- [x] Update README.md if exists
- [x] Test build system works with new name

**✅ COMPLETED**: All references successfully updated and tested!

---

### ✅ **Phase 2: Larger Window Size** (COMPLETED)

**Goal**: Increase game window dimensions for better gameplay

#### Implemented Changes:

- **Current**: Window properly sized at 1280x720 from entities.json
- **Fixed**: Initialization order to load JSON before creating window
- **Working**: All game systems scale properly with new dimensions

#### Tasks:

- [x] Locate window creation code in `Game.cpp`
- [x] Update window dimensions
- [x] Ensure UI elements scale properly ⚠️ (NEEDS FIXING)
- [x] Update any hardcoded position calculations
- [x] Test rendering performance

**✅ COMPLETED**: Window correctly sized, but UI positioning needs adjustment.

---

### ✅ **Phase 3: Aiming & Shooting System** (COMPLETE)

**Goal**: Implement mouse-based aiming with visual feedback

**Status**: ✅ **IMPLEMENTED AND WORKING**

- ✅ Mouse targeting system with aiming line visualization
- ✅ Shooting mechanics with projectile creation and movement
- ✅ Collision detection between projectiles and mobs
- ✅ Weapon system with fire rate limiting and ammo management
- ✅ Visual feedback (crosshair, dotted aiming lines)

**Implementation Notes**:

- ✅ AimingSystem tracks mouse position and calculates direction vectors
- ✅ WeaponSystem handles shooting with fire rate (5 shots/sec) and ammo (30 rounds)
- ✅ ProjectileSystem manages bullet movement and collision detection
- ✅ RenderSystem enhanced with dotted line rendering for aiming feedback
- ✅ All systems properly integrated with ECS architecture
- ✅ **JSON Configuration**: Combat components now load from `entities.json` ensuring proper reset on game restart
- ✅ **Restart Fix**: Player can shoot immediately after game restart (components reset properly)

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

---

### 🚀 **Phase 4: Multiplayer P2P System** (IN PROGRESS)

**Goal**: Implement peer-to-peer multiplayer with asymmetric Player vs Mob King gameplay

**Current Status**: Phase 4a (Menu System) ✅ COMPLETED, Phase 4b-partial ✅ COMPLETED, **NEW Phase 4b-alternate: Dual Player Local Mode** ✅ COMPLETED

#### Game Design:

**Three Game Modes:**

1. **Single Player**: Current 4-level system (keep existing) - Mobs can shoot at Level 4
2. **Dual Player (Local)**: **Player vs Human Mob King** - **UPDATED IMPLEMENTATION** ✅ 
3. **Multiplayer (Online)**: 2-minute survival challenge - Future implementation

**✅ UPDATED: Dual Player Local Mode** 

**Human vs Human Local Controls:**
- **Player 1 (Survivor)**: WASD/Arrow keys movement, Mouse aim & click to shoot
- **Player 2 (Mob King)**: IJKL movement, P key to shoot in facing direction

**Combat System Update:**
- **Regular Mobs**: Cannot shoot in dual player mode (simplified for balance)
- **Mob King (Human-Controlled)**: Full combat abilities with directional shooting
- **Single Player Level 4**: Regular mobs still get weapons (existing behavior preserved)
- **Dual Player Mode**: Only human Mob King can shoot, regular mobs are basic

**Dual Player Rules:**
- **Player 1**: Survives against waves + Human Mob King (mouse aim & click)
- **Player 2**: Controls Mob King with IJKL movement + P to shoot in facing direction
- **Regular Mob Spawn**: Reduced frequency (2x interval) to balance with human Mob King
- **Mob King Stats**: 150 health, high damage (30), medium fire rate (1.5), long range (600)
- **Victory**: Player 1 wins by completing levels, Player 2 wins by eliminating Player 1

**Technical Implementation:**
```cpp
// ✅ COMPLETED COMPONENTS
enum class GameMode {
    SINGLE_PLAYER,      // ✅ Existing behavior
    DUAL_PLAYER_LOCAL,  // ✅ UPDATED: Human vs Human local
    MULTIPLAYER_ONLINE  // 🚧 Future: Human vs Human online
};

// ✅ COMPLETED: Input System Updates
// - Player 1: WASD/Arrow keys + mouse shooting
// - Player 2 (Mob King): IJKL movement + P directional shooting

// ✅ COMPLETED: Weapon System Updates  
void handleMobKingShooting(ECS &ecs, float deltaTime);      // Human Mob King shooting
void handleRegularMobShooting(ECS &ecs, float deltaTime);   // AI mob shooting

// ✅ COMPLETED: Smart shooting mechanics
// - Mob King: Shoots in movement direction (IJKL-based)
// - Regular mobs: Auto-aim at player (single player Level 4 only)
// - Dual player: Only human Mob King can shoot
```

#### Game Design:

**Two Game Modes:**

1. **Single Player**: Current 4-level system (keep existing)
2. **Multiplayer**: 2-minute survival challenge

**Multiplayer Rules:**

- **Player**: Survive 2 minutes to win (same controls as single-player)
- **Mob King**: Kill player to win (JKIL movement, ] for acceleration boost)
- **Dynamic Difficulty**: Linear increase in mob spawn rate and speed over 2 minutes
- **Mob King Respawn**: 8-second timer when killed by player
- **Victory Conditions**: Player wins if survives 2min, Mob King wins if kills player

#### Technical Requirements:

**Networking Layer:**

- SDL_net TCP Host-Client architecture (Host has authority)
- Game state synchronization at 60 FPS
- Input prediction and lag compensation
- Connection handling (join, leave, disconnect)

**Collision Authority Design:**

- **Host Authority**: Mob spawning, mob-player collisions, victory conditions
- **Client Authority**: Own projectile collisions (immediate feedback)
- **Synchronization**: Collision results synced between clients

**Why This Hybrid Approach:**

- **Responsive Shooting**: Players get immediate feedback when hitting mobs
- **Fair Gameplay**: Host decides critical collisions (player death, victory)
- **Reduced Lag**: No waiting for host confirmation on every shot
- **Anti-Cheat**: Host validates all game-ending events

**New Systems Needed:**

```cpp
class NetworkSystem : public System {
    // Handle P2P connections, message sending/receiving
    // Synchronize game state between players
};

class MultiplayerGameManager : public GameManager {
    // Override single-player logic for multiplayer rules
    // Handle role assignment (Player vs Mob King)
    // Manage 2-minute timer and dynamic difficulty
};

class MobKingSystem : public System {
    // Handle human-controlled mob king movement
    // Manage acceleration boost (P key)
    // Handle respawn timer (8 seconds)
};
```

**New Components:**

```cpp
struct NetworkPlayer {
    uint32_t playerID;
    PlayerRole role; // PLAYER or MOB_KING
    bool isLocal;
};

struct MobKing {
    float accelerationBoost = 2.0f;
    float boostDuration = 1.0f;
    float boostCooldown = 3.0f;
    bool isAccelerating = false;
    float respawnTimer = 0.0f;
    bool isDead = false;
};

struct MultiplayerGameState {
    float survivalTimer = 120.0f; // 2 minutes
    float difficultyMultiplier = 1.0f;
    uint32_t playerID;
    uint32_t mobKingID;
};
```

#### Implementation Phases:

**✅ Phase 4a: Menu System Enhancement** (COMPLETED)

**Status**: ✅ **IMPLEMENTED AND WORKING**

- ✅ Add main menu with Single Player / Dual Player / Multiplayer options
- ✅ Menu navigation with W/S and arrow keys  
- ✅ Proper menu centering and state management
- ✅ Clean separation between menu and game UI
- ✅ JSON-configurable menu system with entities.json
- ✅ Host/Join game interface for multiplayer preparation
- ✅ Lobby screen with "waiting for player" status
- ✅ NetworkSystem integration (foundation ready)

**✅ Phase 4b-alternate: Dual Player Local Mode** (COMPLETED)

**Status**: ✅ **IMPLEMENTED AND WORKING** - Human vs Human local multiplayer!

- ✅ Added "Dual Player" menu option between Single Player and Multiplayer
- ✅ GameManager support for DUAL_PLAYER_LOCAL mode
- ✅ Mob King spawning system (Level 2+, 150 health, combat ready)
- ✅ **Human Mob King Controls**: IJKL movement + P directional shooting
- ✅ **InputSystem Updates**: Player 1 (WASD+mouse) vs Player 2 (IJKL+P)
- ✅ **WeaponSystem Updates**: Mob King shoots in facing direction, not auto-aim
- ✅ Smart combat system: Only human Mob King can shoot in dual player mode
- ✅ Balanced spawn rates: Regular mobs spawn at 2x interval when Mob King present
- ✅ Health component added for boss entities
- ✅ Complete game reset system for mode switching
- ✅ Mob King stats: Damage=30, Range=600, FireRate=1.5, Health=150

**New Controls Added:**
```
Player 1 (Survivor):     Player 2 (Mob King):
- WASD/Arrow movement    - I J K L movement
- Mouse aim & click      - P to shoot (facing direction)
```

**Implementation Advantages:**
- **Local Couch Co-op**: Perfect 2-player local experience
- **Code Reuse Ready**: Local mechanics will translate directly to online multiplayer
- **Balanced Combat**: Human vs Human tested dynamics
- **Menu Flow**: Complete navigation between Single → Dual → Multiplayer ready
- **Input Separation**: Clear control schemes for both players
- **Network Preparation**: All game mode infrastructure ready for online implementation

**🚀 Phase 4b: Networking Foundation** (NEXT PRIORITY - 2-3 days)

**Goal**: Implement basic networking infrastructure for P2P multiplayer

- 🚧 Implement SDL_net TCP P2P networking
- 🚧 Create NetworkSystem for message handling  
- 🚧 Implement basic client-server handshake
- 🚧 Add Host/Join game interface to menu system
- 🚧 Add lobby screen with "waiting for player" status
- 🚧 Handle connection/disconnection events

**Key Components Needed**:
```cpp
class NetworkSystem : public System {
    // TCP socket management
    // Message serialization/deserialization  
    // Connection state handling
};

struct NetworkMessage {
    MessageType type;
    uint32_t playerID;
    uint32_t timestamp;
    // Message payload
};
```

**Phase 4c: Game State Synchronization** (2-3 days)

- Synchronize player positions and inputs
- Handle mob spawning across both clients
- Implement projectile synchronization

**Phase 4d: Mob King System** (1-2 days)

- Implement human-controlled mob king
- Add acceleration boost mechanics (P key)
- Add 8-second respawn system

**Phase 4e: Multiplayer Game Rules** (1-2 days)

- Implement 2-minute survival timer
- Add dynamic difficulty scaling
- Handle victory/defeat conditions

**Phase 4f: Polish & Testing** (2-3 days)

- Add network lag compensation
- Handle disconnections gracefully
- Add multiplayer UI elements
- Extensive testing

#### Technical Challenges:

1. **State Synchronization**: Keep both games in sync without lag
2. **Input Prediction**: Smooth movement despite network latency
3. **Collision Authority**: Decide which client handles collision detection
4. **Disconnect Handling**: Graceful degradation when connection lost
5. **Dynamic Difficulty**: Smooth scaling over 2 minutes

#### Network Protocol Design:

```cpp
enum MessageType {
    // Connection & Lobby
    CONNECTION_REQUEST,  // Join game request
    CONNECTION_ACCEPT,   // Connection established
    DICE_ROLL_REQUEST,   // Initiate dice roll
    DICE_ROLL_RESULT,    // Dice roll outcome
    ROLE_SELECTION,      // Player chooses role
    GAME_START,          // Begin multiplayer match

    // Game Play
    PLAYER_INPUT,        // WASD + mouse + shooting
    MOB_KING_INPUT,      // JKIL + ] acceleration
    GAME_STATE_UPDATE,   // Positions, health, score
    MOB_SPAWN,           // New mob creation (Host authority)
    PROJECTILE_CREATE,   // Bullet creation
    PROJECTILE_HIT,      // Collision result sync
    MOB_KING_DEATH,      // Respawn timer start
    GAME_OVER           // Victory/defeat
};
```

---

### Phase 4: Multiplayer Architecture

**Main Menu Flow:**

```
Main Menu
├── Single Player → Current 4-level system
└── Multiplayer
    ├── Host Game → Wait for connection
    └── Join Game → Enter host IP
                 ↓
            Both Players Connected
                 ↓
            Dice Roll for Role Priority
                 ↓
            Role Selection (Winner picks first)
                 ↓
            Game Start → Player vs Mob King
```

**Multiplayer Game Loop:**

```cpp
class MultiplayerGameManager : public GameManager {
private:
    float survivalTimer = 120.0f;  // 2 minutes
    PlayerRole localPlayerRole;
    float difficultyProgression;

public:
    void updateMultiplayerLogic(float deltaTime) {
        // Update 2-minute survival timer
        survivalTimer -= deltaTime;

        // Calculate dynamic difficulty (linear increase over 2 minutes)
        difficultyProgression = 1.0f + (120.0f - survivalTimer) / 120.0f;

        // Increase mob spawn rate over time
        mobSpawnInterval = 0.5f / difficultyProgression;

        // Increase mob speed over time
        mobSpeedMultiplier = 1.0f * difficultyProgression;

        // Check victory conditions
        if (survivalTimer <= 0.0f && localPlayerRole == PLAYER) {
            // Player wins by surviving 2 minutes
            gameOver(PLAYER_VICTORY);
        }
    }
};
```

**Network Message Structure:**

```cpp
struct NetworkMessage {
    MessageType type;
    uint32_t playerID;
    uint32_t timestamp;
    union {
        PlayerInputData playerInput;
        MobKingInputData mobKingInput;
        GameStateData gameState;
        ProjectileData projectile;
        DiceRollData diceRoll;
        RoleSelectionData roleSelection;
    } data;
};

struct DiceRollData {
    uint8_t playerDiceValue;
    uint8_t opponentDiceValue;
    uint32_t winnerPlayerID;
};

struct RoleSelectionData {
    PlayerRole selectedRole; // PLAYER or MOB_KING
    bool isConfirmed;
};
```

**Dice Roll & Role Selection Flow:**

```cpp
class LobbySystem : public System {
private:
    enum LobbyState {
        WAITING_FOR_CONNECTION,
        DICE_ROLL_PHASE,
        ROLE_SELECTION_PHASE,
        READY_TO_START
    };

public:
    void initiateDiceRoll() {
        // Both players roll dice (1-6)
        // Higher roll gets first pick of role
        // Display animated dice rolling for 2-3 seconds
        // Winner sees "You won! Choose your role first"
        // Loser sees "Opponent won, waiting for their choice..."
    }

    void handleRoleSelection(uint32_t winnerPlayerID) {
        // Winner picks Player or Mob King
        // Loser automatically gets remaining role
        // Display final role assignments
        // 3-second countdown before game starts
    }
};
```

**Lobby UI Flow:**

1. **Connection Phase:**

   ```
   Host: "Waiting for player to join..."
   Join: "Connecting to [IP]..."
   ```

2. **Dice Roll Phase:**

   ```
   "Both players connected!"
   "Rolling dice to decide role priority..."
   [Animated dice: 🎲 🎲]
   "You rolled: 4, Opponent rolled: 6"
   "Opponent wins! They choose first."
   ```

3. **Role Selection Phase:**

   ```
   Winner: "Choose your role:"
           [Player Button] [Mob King Button]

   Loser:  "Waiting for opponent to choose..."
           "Opponent chose: Player"
           "You are: Mob King"
   ```

4. **Game Start:**
   ```
   "Final Roles: You=Mob King, Opponent=Player"
   "Game starting in 3... 2... 1..."
   ```

**Dice Roll Implementation:**

```cpp
class DiceRollSystem {
private:
    std::random_device rd;
    std::mt19937 gen;

public:
    DiceRollSystem() : gen(rd()) {}

    uint8_t rollDice() {
        std::uniform_int_distribution<> dis(1, 6);
        return dis(gen);
    }

    void synchronizeDiceRoll(NetworkSystem* network) {
        // Both players roll simultaneously
        // Send results to each other
        // Determine winner (higher roll wins, re-roll on tie)
        // Winner gets first choice of role
    }
};
```

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

## 🎮 New Controls (After Phase 4)

**Single Player Mode** (unchanged):

- **WASD**: Movement
- **Mouse**: Aiming direction
- **Left Click**: Shoot
- **ESC**: Menu/Quit

**Multiplayer Mode**:

_Player Role_:

- **WASD**: Movement
- **Mouse**: Aiming direction
- **Left Click**: Shoot
- **ESC**: Menu/Quit

_Mob King Role_:

- **JKIL**: Movement (reserved for local multiplayer compatibility)
- **]**: Acceleration boost (2x speed for 1 second, 3-second cooldown)
- **ESC**: Menu/Quit

---

## 📝 Success Criteria

**Phase 1 Complete**: Game builds and runs with "Bloodstrike" name ✅
**Phase 2 Complete**: Game window is larger and properly scaled ✅
**Phase 3 Complete**: Player can aim with mouse and shoot projectiles with visual aiming line ✅
**Phase 4 Complete**:

- Main menu with Single/Multiplayer options
- P2P networking connects two players
- Asymmetric Player vs Mob King gameplay works
- 2-minute survival timer with dynamic difficulty
- Mob King respawn system (8 seconds)
- Both victory conditions work correctly

Let's get started! 🔫

---

## 🗓️ Phase 4 Development Timeline (10-14 days)

### Week 1: Foundation & Menu System

**Day 1-2: Menu System Enhancement**

- Add main menu with Single/Multiplayer buttons
- Create Host/Join game interface
- Add lobby screen with connection status
- Add dice roll animation and logic
- Create role selection interface

**Day 3-4: Networking Foundation**

- Integrate SDL_net dependency
- Create NetworkSystem class
- Implement basic TCP Host-Client connection
- Test connection establishment and lobby flow

**Day 5-7: Lobby System**

- Implement dice roll synchronization
- Create role selection logic
- Add lobby state management
- Test complete lobby flow (connect → dice → roles → start)

### Week 2: Gameplay & Polish

**Day 8-9: Mob King System**

- Implement human-controlled mob king
- Add acceleration boost mechanics (P key)
- Create 8-second respawn system
- Test mob king controls

**Day 10-11: Game Rules Implementation**

- Add 2-minute survival timer
- Implement dynamic difficulty scaling
- Handle victory/defeat conditions
- Test complete multiplayer match

**Day 12-14: Polish & Testing**

- Add network lag compensation
- Handle disconnections gracefully
- Add multiplayer UI elements
- Extensive testing and bug fixes

---

## 🎯 Development Questions & Decisions

**✅ Technical Decisions Made:**

1. **Control Scheme**: ✅ JKIL for Mob King (future local multiplayer), ] for acceleration

2. **Network Architecture**: ✅ Host-Client model for simpler authority management

3. **Collision Authority**: ✅ Hybrid approach:

   - Host: Mob spawning, mob-player collisions, victory conditions
   - Client: Own projectile collisions (immediate feedback)
   - Sync: Results communicated between clients

4. **State Synchronization**: ✅ 60 FPS for smooth gameplay

5. **Dynamic Difficulty**: ✅ Linear increase over 2 minutes

6. **Mob King Abilities**: ✅ Start with acceleration only (] key)
   - Future possibilities: Dash attack, temporary invincibility, spawn extra mobs

**Next Steps:**

1. **Immediate**: Start with menu system enhancement
2. **SDL_net Integration**: Add dependency to CMakeLists.txt
3. **Architecture**: Create base networking classes
4. **Testing Strategy**: How to test P2P locally?

Would you like to start with any specific part, or shall we begin with the menu system enhancement? 🚀
