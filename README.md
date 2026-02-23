# Stack-N-Climb

A third-person climbing platformer built in **Unreal Engine 5** (C++ / Blueprint hybrid) based on the Stack-O-Bot startup fab.
The player shoots falling disks to freeze them into platforms, then uses those platforms to climb to a high-altitude win zone.

---

## Gameplay Overview

Disks rain down from above in a grid pattern. The player must aim and shoot them to freeze them mid-air, then jump onto the frozen platforms to gain height. Reach the target altitude to win.

---

## Controls

| Input                         | Action                                                        |
| ----------------------------- | ------------------------------------------------------------- |
| `W A S D`                   | Move                                                          |
| `Space`                     | Jump                                                          |
| `Space` (hold, airborne)    | Jetpack hover                                                 |
| `Right Mouse Button` (hold) | Aim Down Sights (ADS) — shifts camera right, shows crosshair |
| `Left Mouse Button`         | Fire laser (only meaningful while aiming)                     |

---

## Features

### Falling Disk System

- Disks spawn in a configurable **N × M grid** above the player and fall at randomized speeds.
- Each grid cell always contains exactly one disk — if a disk hits the ground or drifts too far below the player it is immediately **respawned** at its cell position above the player's current height.
- Disks have three states:

| State                  | Color   | Description                                           |
| ---------------------- | ------- | ----------------------------------------------------- |
| **Falling**      | Default | Moving downward at its assigned speed                 |
| **Frozen Red**   | Red     | The single disk above the player that has been frozen |
| **Frozen Green** | Green   | Any disk at or below the player that has been frozen  |

### Shoot-to-Freeze Mechanics

- **Red disk rule** — only one disk above the player can be frozen at a time. Shooting a second disk above you automatically unfreezes the previous red disk, which resumes falling at its pre-freeze speed.
- **Green disk rule** — any number of disks at or below the player can be frozen green simultaneously. A frozen green disk instantly unfreezes (resumes falling) the moment the player falls below it.
- **Red → Green promotion** — as the player climbs above a frozen red disk it automatically becomes green, freeing the "one above" slot.
- Shooting a disk that is already below the player freezes it green directly.

### Aim Down Sights (ADS) + Laser

- Holding **RMB** smoothly lerps the spring arm's `SocketOffset` from its default position to a right-side ADS position, giving a classic third-person shooter look.
- Releasing **RMB** smoothly lerps the camera back.
- A **crosshair widget** appears while ADS is active and is hidden otherwise.
- **LMB** fires a **hitscan line trace** from the camera through the crosshair center. Whatever the crosshair is aimed at is what gets hit — accuracy is camera-based, not character-based.
- The laser is visualized as a **Niagara dynamic beam** that originates from the character's chest and extends to the hit point.

### Knockback

- A **falling** disk that physically collides with the player while they are **airborne** launches the player downward and away from the disk center, potentially knocking them off their platform.
- Frozen disks do not knock the player back.

### HUD

Three text lines are displayed in the top-left corner at all times:

| Label              | Content                                                                 |
| ------------------ | ----------------------------------------------------------------------- |
| `Height`         | Player's current height above their starting position                   |
| `Best`           | Highest height reached this session (resets to 0 each new play session) |
| `Reach X to win` | The target altitude the player must reach                               |

### Win Condition

- When the player's altitude exceeds the configured **Win Z** threshold, a win screen appears after a short delay (giving the player a moment to land).
- The win screen offers a **Play Infinite Mode** button which dismisses the screen, disables the win check, and lets the player keep climbing indefinitely.

---

## Architecture

```
ADiskSpawner          – placed once in the level; owns the disk grid
    └── TArray<AFallingDisk*>   – one per grid cell

AFallingDisk          – moves downward each tick; responds to freeze/unfreeze calls
    └── EDiskState    – Falling | FrozenRed | FrozenGreen

UShootingComponent    – UActorComponent attached to BP_Bot
    ├── ADS camera shift  (lerps Spring Arm SocketOffset each tick)
    ├── Crosshair widget  (show/hide on ADS toggle)
    ├── Line-trace fire   (camera-accurate hitscan)
    ├── Niagara beam VFX  (dynamic beam from chest to hit point)
    └── calls ADiskSpawner::NotifyDiskHit(disk, player)

UPlayerHUDWidget      – UUserWidget subclass; updated every tick by ADiskSpawner
    ├── Text_CurrentZ  – current height above spawn
    ├── Text_HighestZ  – session-best height
    └── Text_WinGoal   – target altitude
```

Input is wired entirely in **BP_Bot**'s Event Graph using Enhanced Input:

```
RMB Pressed/Released  →  ShootingComponent::StartADS / StopADS
LMB Pressed           →  ShootingComponent::Fire
```

---

## Project Structure

```
Source/StackOBot/
    FallingDisk.h / .cpp        – Disk actor (state machine, knockback, materials)
    DiskSpawner.h / .cpp        – Grid spawner, freeze rules, win detection, HUD
    ShootingComponent.h / .cpp  – ADS + laser fire + Niagara beam
    PlayerHUDWidget.h / .cpp    – HUD widget C++ base class

Content/StackOBot/Blueprints/
    BP_FallingDisk              – FallingDisk child; assigns mesh + 3 materials
    BP_DiskSpawner              – DiskSpawner child; assigns disk class + HUD/win classes

Content/StackOBot/UI/
    WBP_Crosshair               – Centered crosshair image shown during ADS
    WBP_PlayerHUD               – HUD (parent class: PlayerHUDWidget)
    UI_WinScreen                – Win screen with "Play Infinite Mode" button

Content/StackOBot/Input/
    IMC_Default                 – Input Mapping Context
    IA_ADS                      – Digital bool action (Right Mouse Button)
    IA_Fire                     – Digital bool action (Left Mouse Button)
```

---

## Build & Run

### Prerequisites

- **Unreal Engine 5.6** (or the version matching the project's `.uproject`)
- **Visual Studio 2022** with the *Game development with C++* workload and the *Unreal Engine* optional component

### Steps

1. Right-click `StackOBot.uproject` → **Generate Visual Studio project files**.
2. Open `StackOBot.sln` in Visual Studio 2022.
3. Set the solution configuration to **Development Editor** and platform to **Win64**.
4. **Build → Build Solution** (`Ctrl+Shift+B`).
5. Launch the Unreal Editor from Visual Studio (**Debug → Start Without Debugging**), or double-click `StackOBot.uproject`.
6. Press **Play** in the editor toolbar to run the game.

### Packaging (Shipping build)

1. **Edit → Project Settings → Packaging**: set **Build Configuration** to *Shipping*.
2. Ensure only `LVL_New` (and `LVL_MainMenu` if applicable) are in the map list under *Maps & Modes*.
3. **Platforms → Windows → Package Project** and choose an output directory.

---

## Tunable Parameters (set in BP_DiskSpawner Class Defaults)

| Property                 | Default  | Description                                               |
| ------------------------ | -------- | --------------------------------------------------------- |
| `Grid Columns`         | 4        | Number of grid columns                                    |
| `Grid Rows`            | 4        | Number of grid rows                                       |
| `Cell Size`            | 350 UU   | Width/depth of each grid cell (must exceed disk diameter) |
| `Spawn Height Offset`  | 3000 UU  | How far above the player new/respawned disks appear       |
| `Ground Z`             | −123 UU | World-Z floor below which disks are always respawned      |
| `Respawn Below Offset` | 400 UU   | Disks this far below the player are also respawned        |
| `Min Speed`            | 150 UU/s | Minimum random fall speed                                 |
| `Max Speed`            | 500 UU/s | Maximum random fall speed                                 |
| `Win Z`                | 2000 UU  | Altitude the player must reach to trigger the win screen  |
