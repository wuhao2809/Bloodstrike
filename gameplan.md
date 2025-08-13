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

## 📅 Current Development Status

### ✅ **Phase 1: Project Rebranding** (COMPLETED)

- [x] Updated project name from "Dodge the Creeps" to "Bloodstrike"
- [x] All build system and references updated

### ✅ **Phase 2: Larger Window Size** (COMPLETED)

- [x] Window sized at 1280x720 for optimal gameplay
- [x] All systems scale properly with new dimensions

### ✅ **Phase 3: Aiming & Shooting System** (COMPLETED)

- [x] Mouse-based aiming with visual feedback
- [x] Projectile system with collision detection
- [x] Weapon system with fire rate and ammo management
- [x] Visual aids (crosshair, dotted aiming lines)

### ✅ **Phase 4a: Menu System** (COMPLETED)

- [x] Complete menu navigation (Single Player / Dual Player / Multiplayer)
- [x] JSON-configurable menu system
- [x] Clean state management and UI separation

### ✅ **Phase 4b: Dual Player Local Mode** (COMPLETED)

**Status**: 🎮 **FULLY FUNCTIONAL** - Unified with multiplayer rules!

**Complete Implementation:**

- [x] **Game Modes**: Single Player, Dual Player Local, Multiplayer Online (unified format)
- ✅ Unified Rules\*\*: Both dual and multiplayer use 90-second countdown format
- ✅ **Immediate Mob King**: Spawns at game start in both dual/multiplayer modes
- ✅ **Dynamic Difficulty**: Progressive spawn rate (1.0s→0.2s) and speed (1.0x→2.0x)
- ✅ **Last 15s Shooting**: Mobs shoot every 3 seconds in final phase
- [x] **Human Controls**: Player 1 (WASD+mouse) vs Player 2 (IJKL+P directional shooting)
- [x] **Mob King System**: 160x160px flying sprite, 150 health, visible at (1000, 300)
- [x] **Combat Balance**: Only Mob King shoots in dual mode, regular mobs simplified
- [x] **Spawn Logic**: Mob King appears Level 2+, regular mobs spawn at 2x interval
- [x] **Rendering**: Full mobKing entity support with sprite flipping
- [x] **Input Separation**: Completely independent control schemes

**Game Experience:**

```
Player 1 (Grey Survivor):    Player 2 (Large Flying Mob King):
- WASD movement              - IJKL movement
- Mouse aim & click          - P directional shooting
- Goal: Complete levels      - Goal: Eliminate Player 1
- 30 ammo, reloads          - Unlimited ammo, 150 health
```

---

## 🚀 **Next Phase: Online Multiplayer**

### **Phase 5: Network Implementation** (NEXT PRIORITY - 3-5 days)

**Goal**: Transform the working local dual player system into online multiplayer

**Foundation Advantages:**

- ✅ **Proven Gameplay**: Local human vs human balance tested and working
- ✅ **Input Systems**: IJKL+P controls already implemented and functional
- ✅ **Game Logic**: Mob King spawning, combat, and victory conditions complete
- ✅ **Visual Feedback**: Large, visible Mob King with proper rendering
- ✅ **Menu Infrastructure**: Host/Join interface already in place

**Implementation Plan:**

#### **Phase 5a: Basic Networking** (1-2 days)

- ✅ Implement SDL_net TCP connections using existing NetworkSystem foundation
- ✅ Create Host/Client connection handshake
- ✅ Test basic message passing between players
- ✅ Implement lobby connection flow (Host waits, Client joins by IP)
- ✅ Automatic lobby-to-game transition when both players connected

#### **Phase 5b: Input Synchronization** (1-2 days)

- ✅ Extend current IJKL+P input system to send over network
- ✅ Synchronize Player 1 WASD+mouse inputs across clients
- ✅ Host controls Player (WASD+mouse), Client controls Mob King (IJKL+P)
- ✅ Input message structures and network transmission implemented
- ✅ Fixed lobby-to-game transition (clients now properly connect)
- ✅ Simplified multiplayer mode: 90s single level with dynamic difficulty
- ✅ Mob King spawns immediately
- ✅ Dynamic mob spawn rate: starts at 1.0s, decreases to 0.2s over 90 seconds
- ✅ Dynamic mob speed: starts at 1.0x, increases to 2.0x over 90 seconds
- ✅ Last 15s: mobs shoot every 3 seconds (unified for dual and multiplayer)
- ✅ Unified game rules between local dual player and online multiplayer
- 🚧 Test responsive dual player controls over network

#### **Phase 5c: Shooting & Projectile Synchronization** (1-2 days)

- 🚧 Synchronize mouse click shooting (Player) over network
- 🚧 Synchronize P key shooting (Mob King) over network
- 🚧 Handle projectile creation, movement, and collision across clients
- 🚧 Test complete 90s dynamic difficulty gameplay over network
- 🚧 Implement victory/defeat condition synchronization

**Technical Architecture:**

```cpp
// Extend existing systems for network support
class NetworkSystem : public System {
    void sendPlayerInput(PlayerInput input);
    void receivePlayerInput(uint32_t playerID, PlayerInput input);
    void syncGameState(GameState state);
    void handleConnection();
};

struct NetworkMessage {
    MessageType type;           // INPUT, GAME_STATE, PROJECTILE, etc.
    uint32_t playerID;         // Host=1, Client=2
    uint32_t frameNumber;      // For input synchronization
    union {
        PlayerInput input;      // WASD+mouse or IJKL+P
        GameStateData state;    // Mob positions, health, score
        ProjectileData projectile; // Bullet creation/collision
    } data;
};
```

---

## 🎮 **Current Game Features**

### **Three Complete Game Modes:**

1. **✅ Single Player**: Classic 4-level progression

   - Mouse aiming + WASD movement
   - Mob waves increase in difficulty
   - Level 4: Mobs can shoot back
   - Victory: Complete all levels

2. **✅ Dual Player (Local)**: Human vs Human couch co-op

   - Player 1: Grey survivor (WASD + mouse)
   - Player 2: Large flying Mob King (IJKL + P)
   - Asymmetric gameplay with balanced mechanics
   - Victory: P1 completes levels OR P2 eliminates P1

3. **🚧 Multiplayer (Online)**: Network version of dual player
   - Same mechanics as local dual player
   - Host/Client networking architecture
   - Real-time input synchronization

### **Technical Architecture Completed:**

**Core Systems:**

- ✅ **ECS Framework**: Complete entity-component-system architecture
- ✅ **Input System**: Dual player controls (WASD+mouse vs IJKL+P)
- ✅ **Weapon System**: Smart shooting (directional vs auto-aim)
- ✅ **Render System**: Sprite animation, texture management, visual effects
- ✅ **Audio System**: SDL2_mixer integration ready
- ✅ **Menu System**: Complete navigation with JSON configuration

**Game Logic:**

- ✅ **Mob Spawning**: Smart spawn rates, multiple mob types, Mob King system
- ✅ **Collision System**: Player-mob, projectile-mob, boundary detection
- ✅ **Movement System**: Directional movement with sprite flipping
- ✅ **Animation System**: Frame-based sprite animation
- ✅ **Game Manager**: Multi-mode state management, level progression

**Visual Polish:**

- ✅ **Aiming System**: Dotted line visualization, crosshair, range indication
- ✅ **Projectile Rendering**: Color-coded bullets, collision effects
- ✅ **UI System**: Score display, level indicators, game messages
- ✅ **Large Mob King**: 160x160px flying sprite for high visibility

---

## 🛠️ **How to Play (Current Build)**

### **Starting the Game:**

```bash
cd "/Users/haowu/Desktop/code/projects/Bloodstrike 2D"
./build/Bloodstrike
```

### **Menu Navigation:**

- **Arrow Keys/WS**: Navigate menu options
- **Enter/Space**: Select option
- **ESC**: Back/Quit

### **Single Player Mode:**

- **WASD**: Move grey player
- **Mouse**: Aim direction
- **Left Click**: Shoot
- **Goal**: Complete 4 levels, survive increasing difficulty

### **Dual Player Mode:**

- **Player 1 (Grey)**: WASD movement + mouse aim/shoot
- **Player 2 (Flying Mob King)**: IJKL movement + P to shoot
- **Mob King**: Spawns at Level 2 with 150 health and high visibility
- **Strategy**: P1 must complete levels while avoiding P2's attacks

- ✅ Add main menu with Single Player / Dual Player / Multiplayer options
- ✅ Menu navigation with W/S and arrow keys
- ✅ Proper menu centering and state management
- ✅ Clean separation between menu and game UI
- ✅ JSON-configurable menu system with entities.json
- ✅ Host/Join game interface for multiplayer preparation
- ✅ Lobby screen with "waiting for player" status
- ✅ NetworkSystem integration (foundation ready)

**✅ Phase 4b-alternate: Dual Player Local Mode** (COMPLETED)

---

## 🎯 **Development Summary**

### **✅ Completed Achievements:**

- **Perfect Local Couch Co-op**: Human vs Human dual player mode fully functional
- **Large Visual Mob King**: 160x160px flying sprite, highly visible and responsive
- **Balanced Combat System**: Smart weapon mechanics, directional shooting
- **Complete Input Separation**: WASD+mouse vs IJKL+P controls
- **Robust Menu System**: JSON-configurable, clean navigation
- **ECS Architecture**: Scalable, maintainable codebase ready for network expansion

### **🚀 Next Milestone:**

**Phase 5: Network Implementation** - Transform local dual player into online multiplayer using existing SDL_net foundation and proven gameplay mechanics.

### **🎮 Ready to Play Now:**

```bash
cd "/Users/haowu/Desktop/code/projects/Bloodstrike 2D"
./build/Bloodstrike
# Select "Dual Player" for local couch co-op experience!
```

**Current Status**: 🟢 **STABLE BUILD** - All core features complete and tested! ✅
