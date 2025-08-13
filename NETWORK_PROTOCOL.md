# Bloodstrike 2D - Network Messages

## Network Message Types

| Message Type         | Direction     | Purpose                         |
| -------------------- | ------------- | ------------------------------- |
| `CONNECTION_REQUEST` | Client → Host | Initial connection attempt      |
| `CONNECTION_ACCEPT`  | Host → Client | Connection approved             |
| `CONNECTION_REJECT`  | Host → Client | Connection denied               |
| `DISCONNECT`         | Bidirectional | Clean disconnection             |
| `PING`               | Bidirectional | Heartbeat check (every 2s)      |
| `PONG`               | Bidirectional | Heartbeat response              |
| `PLAYER_READY`       | Bidirectional | Player ready state in lobby     |
| `LOBBY_STATUS`       | Host → Client | Lobby state updates             |
| `GAME_START`         | Host → Client | Start gameplay                  |
| `PLAYER_POSITION`    | Client → Host | Player movement.                |
| `MOB_KING_POSITION`  | Client → Host | Mob King controls               |
| `GAME_STATE_UPDATE`  | Host → Client | Score, health, time (5 FPS)     |
| `MOB_SPAWN`          | Host → Client | Create mob entities             |
| `PROJECTILE_CREATE`  | Bidirectional | Create projectiles              |
| `ENTITY_REMOVE`      | Host → Client | Remove entities after collision |
| `PROJECTILE_HIT`     | Host → Client | Damage notifications            |
| `GAME_OVER`          | Host → Client | End game state                  |
| `MOB_KING_DEATH`     | Host → Client | Boss defeated                   |

---

_Host is authoritative for all game logic. Client sends input and renders._

- _Question 1: Host is handling all the game logic (collision(Entity Remove), Game State change, Mob Spawn initial State, Mob projectile Initial State, Player Movement); Client is only handling itself (Mob king movemeent, and Mob King projectile). Is it a good design? Do we need to move some of the logic to CLient to reduce the bandwith being used?_
  <br><br>
- _Question 2: In every frame(60FPS), the data(Position, Projectile creation, mob creation) is sent between Host and Client, is it too frequent? What's the best practice?_
  <br><br>
- _Question 3: Right now the game is host on Localhost, How can I put it on internet?_
  <br><br>
- _Question 4: For tomorrow, I need a demo video and just bring my game to campus right?_
