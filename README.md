# HEIST — Roguelike Bank Robbery

A top-down roguelike where you rob procedurally generated banks.
Every run earns meta XP that permanently unlocks new weapons, items,
abilities, level themes, and base stat upgrades.

## Build Requirements

- Linux (Ubuntu/Debian recommended)
- GCC with C++17 support
- OpenGL 3.3
- GLFW3, GLEW, Mesa GL

### Install dependencies (Ubuntu/Debian):
```
sudo apt-get install libglfw3-dev libglew-dev libgl1-mesa-dev freeglut3-dev
```

### Build:
```
cd heist
make
./heist
```

## Controls

| Key | Action |
|-----|--------|
| WASD / Arrow Keys | Move |
| Mouse | Aim |
| Left Click | Shoot |
| C / Left Ctrl | Crouch (quieter, harder to detect) |
| G (hold near vault) | Crack the vault |
| 1 / 2 / 3 / 4 | Use item slot |
| Q / E / R / F | Use ability slot |
| ESC | Pause / Back |
| Enter | Confirm |

## Gameplay

### Objective
Enter the bank, crack the vault (hold G), collect loot, and reach the EXIT.

### Stealth vs Loud
- Guards have vision cones — stay outside them or crouch
- Security cameras sweep back and forth — time your crossings
- Silenced weapons don't alert guards, loud weapons do
- Getting detected raises the alarm and spawns more aggressive responses

### Alarm Levels
1. **Suspicious** — guard investigates
2. **Alert** — guards chase and shoot on sight
3. **Lockdown** — all guards alerted, cameras triggered

### Meta Progression
Every run (win or lose) earns meta XP. Spend XP in the **Skill Trees** to:
- Unlock new **Weapons** (SMG, Shotgun, Sniper, EMP Rifle, Railgun)
- Unlock new **Items** (EMP Grenade, Smoke Bomb, Cloak Suit, Hacking Kit...)
- Unlock new **Abilities** (Sprint, Wall Hack, Time Slow, Adrenaline...)
- Unlock new **Level Themes** (Casino, Museum, Federal Reserve, Swiss Vault...)
- Upgrade **Base Stats** (Max HP, Speed, Stealth, Luck, Cash Multiplier)

### Scoring
- Cash collected (scales with theme difficulty)
- Stealth bonus (never detected = +Ghost bonus)
- Vault bonus (crack the vault for big reward)
- Time bonus (faster = better score)

### Themes (unlock via skill tree)
| Theme | Guards | Cameras | Loot Mult | Unlock |
|-------|--------|---------|-----------|--------|
| Corner Bank | 3 | 1 | 1.0x | Start |
| Casino | 5 | 3 | 1.8x | Level 4 |
| Art Museum | 4 | 4 | 2.2x | Level 7 |
| Federal Reserve | 8 | 6 | 3.5x | Level 11 |
| Swiss Vault | 6 | 8 | 4.0x | Level 16 |
| Offshore Fortress | 12 | 10 | 6.0x | Level 22 |

## Save File
Progress is saved to `heist_save.dat` in the working directory.
Delete it to reset all meta progression.
