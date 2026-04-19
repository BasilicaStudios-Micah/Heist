#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <functional>
#include <memory>
#include <cmath>
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <queue>

// ─── Constants ───────────────────────────────────────────────────────────────
inline int SCREEN_W = 1280;
inline int SCREEN_H = 720;
static const int TILE_SIZE  = 32;
static const int MAP_W      = 80;
static const int MAP_H      = 55;
static const int MAX_FLOORS = 3;

// ─── Tile types ──────────────────────────────────────────────────────────────
enum TileType {
    TILE_EMPTY=0, TILE_FLOOR, TILE_WALL,
    TILE_VAULT_DOOR, TILE_LOOT, TILE_EXIT,
    TILE_STAIRS_UP, TILE_STAIRS_DOWN,
    TILE_ELEVATOR
};

// ─── Room types ───────────────────────────────────────────────────────────────
enum RoomType {
    ROOM_LOBBY, ROOM_BASIC, ROOM_HALLWAY, ROOM_VAULT,
    ROOM_SERVER, ROOM_BARRACKS, ROOM_STORAGE,
    ROOM_STAIRS, ROOM_ELEVATOR_SHAFT
};

// ─── Math ────────────────────────────────────────────────────────────────────
struct Vec2 { float x=0,y=0; };
struct Vec2i { int x=0,y=0; };
struct Rect { float x,y,w,h; };
inline float v2len(Vec2 v){ return sqrtf(v.x*v.x+v.y*v.y); }
inline Vec2  v2norm(Vec2 v){ float l=v2len(v); return l>0?Vec2{v.x/l,v.y/l}:Vec2{0,0}; }
inline Vec2  v2sub(Vec2 a,Vec2 b){ return {a.x-b.x,a.y-b.y}; }
inline Vec2  v2add(Vec2 a,Vec2 b){ return {a.x+b.x,a.y+b.y}; }
inline Vec2  v2scale(Vec2 v,float s){ return {v.x*s,v.y*s}; }
inline float v2dot(Vec2 a,Vec2 b){ return a.x*b.x+a.y*b.y; }
inline bool  rectsOverlap(Rect a,Rect b){
    return a.x<b.x+b.w && a.x+a.w>b.x && a.y<b.y+b.h && a.y+a.h>b.y;
}

// ─── Colors ──────────────────────────────────────────────────────────────────
struct Color { float r,g,b,a; };
static const Color C_WHITE  = {1,1,1,1};
static const Color C_BLACK  = {0,0,0,1};
static const Color C_RED    = {0.9f,0.2f,0.2f,1};
static const Color C_GREEN  = {0.2f,0.9f,0.3f,1};
static const Color C_BLUE   = {0.2f,0.4f,0.9f,1};
static const Color C_YELLOW = {0.95f,0.85f,0.2f,1};
static const Color C_ORANGE = {1.0f,0.55f,0.1f,1};
static const Color C_PURPLE = {0.7f,0.2f,0.9f,1};
static const Color C_CYAN   = {0.2f,0.85f,0.9f,1};
static const Color C_GRAY   = {0.5f,0.5f,0.5f,1};
static const Color C_DGRAY  = {0.25f,0.25f,0.25f,1};
static const Color C_LGRAY  = {0.75f,0.75f,0.75f,1};
static const Color C_AMBER  = {0.95f,0.65f,0.1f,1};
static const Color C_TEAL   = {0.1f,0.75f,0.65f,1};
static const Color C_PINK   = {0.95f,0.4f,0.7f,1};
static const Color C_NAVY   = {0.1f,0.15f,0.5f,1};
static const Color C_MAROON = {0.5f,0.05f,0.05f,1};
static const Color C_GOLD   = {1.0f,0.84f,0.0f,1};

// ─── Weapon definitions ──────────────────────────────────────────────────────
struct WeaponDef {
    std::string name;
    float damage, range, fireRate, bulletSpeed;
    bool silenced;
    int unlockLevel;
    Color color;
    int clipSize;        // ammo per reload
    float reloadTime;
    bool isAutomatic;    // hold to fire
    std::string desc;
};
static const std::vector<WeaponDef> WEAPON_DEFS = {
    // name                dmg  range  rate   spd    sil  unlk  color                     clip  rel   auto  desc
    {"Silenced Pistol",    25,  400,   0.5f,  600,   true, 0,   {0.7f,0.7f,0.8f,1},       12,  1.2f, false,"Reliable starter. Silent."},
    {"SMG",                15,  300,   0.12f, 700,   false,3,   {0.6f,0.8f,0.6f,1},       30,  2.0f, true, "Fast fire, high noise."},
    {"Shotgun",            60,  180,   0.9f,  500,   false,5,   C_ORANGE,                  8,   2.5f, false,"Devastating close range."},
    {"Sniper Rifle",       90,  800,   1.5f,  1200,  true, 8,   {0.5f,0.8f,1.0f,1},       5,   2.8f, false,"Long range precision."},
    {"EMP Rifle",          40,  500,   1.2f,  400,   true, 12,  C_PURPLE,                  6,   2.0f, false,"Disables electronics."},
    {"Railgun",            150, 1000,  2.0f,  2000,  true, 18,  C_CYAN,                    1,   3.0f, false,"Penetrates walls."},
    {"Revolver",           55,  450,   0.7f,  750,   false,4,   {0.8f,0.6f,0.3f,1},       6,   2.2f, false,"Hard-hitting. Loud."},
    {"Crossbow",           70,  500,   1.0f,  550,   true, 7,   {0.6f,0.4f,0.2f,1},       4,   2.0f, false,"Silent bolt. Slow."},
    {"Machine Pistol",     12,  250,   0.08f, 650,   false,6,   {0.5f,0.7f,0.5f,1},       20,  1.5f, true, "Compact auto. Noisy."},
    {"Grenade Launcher",   120, 300,   1.8f,  350,   false,14,  C_ORANGE,                  3,   3.5f, false,"Explosive AOE."},
    {"Laser Pistol",       35,  600,   0.3f,  3000,  true, 16,  {0.9f,0.1f,0.5f,1},       20,  1.5f, false,"Instant hit. Silenced."},
    {"PDW",                20,  350,   0.15f, 680,   true, 10,  {0.4f,0.6f,0.8f,1},       25,  2.2f, true, "Silenced auto. Compact."},
    {"Tranq Rifle",        15,  550,   1.0f,  450,   true, 9,   {0.5f,0.8f,0.6f,1},       8,   2.0f, false,"Non-lethal. Stuns guards."},
    {"Flare Gun",          80,  200,   2.5f,  300,   false,11,  {1.0f,0.4f,0.1f,1},       1,   3.0f, false,"Blind + burn. One shot."},
};

// ─── Item definitions ────────────────────────────────────────────────────────
struct ItemDef {
    std::string name, desc;
    int unlockLevel;
    Color color;
    int defaultCount; // starting count when selected
};
static const std::vector<ItemDef> ITEM_DEFS = {
    {"Lockpick",     "Silently open locked doors",        0,  {0.8f,0.7f,0.3f,1},  2},
    {"EMP Grenade",  "Disables cameras/alarms in radius", 3,  {0.3f,0.9f,0.8f,1},  2},
    {"Smoke Bomb",   "Blocks guard vision cone",          5,  {0.6f,0.6f,0.6f,1},  2},
    {"Decoy",        "Distracts guards 8 seconds",        8,  {0.9f,0.9f,0.2f,1},  2},
    {"Medkit",       "Restore 75 HP",                     0,  {0.9f,0.2f,0.2f,1},  2},
    {"Cloak Suit",   "Invisible for 5 seconds",           15, {0.2f,0.4f,0.9f,0.8f},1},
    {"Hacking Kit",  "Bypass security terminals",         10, {0.4f,0.9f,0.4f,1},  2},
    {"Rope",         "Access roof entry points",          6,  {0.7f,0.5f,0.3f,1},  2},
    {"Flashbang",    "Stuns + blinds guards 4s",          7,  C_YELLOW,             2},
    {"Shaped Charge","Blows open vault instantly",        12, C_ORANGE,             1},
    {"Gas Canister", "Toxic cloud slows guards",          9,  C_GREEN,              2},
    {"Disguise Kit", "Guards ignore you for 15s",         13, {0.8f,0.7f,0.9f,1},  1},
    {"Jammer",       "Disables radio calls 30s",          11, {0.3f,0.6f,0.9f,1},  1},
    {"C4",           "Remote-detonated explosive",        17, C_RED,                1},
    {"Night Vision", "See in darkness for 20s",           8,  {0.2f,0.8f,0.2f,1},  1},
};

// ─── Ability definitions ─────────────────────────────────────────────────────
struct AbilityDef {
    std::string name, desc;
    float cooldown;
    int unlockLevel;
    Color color;
};
static const std::vector<AbilityDef> ABILITY_DEFS = {
    {"Dodge Roll",    "Quick roll, brief i-frames",         1.5f, 0,  C_CYAN},
    {"Sprint",        "2x speed for 3 seconds",             4.0f, 2,  C_YELLOW},
    {"Wall Hack",     "See guards through walls 5s",        8.0f, 6,  {0.9f,0.3f,0.9f,1}},
    {"Time Slow",     "50% time scale for 3s",              12.0f,14, {0.3f,0.8f,1.0f,1}},
    {"Adrenaline",    "Ignore damage for 2s",               15.0f,10, C_RED},
    {"Ghost Step",    "Silent movement 8s",                 10.0f,4,  {0.7f,0.7f,1.0f,1}},
    {"Flashstep",     "Teleport 120px in facing dir",       6.0f, 8,  {0.5f,0.9f,1.0f,1}},
    {"Overcharge",    "+75% damage for 5s",                 14.0f,12, C_ORANGE},
    {"Vanish",        "Drop aggro, guards lose sight",      20.0f,16, {0.3f,0.3f,0.7f,1}},
    {"Meditate",      "Regen 40HP over 4s, can't move",    18.0f,13, C_GREEN},
    {"Berserk",       "2x dmg + speed, -50% stealth, 6s",  16.0f,15, C_MAROON},
    {"Trap",          "Place bear trap on floor",           8.0f, 7,  {0.6f,0.4f,0.2f,1}},
};

// ─── Level/Theme definitions ─────────────────────────────────────────────────
struct ThemeDef {
    std::string name;
    Color floorColor, wallColor, accentColor;
    int guardCount, camCount;
    float lootMultiplier, difficultyMult;
    int unlockLevel;
    int targetRooms;
    int extraGuardWave;
    int vaultRooms;       // number of vault rooms
    int serverRooms;      // number of server rooms
    int storageRooms;     // number of storage rooms
    bool hasBarracks;     // barracks room?
    bool hasStairs;       // multi-floor stairs?
    bool hasElevator;     // elevator (fortress only)
    int  maxFloors;       // 1, 2, or 3
    std::string desc;
};
static const std::vector<ThemeDef> THEME_DEFS = {
    // Corner Bank
    {"Corner Bank",
     {0.35f,0.3f,0.25f,1},{0.55f,0.5f,0.45f,1},C_YELLOW,
     3,1, 1.0f,1.0f, 0,
     8,3, 1,1,2, false,false,false,1,
     "A small community bank. Light security."},
    // Casino
    {"Casino",
     {0.2f,0.15f,0.1f,1},{0.4f,0.1f,0.1f,1},C_ORANGE,
     6,3, 1.8f,1.4f, 4,
     11,4, 1,1,2, false,false,false,1,
     "High rollers, high risk. Many cameras."},
    // Art Museum
    {"Art Museum",
     {0.4f,0.38f,0.35f,1},{0.6f,0.58f,0.55f,1},C_WHITE,
     8,5, 2.2f,1.6f, 7,
     13,5, 1,1,3, false,false,false,1,
     "Priceless art. Laser grids everywhere."},
    // Federal Reserve
    {"Federal Reserve",
     {0.2f,0.25f,0.2f,1},{0.3f,0.35f,0.3f,1},C_GREEN,
     12,7, 3.5f,2.2f, 11,
     15,6, 2,2,2, true,true,false,2,
     "Government money. Two floors."},
    // Swiss Vault
    {"Swiss Vault",
     {0.2f,0.2f,0.3f,1},{0.3f,0.3f,0.45f,1},C_BLUE,
     14,10,4.0f,2.5f, 16,
     17,7, 2,2,4, true,true,false,2,
     "Precision Swiss security. Multi-vault."},
    // Offshore Fortress
    {"Offshore Fortress",
     {0.15f,0.2f,0.15f,1},{0.1f,0.15f,0.1f,1},C_CYAN,
     20,13,6.0f,3.5f, 22,
     20,8, 3,3,2, true,false,true,3,
     "Military fortress. Three floors. Elevator."},
};

// ─── Skill tree node ──────────────────────────────────────────────────────────
struct SkillNode {
    std::string name, desc;
    int cost;
    int parentIdx;
    bool unlocked;
    std::string statKey;
    float statVal;
    // Visual position in skill tree (normalized 0..1)
    float treeX, treeY;
};

// ─── Meta save data ───────────────────────────────────────────────────────────
struct MetaData {
    int metaLevel   = 0;
    int metaXP      = 0;
    int totalRuns   = 0;
    int totalCash   = 0;
    int bestScore   = 0;

    std::vector<int> unlockedWeapons   = {0};
    std::vector<int> unlockedItems     = {0,4};
    std::vector<int> unlockedAbilities = {0};
    std::vector<int> unlockedThemes    = {0};

    float bonusMaxHP       = 0;
    float bonusSpeed       = 0;
    float bonusStealth     = 0;
    float bonusLuck        = 0;
    float bonusCashMult    = 1.0f;
    float bonusDamage      = 0;      // flat damage bonus
    float bonusReload      = 0;      // % reload speed bonus
    float bonusClipSize    = 0;      // % extra clip
    float bonusCooldown    = 0;      // % cooldown reduction
    float bonusXP          = 0;      // % extra XP
    int   statPointsSpent  = 0;
};

// ─── Bullet ──────────────────────────────────────────────────────────────────
struct Bullet {
    Vec2  pos, vel;
    float damage, range, traveled;
    bool  alive, fromPlayer;
    Color color;
    bool  penetrating; // railgun pierces walls
    bool  explosive;   // grenade launcher
    bool  tranq;       // tranq rifle
};

// ─── Sprite IDs ──────────────────────────────────────────────────────────────
enum SpriteID {
    SPR_TILE_FLOOR = 0,
    SPR_TILE_WALL,
    SPR_VAULT_DOOR_CLOSED,
    SPR_VAULT_DOOR_OPEN,
    SPR_CAMERA_IDLE,
    SPR_CAMERA_ALERT,
    SPR_EXIT_MARKER,
    SPR_ESCAPE_KEY,
    SPR_LOOT_CASH,
    SPR_LOOT_JEWEL,
    SPR_LOOT_GOLD,
    SPR_LOOT_ART,
    SPR_SPAWN_MARKER,
    SPR_SERVER,
    SPR_STAIRS_UP,
    SPR_STAIRS_DOWN,
    SPR_ELEVATOR,
    SPR_GUARD_PATROL,
    SPR_GUARD_SUSPICIOUS,
    SPR_GUARD_ALERT,
    SPR_GUARD_DEAD,
    SPR_GUARD_REINFORCEMENT,
    SPR_GUARD_ELITE,
    SPR_CIVILIAN,
    SPR_PLAYER,
    SPR_PLAYER_CROUCH,
    SPR_PLAYER_CLOAKED,
    SPR_COUNT
};

// ─── Guard states ─────────────────────────────────────────────────────────────
enum GuardState { GS_PATROL, GS_SUSPICIOUS, GS_ALERT, GS_CHASE, GS_INVESTIGATE, GS_DEAD };

struct PatrolPoint { Vec2 pos; };

struct Guard {
    Vec2  pos;
    float angle;
    float hp, maxHp;
    GuardState state;
    float suspicion;
    float alertTimer;
    float stateTimer;
    int   patrolIdx;
    std::vector<PatrolPoint> patrol;
    bool  alive;
    float visionRange, visionAngle;
    int   weaponIdx;
    float shootCooldown;
    bool  visibleToPlayer;
    bool  isReinforcement;
    bool  isElite;        // elite guard: more HP, faster, melee, better aim

    // AI
    Vec2  lastKnownPlayerPos;
    bool  hasLastKnown;
    float investigateTimer;
    Vec2  coverPos;
    bool  hasCoverPos;
    float suppressTimer;
    bool  radioCalled;
    float radioTimer;
    float hearingRange;
    float walkAnimTime;

    // Pathfinding
    std::vector<Vec2i> path;
    int   pathIdx;
    float pathTimer;     // recompute path every X seconds

    // Melee
    float meleeCooldown;
    static constexpr float MELEE_RANGE  = 35.f;
    static constexpr float MELEE_DAMAGE = 30.f;
    float meleeDmg;      // per-guard (elite gets 2x)

    // Suspicious mode modifiers (compared to normal)
    // In suspicious: visionRange * 1.25, speed * 1.2, suspicion builds faster
    // In alert: back to normal speed but knows player pos
};

// ─── Camera (security) ───────────────────────────────────────────────────────
struct SecurityCam {
    Vec2  pos;
    float angle, sweepSpeed, sweepRange;
    bool  disabled, triggered;
    float disabledTimer;
    int   sectorId;   // which server sector controls this cam (-1 = all)
    bool  isFixed;    // vault cams don't sweep
};

// ─── Loot ────────────────────────────────────────────────────────────────────
struct LootItem {
    Vec2  pos;
    int   value;
    bool  collected;
    Color color;
    std::string label;
};

// ─── Escape Key ───────────────────────────────────────────────────────────────
struct EscapeKey {
    Vec2  pos;
    bool  collected;
};

// ─── Server terminal ─────────────────────────────────────────────────────────
struct ServerTerminal {
    Vec2 pos;
    bool destroyed;
    int  sectorId;   // which cameras this controls
    float hp;
};

// ─── Civilian ─────────────────────────────────────────────────────────────────
enum CivilianState { CIV_WANDER, CIV_PANIC, CIV_HIDING, CIV_HOSTAGE };
struct Civilian {
    Vec2  pos;
    float angle;
    bool  alive;
    CivilianState state;
    float panicTimer;
    float wanderTimer;
    Vec2  wanderTarget;
    float alarmRadius; // if gunshot within this range, civ panics -> suspicious
    bool  panicking;
};

// ─── Particle ────────────────────────────────────────────────────────────────
struct Particle {
    Vec2  pos, vel;
    float life, maxLife, size;
    Color color;
};

// ─── Bear Trap ───────────────────────────────────────────────────────────────
struct BearTrap {
    Vec2 pos;
    bool armed;
    bool triggered;
    float disarmTimer;
};

// ─── Sound effect IDs (procedural / stub) ────────────────────────────────────
enum SoundID {
    SND_GUNSHOT=0, SND_GUNSHOT_SILENCED, SND_RELOAD,
    SND_FOOTSTEP, SND_FOOTSTEP_CROUCH,
    SND_ALARM, SND_GUARD_SHOUT, SND_GUARD_SPOT,
    SND_LOOT_PICKUP, SND_VAULT_OPEN, SND_DOOR_OPEN,
    SND_MELEE_HIT, SND_PLAYER_HURT, SND_PLAYER_DEATH,
    SND_CIVILIAN_SCREAM, SND_SERVER_DESTROY,
    SND_EXPLOSION, SND_EMP, SND_CLICK,
    SND_COUNT
};

// ─── Forward decls ───────────────────────────────────────────────────────────
class Renderer;
class Map;
class Player;
class GameState;
class SoundSystem;