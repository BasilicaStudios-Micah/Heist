#pragma once
#include "game.h"
#include "renderer.h"
#include "map.h"
#include "player.h"
#include "sound.h"
#include <ctime>

enum Screen { SCR_MENU, SCR_MAP_SELECT, SCR_LOADOUT, SCR_SKILL_TREE, SCR_PLAYING, SCR_PAUSE, SCR_RESULTS, SCR_DEAD };
enum SkillTreeTab { ST_WEAPONS, ST_ITEMS, ST_ABILITIES, ST_LEVELS, ST_STATS };

struct AlarmState {
    int   level; // 0=none,1=suspicious,2=alert,3=lockdown
    float timer;
    bool  active;
};

// ─── Skyrim-style skill tree node (with screen-space tree positions) ──────────
// Each tab has nodes positioned in a tree graph with connecting lines

class GameState {
public:
    Screen screen = SCR_MENU;
    MetaData meta;
    Map map;
    Player player;
    std::vector<Guard>        guards;
    std::vector<SecurityCam>  cams;
    std::vector<Bullet>       bullets;
    std::vector<LootItem>     loot;
    std::vector<Particle>     particles;
    std::vector<Civilian>     civilians;
    std::vector<BearTrap>     worldTraps; // traps placed in world
    AlarmState alarm;

    // Loadout selection
    int loadoutWeapon=0;
    std::vector<int> loadoutItems;
    std::vector<int> loadoutAbilities;

    // Map select
    int mapSelectIdx=0;
    int selectedTheme=0;  // chosen theme for next run

    // Skill tree
    SkillTreeTab skillTab=ST_WEAPONS;
    int skillHover=-1;
    // Scroll offset for each tab
    float skillScrollY[5]={0,0,0,0,0};
    // Mouse press tracking (fix hold-to-click 50cps)
    bool  prevMouseLeftMenu=false;
    bool  mouseJustPressed=false;

    // Results
    int   runCash=0, runScore=0, xpEarned=0;
    bool  runSuccess=false;
    std::string deathReason;

    // Timers
    float alarmFlash=0, runTime=0;
    float vaultTimer=0;
    static const float VAULT_CRACK_TIME;
    bool  vaultCracking=false, vaultOpen=false;
    float escapeTimer=0;

    EscapeKey escapeKey;
    bool      hasEscapeKey=false;
    bool      atSpawnRoom=false;
    bool      reinforcementsSpawned=false;

    // Input
    bool   keys[GLFW_KEY_LAST+1];
    double mouseX=0, mouseY=0;
    bool   mouseLeft=false, mouseRight=false;
    bool   prevMouseLeft=false;
    int    menuSel=0;
    int    loadoutTab=0;
    float  camX=0, camY=0;

    void init();
    void startRun();
    void update(float dt);
    void render(Renderer& r);
    void saveData();
    void loadData();
    void endRun(bool success, const std::string& reason="");
    void awardXP(int xp);
    void resetMeta();   // called on death

    void onKey(int key, int action);
    void onMouse(double x, double y);
    void onMouseButton(int btn, int action);

private:
    void updatePlaying(float dt);
    void updateGuards(float dt);
    void updateBullets(float dt);
    void updateParticles(float dt);
    void updateAlarm(float dt);
    void updateCameras(float dt);
    void updateCivilians(float dt);
    void updateBearTraps(float dt);

    void renderMenu(Renderer& r);
    void renderMapSelect(Renderer& r);
    void renderLoadout(Renderer& r);
    void renderSkillTree(Renderer& r);
    void renderPlaying(Renderer& r);
    void renderHUD(Renderer& r);
    void renderMinimap(Renderer& r);
    void renderResults(Renderer& r);
    void renderDeadScreen(Renderer& r);
    void renderPauseMenu(Renderer& r);
    void renderRooms(Renderer& r);

    // Button helper: returns true only on a FRESH click (not hold)
    void drawButton(Renderer& r,float x,float y,float w,float h,
                    const std::string& label,bool sel,Color col={0.2f,0.22f,0.28f,1});
    bool buttonHit(float x,float y,float w,float h); // true on mouseJustPressed inside

    std::vector<SkillNode> weaponNodes();
    std::vector<SkillNode> itemNodes();
    std::vector<SkillNode> abilityNodes();
    std::vector<SkillNode> levelNodes();
    std::vector<SkillNode> statNodes();
    void unlockNode(SkillTreeTab tab,int nodeIdx);
    bool canUnlockNode(SkillTreeTab tab,int nodeIdx);
    std::vector<SkillNode>& getNodes(SkillTreeTab tab);
    std::vector<SkillNode> wNodes,iNodes,aNodes,lNodes,sNodes;

    std::mt19937 rng;
    int   runSeed=0;
    int   stealthScore=0;
    float stealthBreachTimer=0;

    // Suspicious-mode guard multipliers
    // SUSPICIOUS: visionRange*1.3, speed*1.25, suspicion build *1.5
    // ALERT:      normal speed (but knows player)
    static constexpr float SUSP_VISION_MULT  = 1.30f;
    static constexpr float SUSP_SPEED_MULT   = 1.25f;
    static constexpr float SUSP_SUSP_MULT    = 1.50f;
    static constexpr float SUSP_CAM_FOV_MULT = 1.40f; // cameras get wider FOV

    // Civilian panic raises suspicion
    void triggerCivilianPanic(Vec2 pos, float radius);
};

const float GameState::VAULT_CRACK_TIME = 8.f;

// ─── Save / Load ──────────────────────────────────────────────────────────────
void GameState::saveData(){
    std::ofstream f("heist_save.dat");
    if(!f) return;
    f<<meta.metaLevel<<" "<<meta.metaXP<<" "<<meta.totalRuns
     <<" "<<meta.totalCash<<" "<<meta.bestScore<<"\n";
    f<<meta.bonusMaxHP<<" "<<meta.bonusSpeed<<" "<<meta.bonusStealth
     <<" "<<meta.bonusLuck<<" "<<meta.bonusCashMult<<" "<<meta.statPointsSpent<<"\n";
    f<<meta.bonusDamage<<" "<<meta.bonusReload<<" "<<meta.bonusClipSize
     <<" "<<meta.bonusCooldown<<" "<<meta.bonusXP<<"\n";
    auto writeVec=[&](std::vector<int>& v){ f<<v.size(); for(int x:v)f<<" "<<x; f<<"\n"; };
    writeVec(meta.unlockedWeapons); writeVec(meta.unlockedItems);
    writeVec(meta.unlockedAbilities); writeVec(meta.unlockedThemes);
    auto writeNodes=[&](std::vector<SkillNode>& nodes){
        f<<nodes.size();
        for(auto& n:nodes) f<<" "<<(n.unlocked?1:0);
        f<<"\n";
    };
    writeNodes(wNodes); writeNodes(iNodes); writeNodes(aNodes);
    writeNodes(lNodes); writeNodes(sNodes);
}

void GameState::loadData(){
    std::ifstream f("heist_save.dat");
    if(!f) return;
    f>>meta.metaLevel>>meta.metaXP>>meta.totalRuns>>meta.totalCash>>meta.bestScore;
    f>>meta.bonusMaxHP>>meta.bonusSpeed>>meta.bonusStealth
     >>meta.bonusLuck>>meta.bonusCashMult>>meta.statPointsSpent;
    f>>meta.bonusDamage>>meta.bonusReload>>meta.bonusClipSize
     >>meta.bonusCooldown>>meta.bonusXP;
    size_t sz;
    auto readVec=[&](std::vector<int>& v){ f>>sz; v.resize(sz); for(auto& x:v)f>>x; };
    readVec(meta.unlockedWeapons); readVec(meta.unlockedItems);
    readVec(meta.unlockedAbilities); readVec(meta.unlockedThemes);
    auto readNodes=[&](std::vector<SkillNode>& nodes){
        size_t n; f>>n;
        if(n==nodes.size()) for(auto& nd:nodes){int u;f>>u;nd.unlocked=(u==1);}
    };
    readNodes(wNodes); readNodes(iNodes); readNodes(aNodes);
    readNodes(lNodes); readNodes(sNodes);
}

void GameState::resetMeta(){
    meta=MetaData{};
    // Remove save file
    std::remove("heist_save.dat");
    // Re-init nodes
    wNodes=weaponNodes(); iNodes=itemNodes(); aNodes=abilityNodes();
    lNodes=levelNodes();  sNodes=statNodes();
    // Ensure starters unlocked
    if(!wNodes.empty()) wNodes[0].unlocked=true;
    if(!iNodes.empty()) iNodes[0].unlocked=true;
    if(iNodes.size()>1) iNodes[1].unlocked=true;
    if(!aNodes.empty()) aNodes[0].unlocked=true;
    if(!lNodes.empty()) lNodes[0].unlocked=true;
}

// ─── Skill tree node builders (Skyrim-style with tree positions) ──────────────
// treeX/treeY are 0..1 fractions of the skill tree panel
std::vector<SkillNode> GameState::weaponNodes(){
    std::vector<SkillNode> n;
    // Root weapons (unlocked at start)
    n.push_back({"Silenced Pistol", "Starting weapon",             0,   -1,true,  "weapon",0,    0.5f,0.05f});
    // Tier 1
    n.push_back({"SMG Training",    "Unlock SMG",                 500,   0,false, "weapon",1,    0.25f,0.25f});
    n.push_back({"Revolver Cert",   "Unlock Revolver",            550,   0,false, "weapon",6,    0.5f, 0.25f});
    n.push_back({"Machine Pistol",  "Unlock Machine Pistol",      480,   0,false, "weapon",8,    0.75f,0.25f});
    // Tier 2
    n.push_back({"Shotgun Cert",    "Unlock Shotgun",            1000,   1,false, "weapon",2,    0.15f,0.45f});
    n.push_back({"PDW Training",    "Unlock PDW",                 900,   1,false, "weapon",11,   0.4f, 0.45f});
    n.push_back({"Crossbow Cert",   "Unlock Crossbow",            950,   2,false, "weapon",7,    0.65f,0.45f});
    n.push_back({"Tranq Rifle",     "Unlock Tranq Rifle",         900,   2,false, "weapon",12,   0.85f,0.45f});
    // Tier 3
    n.push_back({"Sniper Training", "Unlock Sniper Rifle",       1500,   4,false, "weapon",3,    0.2f, 0.65f});
    n.push_back({"Flare Gun Cert",  "Unlock Flare Gun",          1200,   6,false, "weapon",13,   0.5f, 0.65f});
    n.push_back({"Grenade Launcher","Unlock Grenade Launcher",   2000,   5,false, "weapon",9,    0.75f,0.65f});
    // Tier 4
    n.push_back({"Tech Weapons",    "Unlock EMP Rifle",          2500,   8,false, "weapon",4,    0.2f, 0.82f});
    n.push_back({"Laser Pistol",    "Unlock Laser Pistol",       2800,   9,false, "weapon",10,   0.5f, 0.82f});
    n.push_back({"Railgun Access",  "Unlock Railgun",            5000,  11,false, "weapon",5,    0.35f,0.95f});
    // Passive upgrades
    n.push_back({"Quick Draw",      "+20% fire rate",             800,   0,false, "fireRate",0.2f, 0.9f,0.32f});
    n.push_back({"Hollow Point",    "+25 flat damage",           1200,  14,false, "dmg",25.f,    0.9f,0.55f});
    n.push_back({"Extended Mag",    "+30% clip size",             900,   0,false, "clipSize",0.3f, 0.9f,0.15f});
    n.push_back({"Suppressor Mk2",  "Silences non-silenced guns",3000,  13,false,"allSilenced",1, 0.9f,0.75f});
    n.push_back({"Speed Loader",    "-30% reload time",          1100,  16,false, "reload",0.3f,  0.9f,0.92f});
    return n;
}

std::vector<SkillNode> GameState::itemNodes(){
    std::vector<SkillNode> n;
    n.push_back({"Lockpick",       "Unlock Lockpick",             0,  -1,true,  "item",0,   0.15f,0.05f});
    n.push_back({"Medkit",         "Unlock Medkit",               0,  -1,true,  "item",4,   0.5f, 0.05f});
    n.push_back({"EMP Grenade",    "Unlock EMP Grenade",         600,   0,false, "item",1,   0.15f,0.25f});
    n.push_back({"Smoke Bomb",     "Unlock Smoke Bomb",          800,   0,false, "item",2,   0.5f, 0.25f});
    n.push_back({"Rope",           "Unlock Rope",                800,   1,false, "item",7,   0.85f,0.25f});
    n.push_back({"Flashbang",      "Unlock Flashbang",           900,   2,false, "item",8,   0.15f,0.45f});
    n.push_back({"Decoy",          "Unlock Decoy",              1000,   3,false, "item",3,   0.5f, 0.45f});
    n.push_back({"Gas Canister",   "Unlock Gas Canister",       1000,   5,false, "item",10,  0.85f,0.45f});
    n.push_back({"Hacking Kit",    "Unlock Hacking Kit",        1500,   5,false, "item",6,   0.25f,0.65f});
    n.push_back({"Jammer",         "Unlock Jammer",             1500,   6,false, "item",12,  0.6f, 0.65f});
    n.push_back({"Night Vision",   "Unlock Night Vision",       1200,   4,false, "item",14,  0.85f,0.65f});
    n.push_back({"Shaped Charge",  "Unlock Shaped Charge",      2000,   8,false, "item",9,   0.25f,0.82f});
    n.push_back({"Disguise Kit",   "Unlock Disguise Kit",       2500,   9,false, "item",11,  0.6f, 0.82f});
    n.push_back({"C4",             "Unlock C4 Explosive",       3000,  11,false, "item",13,  0.4f, 0.95f});
    n.push_back({"Extra Carry",    "+1 item slot",              1200,   0,false, "itemSlot",1, 0.9f,0.15f});
    n.push_back({"Ammo Vest",      "+2 item charges each",       900,   0,false, "itemCharge",2,0.9f,0.45f});
    n.push_back({"Field Medic",    "Medkits restore 100HP",     1500,   1,false, "medkitBonus",25,0.9f,0.75f});
    return n;
}

std::vector<SkillNode> GameState::abilityNodes(){
    std::vector<SkillNode> n;
    n.push_back({"Dodge Roll",    "Unlock Dodge Roll",            0, -1,true,  "ability",0,  0.5f, 0.05f});
    n.push_back({"Sprint",        "Unlock Sprint",              600,  0,false, "ability",1,  0.2f, 0.25f});
    n.push_back({"Ghost Step",    "Unlock Ghost Step",         1000,  1,false, "ability",5,  0.5f, 0.25f});
    n.push_back({"Trap",          "Unlock Bear Trap",           900,  0,false, "ability",11, 0.8f, 0.25f});
    n.push_back({"Flashstep",     "Unlock Flashstep",          1800,  2,false, "ability",6,  0.2f, 0.45f});
    n.push_back({"Wall Hack",     "Unlock Wall Hack",          2000,  1,false, "ability",2,  0.5f, 0.45f});
    n.push_back({"Meditate",      "Unlock Meditate",           1500,  2,false, "ability",9,  0.8f, 0.45f});
    n.push_back({"Adrenaline",    "Unlock Adrenaline",         2500,  4,false, "ability",4,  0.2f, 0.65f});
    n.push_back({"Overcharge",    "Unlock Overcharge",         2800,  5,false, "ability",7,  0.5f, 0.65f});
    n.push_back({"Berserk",       "Unlock Berserk",            3000,  7,false, "ability",10, 0.8f, 0.65f});
    n.push_back({"Vanish",        "Unlock Vanish",             4000,  7,false, "ability",8,  0.35f,0.82f});
    n.push_back({"Time Slow",     "Unlock Time Slow",          4000,  5,false, "ability",3,  0.65f,0.82f});
    n.push_back({"Agility I",     "-20% all cooldowns",         800,  0,false, "cdReduce",0.2f, 0.9f,0.35f});
    n.push_back({"Agility II",    "-20% more cooldowns",       1600, 12,false, "cdReduce",0.2f, 0.9f,0.65f});
    return n;
}

std::vector<SkillNode> GameState::levelNodes(){
    std::vector<SkillNode> n;
    n.push_back({"Corner Bank",      "Starting theme",              0, -1,true,  "theme",0,  0.15f,0.05f});
    n.push_back({"Casino",           "Unlock Casino",             800,  0,false, "theme",1,  0.35f,0.25f});
    n.push_back({"Art Museum",       "Unlock Art Museum",        1300,  1,false, "theme",2,  0.55f,0.45f});
    n.push_back({"Federal Reserve",  "Unlock Federal Reserve",   2200,  2,false, "theme",3,  0.35f,0.65f});
    n.push_back({"Swiss Vault",      "Unlock Swiss Vault",       3500,  3,false, "theme",4,  0.55f,0.82f});
    n.push_back({"Offshore Fortress","Unlock Fortress",          6000,  4,false, "theme",5,  0.45f,0.95f});
    n.push_back({"Loot Boost I",    "+25% loot value",          1000,  0,false, "loot",0.25f, 0.8f,0.15f});
    n.push_back({"Loot Boost II",   "+50% loot value",          2000,  6,false, "loot",0.5f,  0.8f,0.45f});
    n.push_back({"Loot Boost III",  "+75% loot value",          4000,  7,false, "loot",0.75f, 0.8f,0.75f});
    n.push_back({"XP Boost I",      "+25% XP earned",           1200,  0,false, "xp",0.25f,  0.95f,0.25f});
    n.push_back({"XP Boost II",     "+50% XP earned",           2500,  9,false, "xp",0.5f,   0.95f,0.55f});
    n.push_back({"Cash Multiplier", "+20% final cash",           1500,  0,false, "cash",0.2f, 0.95f,0.82f});
    return n;
}

std::vector<SkillNode> GameState::statNodes(){
    std::vector<SkillNode> n;
    // HP branch (left)
    n.push_back({"HP I",       "+30 max HP",            400, -1,false, "hp",30,     0.1f,0.15f});
    n.push_back({"HP II",      "+30 max HP",            800,  0,false, "hp",30,     0.1f,0.35f});
    n.push_back({"HP III",     "+50 max HP",           1600,  1,false, "hp",50,     0.1f,0.55f});
    n.push_back({"HP IV",      "+75 max HP",           3000,  2,false, "hp",75,     0.1f,0.75f});
    // Speed branch
    n.push_back({"Speed I",    "+20 move speed",        400, -1,false, "speed",20,  0.3f,0.15f});
    n.push_back({"Speed II",   "+25 move speed",        900,  4,false, "speed",25,  0.3f,0.45f});
    n.push_back({"Speed III",  "+30 move speed",       1800,  5,false, "speed",30,  0.3f,0.75f});
    // Stealth branch
    n.push_back({"Stealth I",  "-20 detect radius",    500, -1,false, "stealth",20, 0.55f,0.15f});
    n.push_back({"Stealth II", "-30 detect radius",   1000,  7,false, "stealth",30, 0.55f,0.45f});
    n.push_back({"Stealth III","-40 detect radius",   2000,  8,false, "stealth",40, 0.55f,0.75f});
    // Luck branch
    n.push_back({"Luck I",     "+10% luck",             600, -1,false, "luck",0.1f, 0.75f,0.15f});
    n.push_back({"Luck II",    "+15% luck",            1200, 10,false, "luck",0.15f,0.75f,0.45f});
    n.push_back({"Luck III",   "+25% luck",            2500, 11,false, "luck",0.25f,0.75f,0.75f});
    // Damage branch
    n.push_back({"Damage I",   "+15 flat damage",       700, -1,false, "dmg",15,    0.9f, 0.15f});
    n.push_back({"Damage II",  "+25 flat damage",      1400, 13,false, "dmg",25,    0.9f, 0.45f});
    n.push_back({"Damage III", "+40 flat damage",      2800, 14,false, "dmg",40,    0.9f, 0.75f});
    return n;
}

std::vector<SkillNode>& GameState::getNodes(SkillTreeTab tab){
    switch(tab){
        case ST_WEAPONS:   return wNodes;
        case ST_ITEMS:     return iNodes;
        case ST_ABILITIES: return aNodes;
        case ST_LEVELS:    return lNodes;
        case ST_STATS:     return sNodes;
    }
    return wNodes;
}

bool GameState::canUnlockNode(SkillTreeTab tab,int idx){
    auto& nodes=getNodes(tab);
    if(idx<0||idx>=(int)nodes.size()) return false;
    auto& n=nodes[idx];
    if(n.unlocked) return false;
    if(n.cost>meta.metaXP) return false;
    if(n.parentIdx>=0&&!nodes[n.parentIdx].unlocked) return false;
    return true;
}

void GameState::unlockNode(SkillTreeTab tab,int idx){
    if(!canUnlockNode(tab,idx)) return;
    SND(SND_CLICK,0.6f,1.8f);
    auto& nodes=getNodes(tab);
    auto& n=nodes[idx];
    meta.metaXP-=n.cost;
    n.unlocked=true;
    const std::string& k=n.statKey;
    float v=n.statVal;
    if(k=="weapon"){
        int wi=(int)v;
        if(std::find(meta.unlockedWeapons.begin(),meta.unlockedWeapons.end(),wi)==meta.unlockedWeapons.end())
            meta.unlockedWeapons.push_back(wi);
    }else if(k=="item"){
        int ii=(int)v;
        if(std::find(meta.unlockedItems.begin(),meta.unlockedItems.end(),ii)==meta.unlockedItems.end())
            meta.unlockedItems.push_back(ii);
    }else if(k=="ability"){
        int ai=(int)v;
        if(std::find(meta.unlockedAbilities.begin(),meta.unlockedAbilities.end(),ai)==meta.unlockedAbilities.end())
            meta.unlockedAbilities.push_back(ai);
    }else if(k=="theme"){
        int ti=(int)v;
        if(std::find(meta.unlockedThemes.begin(),meta.unlockedThemes.end(),ti)==meta.unlockedThemes.end())
            meta.unlockedThemes.push_back(ti);
    }else if(k=="hp")        meta.bonusMaxHP   +=v;
    else if(k=="speed")      meta.bonusSpeed   +=v;
    else if(k=="stealth")    meta.bonusStealth +=v;
    else if(k=="luck")       meta.bonusLuck    +=v;
    else if(k=="cash")       meta.bonusCashMult+=v;
    else if(k=="dmg")        meta.bonusDamage  +=v;
    else if(k=="reload")     meta.bonusReload  +=v;
    else if(k=="clipSize")   meta.bonusClipSize+=v;
    else if(k=="cdReduce")   meta.bonusCooldown+=v;
    else if(k=="xp")         meta.bonusXP      +=v;
    meta.statPointsSpent++;
    meta.metaLevel=meta.statPointsSpent/3;
    saveData();
}

void GameState::init(){
    memset(keys,0,sizeof(keys));
    alarm={0,0,false};
    rng.seed((unsigned)time(nullptr));
    wNodes=weaponNodes(); iNodes=itemNodes(); aNodes=abilityNodes();
    lNodes=levelNodes();  sNodes=statNodes();
    loadData();
    if(!wNodes.empty()) wNodes[0].unlocked=true;
    if(!iNodes.empty()) iNodes[0].unlocked=true;
    if(iNodes.size()>1) iNodes[1].unlocked=true;
    if(!aNodes.empty()) aNodes[0].unlocked=true;
    if(!lNodes.empty()) lNodes[0].unlocked=true;
    g_sound.init();
    loadoutItems={0,4};
    loadoutAbilities={0};
}

void GameState::startRun(){
    runSeed=(int)rng();
    int themeIdx=selectedTheme;
    // Clamp to unlocked
    if(std::find(meta.unlockedThemes.begin(),meta.unlockedThemes.end(),themeIdx)==meta.unlockedThemes.end())
        themeIdx=meta.unlockedThemes.empty()?0:meta.unlockedThemes[0];

    map.generate(runSeed,themeIdx,meta.metaLevel);
    loot=map.makeLoot(meta.metaLevel,meta.bonusLuck);
    int guardCount=THEME_DEFS[themeIdx].guardCount+meta.metaLevel/3;
    guards=map.makeGuards(guardCount,meta.metaLevel);
    cams=map.makeCameras(THEME_DEFS[themeIdx].camCount+meta.metaLevel/4);
    // Civilians: 3-6 based on theme
    int civCount=3+(themeIdx==0?3:themeIdx==1?5:2);
    civilians=map.makeCivilians(civCount);
    bullets.clear(); particles.clear(); worldTraps.clear();
    alarm={0,0,false}; alarmFlash=0; runTime=0;
    vaultTimer=0; vaultCracking=false; vaultOpen=false;
    stealthScore=100; stealthBreachTimer=0;
    escapeKey=map.makeEscapeKey(); hasEscapeKey=false; atSpawnRoom=false;
    reinforcementsSpawned=false; runCash=0;

    player.init(meta,loadoutWeapon,loadoutItems,loadoutAbilities);
    player.pos=map.playerStart;
    camX=player.pos.x-SCREEN_W/2.f;
    camY=player.pos.y-SCREEN_H/2.f;
    screen=SCR_PLAYING;
    SND(SND_CLICK,0.4f);
}

void GameState::awardXP(int xp){
    float mult=1.f+meta.bonusXP;
    meta.metaXP+=(int)(xp*mult);
    saveData();
}

void GameState::endRun(bool success,const std::string& reason){
    runSuccess=success; deathReason=reason;
    meta.totalRuns++;
    runScore=runCash;
    if(success){ runScore+=stealthScore*10; }
    if(success&&!player.detected) runScore+=5000;
    if(success) runScore+=std::max(0,(int)(300.f-runTime)*10);
    runScore=std::max(0,runScore);
    if(runScore>meta.bestScore) meta.bestScore=runScore;
    meta.totalCash+=runCash;
    xpEarned=runScore/10+runCash/50+(success?200:20);
    awardXP(xpEarned);
    if(success) screen=SCR_RESULTS;
    else{
        // DEATH: reset all meta progress
        screen=SCR_DEAD;
    }
}

// ─── Input ────────────────────────────────────────────────────────────────────
void GameState::onKey(int key,int action){
    if(key>=0&&key<=GLFW_KEY_LAST){
        keys[key]=(action!=GLFW_RELEASE);
    }
    if(screen==SCR_PLAYING||screen==SCR_PAUSE){
        if(action==GLFW_PRESS){
            if(key==GLFW_KEY_ESCAPE)
                screen=(screen==SCR_PAUSE?SCR_PLAYING:SCR_PAUSE);
            if(screen==SCR_PLAYING){
                if(key==GLFW_KEY_1&&player.abilityLoadout.size()>0) player.useAbility(0);
                if(key==GLFW_KEY_2&&player.abilityLoadout.size()>1) player.useAbility(1);
                if(key==GLFW_KEY_3&&player.abilityLoadout.size()>2) player.useAbility(2);
                if(key==GLFW_KEY_4&&player.abilityLoadout.size()>3) player.useAbility(3);
                if(key==GLFW_KEY_Z) player.useItem(0,particles,cams,guards,civilians);
                if(key==GLFW_KEY_X) player.useItem(1,particles,cams,guards,civilians);
                if(key==GLFW_KEY_V) player.useItem(2,particles,cams,guards,civilians);
                if(key==GLFW_KEY_B) player.useItem(3,particles,cams,guards,civilians);
            }
        }
    }
}

void GameState::onMouse(double x,double y){ mouseX=x; mouseY=y; }
void GameState::onMouseButton(int btn,int action){
    if(btn==GLFW_MOUSE_BUTTON_LEFT){
        bool wasDown=mouseLeft;
        mouseLeft=(action==GLFW_PRESS);
        mouseJustPressed=(mouseLeft&&!wasDown);
        if(btn==GLFW_MOUSE_BUTTON_RIGHT) mouseRight=(action==GLFW_PRESS);
    }
    if(btn==GLFW_MOUSE_BUTTON_RIGHT) mouseRight=(action==GLFW_PRESS);
}

// ─── Button helpers ───────────────────────────────────────────────────────────
void GameState::drawButton(Renderer& r,float x,float y,float w,float h,
                            const std::string& label,bool sel,Color col){
    bool hover=(mouseX>=x&&mouseX<=x+w&&mouseY>=y&&mouseY<=y+h);
    Color bg=col;
    if(hover){ bg.r=std::min(1.f,bg.r+0.15f); bg.g=std::min(1.f,bg.g+0.15f); bg.b=std::min(1.f,bg.b+0.15f); }
    if(sel){ bg.r=std::min(1.f,bg.r+0.2f); bg.g=std::min(1.f,bg.g+0.2f); }
    r.drawQuadScreen(x,y,w,h,bg);
    r.drawQuadScreen(x,y,w,2,{1,1,1,0.3f});
    r.drawQuadScreen(x,y+h-2,w,2,{0,0,0,0.3f});
    float tx=x+w/2.f-label.size()*4.8f;
    r.drawText(label,tx,y+h/2.f-6,1.5f,sel?C_YELLOW:C_WHITE);
}

bool GameState::buttonHit(float x,float y,float w,float h){
    return mouseJustPressed&&mouseX>=x&&mouseX<=x+w&&mouseY>=y&&mouseY<=y+h;
}

// ─── Update ───────────────────────────────────────────────────────────────────
void GameState::update(float dt){
    // Clamp dt
    dt=std::min(dt,0.05f);

    // Build mouseJustPressed from raw state (also covers keyboard-driven menus)
    // mouseJustPressed is set in onMouseButton; clear it after one frame
    // (it gets cleared at the end of update)

    switch(screen){
    case SCR_PLAYING:
    case SCR_PAUSE:
        if(screen==SCR_PLAYING) updatePlaying(dt);
        break;
    default: break;
    }
    mouseJustPressed=false;
    prevMouseLeft=mouseLeft;
}

// ─── Civilian panic ───────────────────────────────────────────────────────────
void GameState::triggerCivilianPanic(Vec2 pos2,float radius){
    for(auto& civ:civilians){
        if(!civ.alive) continue;
        if(v2len(v2sub(civ.pos,pos2))<radius){
            civ.state=CIV_PANIC;
            civ.panicTimer=8.f;
            if(!alarm.active||alarm.level<1){
                alarm.level=std::max(alarm.level,1);
                alarm.active=true;
                alarm.timer=20.f;
                SND(SND_CIVILIAN_SCREAM,0.7f);
            }
        }
    }
}

void GameState::updateCivilians(float dt){
    const float CIV_SPEED=70.f, WANDER_RADIUS=80.f;
    for(auto& civ:civilians){
        if(!civ.alive) continue;

        civ.wanderTimer-=dt;
        if(civ.panicTimer>0) civ.panicTimer-=dt;
        if(civ.panicTimer<=0&&civ.state==CIV_PANIC) civ.state=CIV_WANDER;

        // Move toward wander target
        Vec2 d=v2sub(civ.wanderTarget,civ.pos);
        float dist=v2len(d);
        float spd=(civ.state==CIV_PANIC)?CIV_SPEED*2.f:CIV_SPEED;
        if(dist>8.f){
            Vec2 dir=v2norm(d);
            Vec2 np={civ.pos.x+dir.x*spd*dt, civ.pos.y+dir.y*spd*dt};
            if(!map.isSolid(np.x,np.y)) civ.pos=np;
            else { // pick new target
                civ.wanderTimer=0;
            }
            civ.angle=atan2f(dir.y,dir.x);
        }
        if(civ.wanderTimer<=0||dist<8.f){
            civ.wanderTimer=3.f+((float)rand()/RAND_MAX)*4.f;
            float a=(float)(rand()%628)/100.f;
            float r=20.f+((float)rand()/RAND_MAX)*WANDER_RADIUS;
            civ.wanderTarget={civ.pos.x+cosf(a)*r, civ.pos.y+sinf(a)*r};
        }

        // Detect nearby gunshots (via alarm level or bullet near them)
        for(auto& b:bullets){
            if(!b.alive||!b.fromPlayer) continue;
            float dist2=v2len(v2sub(b.pos,civ.pos));
            if(dist2<civ.alarmRadius&&!WEAPON_DEFS[player.weaponIdx].silenced){
                triggerCivilianPanic(civ.pos,civ.alarmRadius*0.5f);
                civ.state=CIV_PANIC; civ.panicTimer=10.f;
            }
        }
    }
}

void GameState::updateBearTraps(float dt){
    for(auto& trap:worldTraps){
        if(!trap.armed||trap.triggered) continue;
        // Check guards
        for(auto& g:guards){
            if(!g.alive) continue;
            if(v2len(v2sub(g.pos,trap.pos))<20.f){
                trap.triggered=true;
                g.hp-=60.f; if(g.hp<=0){ g.alive=false; g.state=GS_DEAD; }
                else{ g.stateTimer=4.f; } // stunned
                SND(SND_MELEE_HIT,0.8f,0.7f);
                for(int i=0;i<20;i++){
                    Particle p{}; p.pos=trap.pos;
                    float a=(float)(rand()%628)/100.f;
                    p.vel={cosf(a)*100,sinf(a)*100};
                    p.life=p.maxLife=0.5f; p.size=4; p.color=C_RED;
                    particles.push_back(p);
                }
            }
        }
    }
    // Sync player bear traps
    for(auto& pt:player.bearTraps){
        if(!pt.armed) continue;
        worldTraps.push_back(pt);
        pt.armed=false;
    }
    player.bearTraps.clear();
}

void GameState::updateBullets(float dt){
    for(auto& b:bullets){
        if(!b.alive) continue;
        float speed=v2len(b.vel);
        Vec2 step=v2scale(v2norm(b.vel),speed*dt);
        // Railgun: pierce walls
        if(!b.penetrating&&map.isSolid(b.pos.x+step.x,b.pos.y)){
            b.alive=false;
            if(b.explosive) goto explode;
            continue;
        }
        if(!b.penetrating&&map.isSolid(b.pos.x,b.pos.y+step.y)){
            b.alive=false;
            if(b.explosive) goto explode;
            continue;
        }
        b.pos.x+=step.x; b.pos.y+=step.y;
        b.traveled+=speed*dt;
        if(b.traveled>b.range){ b.alive=false; continue; }

        if(b.fromPlayer){
            // Hit guards
            for(auto& g:guards){
                if(!g.alive||g.state==GS_DEAD) continue;
                if(v2len(v2sub(g.pos,b.pos))<16.f){
                    b.alive=false;
                    if(b.tranq){
                        // Stun: guard becomes suspicious for 10s
                        g.stateTimer=10.f; g.state=GS_SUSPICIOUS;
                    }else{
                        float dmg=b.damage*(g.isElite?0.7f:1.f);
                        g.hp-=dmg;
                        if(g.hp<=0){ g.hp=0; g.alive=false; g.state=GS_DEAD; }
                        else if(g.state==GS_PATROL||g.state==GS_SUSPICIOUS){
                            g.state=GS_CHASE; g.alertTimer=15.f;
                            g.lastKnownPlayerPos=player.pos; g.hasLastKnown=true;
                        }
                    }
                    // Hit particles
                    for(int i=0;i<12;i++){
                        Particle p{}; p.pos=b.pos;
                        float a=(float)(rand()%628)/100.f;
                        p.vel={cosf(a)*80.f,sinf(a)*80.f};
                        p.life=p.maxLife=0.4f; p.size=3;
                        p.color=b.tranq?C_GREEN:C_RED;
                        particles.push_back(p);
                    }
                    if(!b.penetrating) break;
                }
            }
            // Hit servers
            for(auto& sv:map.servers){
                if(sv.destroyed) continue;
                if(v2len(v2sub(sv.pos,b.pos))<18.f){
                    sv.hp-=b.damage*0.5f;
                    if(sv.hp<=0){
                        sv.destroyed=true;
                        SND(SND_SERVER_DESTROY,0.8f);
                        // Disable cameras in this sector (or all if only 1 server)
                        bool onlyOne=map.servers.size()==1;
                        for(auto& cam:cams){
                            if(onlyOne||cam.sectorId==sv.sectorId){
                                cam.disabled=true; cam.disabledTimer=9999.f;
                            }
                        }
                        for(int i=0;i<40;i++){
                            Particle p{}; p.pos=sv.pos;
                            float a=(float)(rand()%628)/100.f;
                            p.vel={cosf(a)*120,sinf(a)*120};
                            p.life=p.maxLife=0.8f; p.size=5;
                            p.color=(i%2)?C_CYAN:C_ORANGE;
                            particles.push_back(p);
                        }
                    }
                    b.alive=false;
                }
            }
            // Hearing: unsilenced shots trigger civilian panic
            if(!WEAPON_DEFS[player.weaponIdx].silenced)
                triggerCivilianPanic(b.pos,180.f);
        }else{
            // Enemy bullet hits player
            if(v2len(v2sub(player.pos,b.pos))<12.f&&player.alive){
                player.takeDamage(b.damage,particles);
                b.alive=false;
                if(!player.alive){ endRun(false,"Killed by guard fire"); }
            }
        }
        continue;
        explode:
        {
            SND(SND_EXPLOSION,0.9f);
            float erad=100.f;
            for(auto& g:guards){
                if(!g.alive) continue;
                float d=v2len(v2sub(g.pos,b.pos));
                if(d<erad){ g.hp-=b.damage*(1.f-d/erad); if(g.hp<=0){g.alive=false;g.state=GS_DEAD;} }
            }
            if(v2len(v2sub(player.pos,b.pos))<erad)
                player.takeDamage(b.damage*0.5f,particles);
            for(int i=0;i<60;i++){
                Particle p{}; p.pos=b.pos;
                float a=(float)(rand()%628)/100.f; float spd2=50+rand()%350;
                p.vel={cosf(a)*spd2,sinf(a)*spd2};
                p.life=p.maxLife=1.f; p.size=5+rand()%8;
                p.color=(i%3==0)?C_RED:(i%3==1)?C_ORANGE:C_YELLOW;
                particles.push_back(p);
            }
        }
    }
}

void GameState::updateParticles(float dt){
    for(auto& p:particles){ p.pos=v2add(p.pos,v2scale(p.vel,dt)); p.life-=dt; }
    particles.erase(std::remove_if(particles.begin(),particles.end(),
        [](const Particle& p){return p.life<=0;}),particles.end());
}

void GameState::updateCameras(float dt){
    bool suspicious=(alarm.active&&alarm.level==1);
    float fovMult=suspicious?SUSP_CAM_FOV_MULT:1.f;

    for(auto& cam:cams){
        if(cam.disabled){
            cam.disabledTimer-=dt;
            if(cam.disabledTimer<=0){ cam.disabled=false; cam.triggered=false; }
            continue;
        }
        if(!cam.isFixed) cam.angle+=cam.sweepSpeed*dt;

        // Check if player in cone
        float effectiveFOV=cam.sweepRange*fovMult;
        Vec2 d=v2sub(player.pos,cam.pos);
        float dist=v2len(d);
        if(dist<250.f&&!player.cloaked&&!player.disguised){
            float pa=atan2f(d.y,d.x);
            float diff=pa-cam.angle;
            while(diff>(float)M_PI)  diff-=2*(float)M_PI;
            while(diff<-(float)M_PI) diff+=2*(float)M_PI;
            if(fabsf(diff)<effectiveFOV*0.5f&&map.lineOfSight(cam.pos,player.pos)){
                if(!cam.triggered){
                    cam.triggered=true;
                    alarm.level=std::max(alarm.level,2);
                    if(!alarm.active) reinforcementsSpawned=false;
                    alarm.active=true; alarm.timer=30.f;
                    player.detected=true;
                    SND(SND_ALARM,0.8f);
                }
            }
        }
    }
}

void GameState::updateAlarm(float dt){
    if(!alarm.active) return;
    alarm.timer-=dt;
    if(alarm.level>=2){
        for(auto& g:guards){
            if(!g.alive||g.state==GS_DEAD) continue;
            if(g.state!=GS_CHASE&&g.state!=GS_ALERT){
                g.state=GS_ALERT; g.alertTimer=alarm.timer+1.f;
            }
            Vec2 d=v2sub(player.pos,g.pos);
            if(v2len(d)>1.f) g.angle=atan2f(d.y,d.x);
        }
    }
    if(alarm.timer<=0){
        if(!reinforcementsSpawned){
            reinforcementsSpawned=true;
            int wave=THEME_DEFS[map.themeIdx].extraGuardWave;
            // Spawn from barracks rooms
            Room* barracks=nullptr;
            for(auto& r:map.rooms) if(r.type==ROOM_BARRACKS){ barracks=&r; break; }
            Vec2 spawnPos=barracks?map.roomCenter(*barracks):map.playerStart;
            for(int i=0;i<wave;i++){
                Guard g{};
                g.pos={spawnPos.x+(float)(rand()%60-30),spawnPos.y+(float)(rand()%60-30)};
                g.maxHp=80.f; g.hp=g.maxHp; g.alive=true;
                g.state=GS_CHASE; g.alertTimer=30.f;
                g.isReinforcement=true; g.isElite=false;
                g.visionRange=200.f; g.visionAngle=(float)M_PI*0.55f;
                g.weaponIdx=std::min((int)(meta.metaLevel/4),(int)WEAPON_DEFS.size()-1);
                g.hasLastKnown=true; g.lastKnownPlayerPos=player.pos;
                g.hearingRange=180.f; g.meleeDmg=30.f;
                guards.push_back(g);
            }
        }
        alarm.active=false; alarm.level=0;
        for(auto& c:cams) c.triggered=false;
    }
    alarmFlash=fmodf(alarmFlash+dt*4,2*(float)M_PI);
}

void GameState::updateGuards(float dt){
    float timeScale=player.timeSlow?0.4f:1.f;
    float adt=dt*timeScale;

    // Suspicious mode multipliers
    bool globalSuspicious=(alarm.active&&alarm.level==1);

    auto findCoverPos=[&](Guard& g)->Vec2{
        Vec2 away=v2norm(v2sub(g.pos,player.pos));
        for(int a=0;a<8;a++){
            float spread=((float)(a%4)-1.5f)*0.4f;
            float ang=atan2f(away.y,away.x)+spread;
            float dist2=60.f+(a/4)*40.f;
            Vec2 c={g.pos.x+cosf(ang)*dist2,g.pos.y+sinf(ang)*dist2};
            if(!map.isSolid(c.x,c.y)) return c;
        }
        return g.pos;
    };

    auto radioBroadcast=[&](Guard& caller,Vec2 knownPos){
        if(caller.radioCalled||caller.radioTimer>0) return;
        caller.radioCalled=true; caller.radioTimer=15.f;
        for(auto& g2:guards){
            if(!g2.alive||g2.state==GS_DEAD||&g2==&caller) continue;
            if(v2len(v2sub(g2.pos,caller.pos))<350.f){
                g2.lastKnownPlayerPos=knownPos; g2.hasLastKnown=true;
                if(g2.state==GS_PATROL){ g2.state=GS_SUSPICIOUS; g2.suspicion=0.6f; }
            }
        }
        SND(SND_GUARD_SHOUT,0.5f);
    };

    for(auto& g:guards){
        if(!g.alive||g.state==GS_DEAD) continue;
        g.shootCooldown-=adt; g.stateTimer-=adt; g.walkAnimTime+=adt;
        if(g.alertTimer>0) g.alertTimer-=adt;
        if(g.radioTimer>0){ g.radioTimer-=adt; if(g.radioTimer<=0) g.radioCalled=false; }
        if(g.suppressTimer>0) g.suppressTimer-=adt;
        if(g.meleeCooldown>0) g.meleeCooldown-=adt;
        if(g.pathTimer>0) g.pathTimer-=adt;

        // ── Vision ──────────────────────────────────────────────────────
        bool canSeePlayer=false;
        float distToPlayer=v2len(v2sub(player.pos,g.pos));

        // Suspicious mode: wider vision
        float visionMult=globalSuspicious?SUSP_VISION_MULT:1.f;
        float suspBuildMult=globalSuspicious?SUSP_SUSP_MULT:1.f;

        if(!player.cloaked&&!player.disguised){
            float effRange=g.visionRange*visionMult*(player.crouching?0.65f:1.f);
            if(distToPlayer<effRange){
                float pa=atan2f(player.pos.y-g.pos.y,player.pos.x-g.pos.x);
                float diff=pa-g.angle;
                while(diff>(float)M_PI)  diff-=2*(float)M_PI;
                while(diff<-(float)M_PI) diff+=2*(float)M_PI;
                if(fabsf(diff)<g.visionAngle*0.5f&&map.lineOfSight(g.pos,player.pos))
                    canSeePlayer=true;
            }
            // Hearing
            float noiseR=player.noiseRadius();
            if(noiseR>0&&distToPlayer<noiseR) canSeePlayer=true;
            for(auto& b:bullets){
                if(!b.alive||!b.fromPlayer) continue;
                if(!WEAPON_DEFS[player.weaponIdx].silenced)
                    if(v2len(v2sub(b.pos,g.pos))<g.hearingRange) canSeePlayer=true;
            }
        }
        if(canSeePlayer){ g.lastKnownPlayerPos=player.pos; g.hasLastKnown=true; }
        g.visibleToPlayer=player.wallHack||map.lineOfSight(player.pos,g.pos);

        // ── Guard speed modifier: suspicious = faster, alert = normal ───
        float baseSpeed=globalSuspicious&&g.state==GS_SUSPICIOUS?75.f*SUSP_SPEED_MULT:
                        (g.isElite?85.f:60.f);
        if(g.state==GS_ALERT||g.state==GS_CHASE) baseSpeed=g.isElite?90.f:65.f; // alert: normal

        // ── Melee check ────────────────────────────────────────────────
        if(distToPlayer<Guard::MELEE_RANGE&&g.meleeCooldown<=0&&player.alive){
            g.meleeCooldown=1.5f;
            player.takeDamage(g.meleeDmg,particles);
            SND(SND_MELEE_HIT,0.7f);
            if(!player.alive) endRun(false,"Beaten to death by a guard");
        }

        // ── A* Pathfinding helper ──────────────────────────────────────
        auto moveToward=[&](Vec2 target, float spd){
            Vec2i tileStart={(int)(g.pos.x/TILE_SIZE),(int)(g.pos.y/TILE_SIZE)};
            Vec2i tileGoal={(int)(target.x/TILE_SIZE),(int)(target.y/TILE_SIZE)};
            bool needPath=(g.path.empty()||g.pathIdx>=(int)g.path.size()||g.pathTimer<=0);
            if(needPath){
                g.path=map.findPath(tileStart,tileGoal);
                g.pathIdx=0; g.pathTimer=0.8f;
            }
            Vec2 dest=target;
            if(!g.path.empty()&&g.pathIdx<(int)g.path.size()){
                auto& step=g.path[g.pathIdx];
                dest={step.x*(float)TILE_SIZE+TILE_SIZE/2.f,
                      step.y*(float)TILE_SIZE+TILE_SIZE/2.f};
                if(v2len(v2sub(dest,g.pos))<12.f) g.pathIdx++;
            }
            Vec2 d=v2sub(dest,g.pos);
            float dist2=v2len(d);
            if(dist2>4.f){
                Vec2 dir=v2norm(d);
                Vec2 np={g.pos.x+dir.x*spd*adt, g.pos.y+dir.y*spd*adt};
                if(!map.isSolid(np.x,np.y)) g.pos=np;
                else{ g.path.clear(); } // recompute
                g.angle=atan2f(dir.y,dir.x);
            }
        };

        // ── State machine ─────────────────────────────────────────────
        switch(g.state){
        case GS_PATROL:{
            if(!g.patrol.empty()){
                Vec2 t=g.patrol[g.patrolIdx].pos;
                if(v2len(v2sub(t,g.pos))<14) g.patrolIdx=(g.patrolIdx+1)%g.patrol.size();
                moveToward(t,baseSpeed);
            }
            if(g.hasLastKnown&&!canSeePlayer){
                g.state=GS_INVESTIGATE; g.investigateTimer=6.f;
            }
            if(canSeePlayer){
                g.suspicion+=dt*0.9f*suspBuildMult;
                if(g.suspicion>=0.4f){ g.state=GS_SUSPICIOUS; SND(SND_GUARD_SPOT,0.6f); }
            }else{
                g.suspicion=std::max(0.f,g.suspicion-dt*0.3f);
            }
            break;
        }
        case GS_SUSPICIOUS:{
            // In suspicious: guard is faster, wider vision, suspicion builds faster
            moveToward(canSeePlayer?player.pos:g.lastKnownPlayerPos, baseSpeed*SUSP_SPEED_MULT);
            if(canSeePlayer){
                g.suspicion+=dt*1.5f*suspBuildMult;
                if(g.suspicion>=1.f){
                    g.state=GS_ALERT; g.alertTimer=20.f;
                    alarm.level=std::max(alarm.level,1);
                    if(!alarm.active) reinforcementsSpawned=false;
                    alarm.active=true; alarm.timer=20.f;
                    player.detected=true;
                    SND(SND_GUARD_SHOUT,0.8f);
                    radioBroadcast(g,player.pos);
                }
            }else{
                g.suspicion=std::max(0.f,g.suspicion-dt*0.5f);
                if(g.suspicion<=0){ g.state=GS_PATROL; }
            }
            break;
        }
        case GS_ALERT:
        case GS_CHASE:{
            if(g.hasLastKnown) moveToward(player.pos,baseSpeed);
            if(canSeePlayer){
                g.alertTimer=10.f;
                // Shoot at player
                if(g.shootCooldown<=0&&map.lineOfSight(g.pos,player.pos)){
                    const WeaponDef& w=WEAPON_DEFS[g.weaponIdx];
                    g.shootCooldown=w.fireRate*(g.isElite?0.7f:1.f);
                    float spread=0.15f-(g.isElite?0.06f:0.f);
                    float ba=atan2f(player.pos.y-g.pos.y,player.pos.x-g.pos.x);
                    ba+=(float)(rand()%200-100)/100.f*spread;
                    Bullet b{};
                    b.pos={g.pos.x+cosf(ba)*16,g.pos.y+sinf(ba)*16};
                    b.vel={cosf(ba)*w.bulletSpeed,sinf(ba)*w.bulletSpeed};
                    b.damage=w.damage*(g.isElite?1.4f:1.f);
                    b.range=w.range; b.alive=true; b.fromPlayer=false; b.color=C_RED;
                    bullets.push_back(b);
                    if(!w.silenced) SND(SND_GUNSHOT,0.4f,1.1f);
                    else SND(SND_GUNSHOT_SILENCED,0.2f);
                }
            }
            if(g.alertTimer<=0){ g.state=GS_PATROL; g.suspicion=0; g.path.clear(); }
            break;
        }
        case GS_INVESTIGATE:{
            if(g.hasLastKnown) moveToward(g.lastKnownPlayerPos,baseSpeed);
            g.investigateTimer-=adt;
            if(canSeePlayer){ g.state=GS_ALERT; g.alertTimer=15.f; radioBroadcast(g,player.pos); }
            if(g.investigateTimer<=0){ g.state=GS_PATROL; g.hasLastKnown=false; g.path.clear(); }
            break;
        }
        case GS_DEAD: break;
        }
    }
}

void GameState::updatePlaying(float dt){
    if(!player.alive) return;
    runTime+=dt;

    // Convert mouse screen->world
    double wx=mouseX+camX, wy=mouseY+camY;

    player.update(dt,map,keys,wx,wy,mouseLeft,mouseRight,bullets,particles);

    // Place bear traps from player
    for(auto& bt:player.bearTraps) worldTraps.push_back(bt);
    player.bearTraps.clear();

    updateBullets(dt);
    updateGuards(dt);
    updateCameras(dt);
    updateAlarm(dt);
    updateParticles(dt);
    updateCivilians(dt);
    updateBearTraps(dt);

    // Smooth camera
    float targetX=player.pos.x-SCREEN_W/2.f;
    float targetY=player.pos.y-SCREEN_H/2.f;
    camX+=(targetX-camX)*std::min(1.f,dt*8.f);
    camY+=(targetY-camY)*std::min(1.f,dt*8.f);

    // Loot pickup
    for(auto& l:loot){
        if(l.collected) continue;
        if(v2len(v2sub(l.pos,player.pos))<20.f){
            l.collected=true;
            runCash+=(int)(l.value*player.cashMult);
            SND(SND_LOOT_PICKUP,0.4f,1.0f+(float)rand()/RAND_MAX*0.2f);
            for(int i=0;i<15;i++){
                Particle p{}; p.pos=l.pos;
                float a=(float)(rand()%628)/100.f;
                p.vel={cosf(a)*60,sinf(a)*60};
                p.life=p.maxLife=0.5f; p.size=4; p.color=l.color;
                particles.push_back(p);
            }
        }
    }

    // Escape key pickup
    if(!hasEscapeKey&&!escapeKey.collected){
        if(v2len(v2sub(escapeKey.pos,player.pos))<22.f){
            escapeKey.collected=true; hasEscapeKey=true;
            SND(SND_LOOT_PICKUP,0.7f,0.7f);
        }
    }

    // Vault cracking (G key)
    {
        float distVault=v2len(v2sub(map.vaultPos,player.pos));
        vaultCracking=(keys[GLFW_KEY_G]&&distVault<50.f&&!vaultOpen);
        if(vaultCracking){
            vaultTimer+=dt;
            if(vaultTimer>=VAULT_CRACK_TIME){
                vaultOpen=true; vaultCracking=false;
                player.vaultCracked=true;
                SND(SND_VAULT_OPEN,0.9f);
                // Unlock vault loot
                for(int y=0;y<MAP_H;y++) for(int x=0;x<MAP_W;x++)
                    if(map.tiles[map.activeFloor][y][x]==TILE_VAULT_DOOR)
                        map.tiles[map.activeFloor][y][x]=TILE_FLOOR;
                map.vaultPos={-1000,-1000};
            }
        }else{
            vaultTimer=std::max(0.f,vaultTimer-dt*0.5f);
        }
    }

    // Escape condition: lobby spawn room
    if(hasEscapeKey&&player.vaultCracked){
        float distSpawn=v2len(v2sub(map.playerStart,player.pos));
        if(distSpawn<50.f){
            player.escaped=true;
            endRun(true,"");
        }
    }

    // Staircase/elevator interaction
    {
        int tx=(int)(player.pos.x/TILE_SIZE), ty=(int)(player.pos.y/TILE_SIZE);
        TileType t=map.getTile(tx,ty,map.activeFloor);
        if((t==TILE_STAIRS_UP||t==TILE_ELEVATOR)&&keys[GLFW_KEY_E]){
            map.setFloor(map.activeFloor+1);
        }else if(t==TILE_STAIRS_DOWN&&keys[GLFW_KEY_E]){
            map.setFloor(map.activeFloor-1);
        }
    }

    // Stealth score
    if(alarm.active) stealthBreachTimer+=dt;
}

// ─── Render ───────────────────────────────────────────────────────────────────
void GameState::renderRooms(Renderer& r){
    const ThemeDef& theme=THEME_DEFS[map.themeIdx];
    int f=map.activeFloor;

    // Draw tiles
    for(int ty=0;ty<MAP_H;ty++){
        for(int tx=0;tx<MAP_W;tx++){
            int t=map.tiles[f][ty][tx];
            if(t==TILE_EMPTY||t==TILE_WALL) continue;
            float wx=(float)tx*TILE_SIZE, wy=(float)ty*TILE_SIZE;
            Color col=theme.floorColor;
            if(t==TILE_VAULT_DOOR)  col={0.4f,0.4f,0.8f,1};
            else if(t==TILE_EXIT)   col={0.2f,0.8f,0.2f,1};
            else if(t==TILE_STAIRS_UP)  col={0.8f,0.6f,0.2f,1};
            else if(t==TILE_STAIRS_DOWN)col={0.6f,0.5f,0.2f,1};
            else if(t==TILE_ELEVATOR)   col={0.3f,0.5f,0.8f,1};
            r.drawQuad(wx,wy,TILE_SIZE,TILE_SIZE,col);
        }
    }
    // Draw walls
    for(int ty=0;ty<MAP_H;ty++){
        for(int tx=0;tx<MAP_W;tx++){
            int t=map.tiles[f][ty][tx];
            if(t!=TILE_WALL&&t!=TILE_EMPTY) continue;
            // Only draw wall if adjacent to floor
            bool adj=false;
            for(int d=0;d<4;d++){
                static const int DDX[]={0,0,1,-1}, DDY[]={1,-1,0,0};
                int nx2=tx+DDX[d], ny2=ty+DDY[d];
                if(nx2<0||ny2<0||nx2>=MAP_W||ny2>=MAP_H) continue;
                if(map.tiles[f][ny2][nx2]==TILE_FLOOR||
                   map.tiles[f][ny2][nx2]==TILE_EXIT) adj=true;
            }
            if(adj) r.drawQuad((float)tx*TILE_SIZE,(float)ty*TILE_SIZE,
                               TILE_SIZE,TILE_SIZE,theme.wallColor);
        }
    }

    // Draw room labels (faint)
    for(auto& rm:map.rooms){
        if(rm.floor!=f) continue;
        Vec2 c=map.roomCenter(rm);
        static const char* RLABELS[]={"LOBBY","","HALL","VAULT","SERVER","BARRACKS","STORAGE","STAIRS","ELEV"};
        if(rm.type==ROOM_LOBBY||rm.type==ROOM_VAULT||rm.type==ROOM_SERVER||
           rm.type==ROOM_BARRACKS||rm.type==ROOM_STORAGE||rm.type==ROOM_STAIRS||rm.type==ROOM_ELEVATOR_SHAFT){
            r.drawText(RLABELS[rm.type],c.x-20,c.y-6,1.1f,{0.8f,0.8f,0.8f,0.35f},false);
        }
    }
}

void GameState::renderPlaying(Renderer& r){
    renderRooms(r);

    // ── Server terminals ─────────────────────────────────────────────────
    for(auto& sv:map.servers){
        Color sc=sv.destroyed?C_DGRAY:C_CYAN;
        r.drawQuad(sv.pos.x-10,sv.pos.y-10,20,20,{0.1f,0.1f,0.2f,1});
        r.drawQuad(sv.pos.x-8,sv.pos.y-8,16,16,sc);
        r.drawText(sv.destroyed?"DEAD":"SRV",sv.pos.x-12,sv.pos.y-20,1.f,sc,false);
    }

    // ── Escape key ────────────────────────────────────────────────────────
    if(!escapeKey.collected){
        float pulse=0.5f+0.5f*sinf(runTime*4.f);
        r.drawCircle(escapeKey.pos.x,escapeKey.pos.y,14,{0.2f,0.9f,0.9f,0.3f+0.3f*pulse},false);
        r.drawCircle(escapeKey.pos.x,escapeKey.pos.y,8,{0.1f,0.8f,0.9f,pulse},false,12);
        r.drawText("KEY",escapeKey.pos.x-12,escapeKey.pos.y-24,1.2f,C_CYAN,false);
    }

    // ── Loot ─────────────────────────────────────────────────────────────
    for(auto& l:loot){
        if(l.collected) continue;
        r.drawCircle(l.pos.x,l.pos.y,10,l.color,false);
        r.drawText(l.label,l.pos.x-12,l.pos.y-22,1.0f,l.color,false);
    }

    // ── Bear traps ────────────────────────────────────────────────────────
    for(auto& bt:worldTraps){
        if(!bt.armed) continue;
        r.drawCircle(bt.pos.x,bt.pos.y,10,{0.5f,0.4f,0.2f,0.8f},false,6);
        r.drawText("T",bt.pos.x-4,bt.pos.y-5,1.2f,C_ORANGE,false);
    }

    // ── Cameras ───────────────────────────────────────────────────────────
    bool suspicious=(alarm.active&&alarm.level==1);
    float fovMult=suspicious?SUSP_CAM_FOV_MULT:1.f;
    for(auto& cam:cams){
        if(cam.disabled) continue;
        bool triggered=cam.triggered;
        Color cc=triggered?C_RED:(cam.disabled?C_DGRAY:C_YELLOW);
        float effectiveFOV=cam.sweepRange*fovMult;
        r.drawCone(cam.pos.x,cam.pos.y,cam.angle,effectiveFOV,120,
                   {cc.r,cc.g,cc.b,triggered?0.25f:0.12f});
        r.drawCircle(cam.pos.x,cam.pos.y,6,cc,false);
    }

    // ── Guards ───────────────────────────────────────────────────────────
    for(auto& g:guards){
        if(!g.alive||!g.visibleToPlayer) continue;
        float vs=g.state==GS_DEAD?0.6f:1.f;
        Color gc=g.isElite?C_GOLD:(g.isReinforcement?C_MAROON:C_BLUE);
        if(g.state==GS_SUSPICIOUS) gc=C_YELLOW;
        else if(g.state==GS_ALERT||g.state==GS_CHASE) gc=C_RED;
        else if(g.state==GS_DEAD) gc=C_DGRAY;

        // Vision cone
        if(g.alive&&g.state!=GS_DEAD){
            float visionFOV=g.visionAngle*(suspicious?SUSP_VISION_MULT:1.f);
            Color vc={gc.r,gc.g,gc.b,0.09f};
            r.drawCone(g.pos.x,g.pos.y,g.angle,visionFOV,g.visionRange,vc);
        }

        // Body
        r.drawCircle(g.pos.x,g.pos.y,10*vs,gc,false);
        // Facing arrow
        if(g.state!=GS_DEAD){
            Vec2 tip={g.pos.x+cosf(g.angle)*14.f,g.pos.y+sinf(g.angle)*14.f};
            r.drawQuad(g.pos.x-2,g.pos.y-2,tip.x-g.pos.x+2,tip.y-g.pos.y+2,{gc.r,gc.g,gc.b,0.8f});
        }
        // HP bar
        if(g.alive&&g.state!=GS_DEAD){
            float hpf=g.hp/g.maxHp;
            r.drawQuad(g.pos.x-12,g.pos.y+12,24,3,{0.1f,0.1f,0.1f,0.8f},0,0,1,1);
            r.drawQuad(g.pos.x-12,g.pos.y+12,24*hpf,3,hpf>0.5f?C_GREEN:C_RED,0,0,1,1);
        }
        // Label
        if(g.isElite) r.drawText("ELITE",g.pos.x-16,g.pos.y-24,1.1f,C_GOLD,false);
        // Suspicion meter
        if(g.state==GS_SUSPICIOUS&&g.suspicion>0){
            r.drawQuad(g.pos.x-12,g.pos.y-18,24,4,{0.1f,0.1f,0.1f,0.6f},0,0,1,1);
            r.drawQuad(g.pos.x-12,g.pos.y-18,24*g.suspicion,4,C_YELLOW,0,0,1,1);
        }
    }

    // ── Civilians ────────────────────────────────────────────────────────
    for(auto& civ:civilians){
        if(!civ.alive) continue;
        Color cc=(civ.state==CIV_PANIC)?C_ORANGE:C_LGRAY;
        r.drawCircle(civ.pos.x,civ.pos.y,8,cc,false,12);
        if(civ.state==CIV_PANIC) r.drawText("!",civ.pos.x-3,civ.pos.y-22,1.5f,C_ORANGE,false);
    }

    // ── Bullets ───────────────────────────────────────────────────────────
    for(auto& b:bullets){
        if(!b.alive) continue;
        r.drawCircle(b.pos.x,b.pos.y,3,b.color,false,6);
    }

    // ── Particles ─────────────────────────────────────────────────────────
    for(auto& p:particles){
        if(p.life<=0) continue;
        float a=p.life/p.maxLife;
        Color pc={p.color.r,p.color.g,p.color.b,p.color.a*a};
        r.drawCircle(p.pos.x,p.pos.y,p.size*a,pc,false,6);
    }

    // ── Player ────────────────────────────────────────────────────────────
    if(player.alive){
        Color ptint=C_WHITE;
        if(player.cloaked)    ptint={0.4f,0.4f,0.9f,0.5f};
        if(player.berserk)    ptint={0.9f,0.2f,0.1f,1.f};
        if(player.overcharge) ptint={1.f,0.6f,0.1f,1.f};
        if(player.adrenaline){
            float ap=0.6f+0.4f*sinf(runTime*8.f);
            ptint={1.f,0.3f*ap,0.3f*ap,1};
        }
        float ps=player.crouching?8.f:11.f;
        r.drawCircle(player.pos.x,player.pos.y,ps,ptint,false,12);
        // Facing arrow
        Vec2 tip={player.pos.x+cosf(player.angle)*18.f,player.pos.y+sinf(player.angle)*18.f};
        r.drawCircle(tip.x,tip.y,4,{1.f,0.9f,0.2f,0.9f},false,6);

        // HP bar
        float hpf=player.hp/player.maxHp;
        Color hc=hpf>0.5f?C_GREEN:(hpf>0.25f?C_YELLOW:C_RED);
        r.drawQuad(player.pos.x-14,player.pos.y+14,28,4,{0.1f,0.1f,0.1f,0.8f},0,0,1,1);
        r.drawQuad(player.pos.x-14,player.pos.y+14,28*hpf,4,hc,0,0,1,1);
    }

    // Vault crack bar
    if(vaultCracking&&!vaultOpen){
        float pct=vaultTimer/VAULT_CRACK_TIME;
        r.drawQuad(map.vaultPos.x-40,map.vaultPos.y-36,80,10,{0.1f,0.1f,0.15f,0.9f},0,0,1,1);
        r.drawQuad(map.vaultPos.x-40,map.vaultPos.y-36,80*pct,10,C_CYAN,0,0,1,1);
        r.drawText("CRACKING",map.vaultPos.x-30,map.vaultPos.y-52,1.4f,C_CYAN,false);
    }

    // Alarm overlay
    r.flush();
    if(alarm.active&&alarm.level>=2){
        float intensity=0.1f+0.07f*sinf(alarmFlash);
        r.drawQuadScreen(0,0,(float)SCREEN_W,(float)SCREEN_H,{0.9f,0.1f,0.1f,intensity});
    }else if(alarm.active&&alarm.level==1){
        float intensity=0.05f+0.04f*sinf(alarmFlash*0.5f);
        r.drawQuadScreen(0,0,(float)SCREEN_W,(float)SCREEN_H,{0.9f,0.8f,0.1f,intensity});
    }

    renderHUD(r);
    renderMinimap(r);
}

void GameState::renderHUD(Renderer& r){
    // HP bar
    r.drawQuadScreen(10,10,200,18,{0.1f,0.1f,0.1f,0.85f});
    float hpf=player.hp/player.maxHp;
    Color hc=hpf>0.5f?C_GREEN:(hpf>0.25f?C_YELLOW:C_RED);
    r.drawQuadScreen(10,10,200*hpf,18,hc);
    r.drawText("HP:"+std::to_string((int)player.hp)+"/"+std::to_string((int)player.maxHp),14,13,1.5f,C_WHITE);

    // Cash
    r.drawText("$"+std::to_string(runCash),10,35,2.f,C_YELLOW);

    // Weapon + ammo
    std::string ammoStr=player.reloading?"[RELOADING]":
                        std::to_string(player.currentAmmo)+"/"+
                        std::to_string(WEAPON_DEFS[player.weaponIdx].clipSize);
    r.drawText(WEAPON_DEFS[player.weaponIdx].name+" "+ammoStr,10,62,1.5f,WEAPON_DEFS[player.weaponIdx].color);

    // Items
    static const char* ISLOTS[]={"Z","X","V","B"};
    for(int i=0;i<(int)player.itemInventory.size()&&i<4;i++){
        int idx=player.itemInventory[i];
        if(idx>=(int)ITEM_DEFS.size()) continue;
        float x2=10.f+i*95.f;
        bool hasItem=player.itemCounts[idx]>0;
        r.drawQuadScreen(x2,82,88,26,{0.12f,0.12f,0.18f,hasItem?0.9f:0.4f});
        r.drawText(std::string(ISLOTS[i])+":"+ITEM_DEFS[idx].name.substr(0,7),x2+3,85,1.2f,
                   hasItem?ITEM_DEFS[idx].color:C_DGRAY);
        r.drawText("x"+std::to_string(player.itemCounts[idx]),x2+3,97,1.2f,C_LGRAY);
    }

    // Abilities
    static const char* ABKEYS[]={"1","2","3","4"};
    for(int i=0;i<(int)player.abilityLoadout.size()&&i<4;i++){
        int idx=player.abilityLoadout[i];
        if(idx>=(int)ABILITY_DEFS.size()) continue;
        float x2=10.f+i*125.f, y2=115.f;
        float cd=player.abilityCooldowns[i];
        r.drawQuadScreen(x2,y2,118,28,{0.1f,0.1f,0.2f,0.85f});
        r.drawText(std::string(ABKEYS[i])+":"+ABILITY_DEFS[idx].name.substr(0,8),
                   x2+3,y2+3,1.2f,cd>0?C_GRAY:ABILITY_DEFS[idx].color);
        if(cd>0){
            float pct=1.f-cd/ABILITY_DEFS[idx].cooldown;
            r.drawQuadScreen(x2,y2+22,118*pct,4,C_CYAN);
            r.drawText(std::to_string((int)cd+1)+"s",x2+90,y2+4,1.f,C_LGRAY);
        }
    }

    // Alarm
    if(alarm.active){
        static const char* ATXT[]={"","SUSPICIOUS","ALERT","LOCKDOWN"};
        Color ac=alarm.level==3?C_RED:(alarm.level==2?C_ORANGE:C_YELLOW);
        float midX=(float)SCREEN_W/2.f;
        r.drawQuadScreen(midX-140,5,280,30,{0,0,0,0.6f});
        r.drawText(ATXT[alarm.level],midX-70,8,2.2f,ac);
        r.drawText("Timer:"+std::to_string((int)alarm.timer)+"s",midX-55,28,1.5f,ac);
        if(alarm.level>=2)
            r.drawText("GUARDS KNOW YOUR POSITION!",midX-160,50,1.5f,C_RED);
        if(alarm.timer<5.f&&!reinforcementsSpawned)
            r.drawText("REINFORCEMENTS INCOMING!",midX-145,68,1.5f,C_ORANGE);
    }

    // Objectives
    float oy=(float)SCREEN_H-65;
    if(!vaultOpen)
        r.drawText("G = Crack Vault",SCREEN_W/2.f-80,oy,1.5f,C_LGRAY);
    else
        r.drawText("VAULT OPEN",SCREEN_W/2.f-55,oy,1.5f,C_GREEN);
    if(!hasEscapeKey)
        r.drawText("Find the ESCAPE KEY (cyan)",SCREEN_W/2.f-130,SCREEN_H-45,1.5f,C_CYAN);
    else if(!player.vaultCracked)
        r.drawText("KEY FOUND - Crack vault first!",SCREEN_W/2.f-155,SCREEN_H-45,1.5f,C_YELLOW);
    else
        r.drawText("Return to SPAWN ROOM to ESCAPE!",SCREEN_W/2.f-165,SCREEN_H-45,1.5f,C_GREEN);

    // Status flags
    std::string statStr="";
    if(player.cloaked) statStr+="CLOAKED ";
    if(player.crouching) statStr+="CROUCH ";
    if(player.berserk) statStr+="BERSERK ";
    if(player.overcharge) statStr+="OVERCHARGED ";
    if(player.disguised) statStr+="DISGUISED ";
    if(!statStr.empty()) r.drawText(statStr,SCREEN_W-350,30,1.4f,C_CYAN);

    // Floor indicator
    if(map.numFloors>1)
        r.drawText("Floor "+(std::to_string(map.activeFloor+1))+"/"+std::to_string(map.numFloors),
                   SCREEN_W-90,50,1.4f,C_LGRAY);

    // Run time
    r.drawText("T:"+std::to_string((int)runTime)+"s",SCREEN_W-80,10,1.5f,C_LGRAY);

    // Controls
    r.drawText("WASD=Move  LMB=Shoot  G=Vault  C=Crouch  R=Reload  1-4=Ability  Z/X/V/B=Item  E=Stairs  ESC=Pause",
               10,SCREEN_H-16,1.1f,{0.45f,0.45f,0.45f,1});
}

void GameState::renderMinimap(Renderer& r){
    const float MM_X=SCREEN_W-212, MM_Y=SCREEN_H-175;
    const float MM_W=200, MM_H=160;
    const float SX=MM_W/(MAP_W*TILE_SIZE), SY=MM_H/(MAP_H*TILE_SIZE);
    r.drawQuadScreen(MM_X-2,MM_Y-2,MM_W+4,MM_H+4,{0,0,0,0.7f});
    r.drawQuadScreen(MM_X,MM_Y,MM_W,MM_H,{0.04f,0.04f,0.06f,0.9f});
    int f=map.activeFloor;
    for(int ty=0;ty<MAP_H;ty++){
        for(int tx=0;tx<MAP_W;tx++){
            int t=map.tiles[f][ty][tx];
            if(t==TILE_EMPTY||t==TILE_WALL) continue;
            Color mc;
            switch(t){
            case TILE_FLOOR:      mc={0.3f,0.3f,0.3f,1}; break;
            case TILE_EXIT:       mc={0.2f,0.7f,0.2f,1}; break;
            case TILE_VAULT_DOOR: mc={0.4f,0.4f,0.8f,1}; break;
            case TILE_STAIRS_UP:
            case TILE_STAIRS_DOWN:mc={0.8f,0.6f,0.2f,1}; break;
            case TILE_ELEVATOR:   mc={0.3f,0.5f,0.9f,1}; break;
            default:              mc={0.32f,0.28f,0.24f,1}; break;
            }
            r.drawQuadScreen(MM_X+tx*TILE_SIZE*SX, MM_Y+ty*TILE_SIZE*SY,
                             TILE_SIZE*SX+0.5f, TILE_SIZE*SY+0.5f, mc);
        }
    }
    // Guards
    for(auto& g:guards){
        if(!g.alive) continue;
        float gx=MM_X+g.pos.x*SX, gy=MM_Y+g.pos.y*SY;
        Color gc=(g.state==GS_ALERT||g.state==GS_CHASE)?C_RED:
                 (g.state==GS_SUSPICIOUS?C_YELLOW:(g.isElite?C_GOLD:C_BLUE));
        r.drawQuadScreen(gx-2,gy-2,4,4,gc);
    }
    // Civilians
    for(auto& civ:civilians){
        if(!civ.alive) continue;
        float cx2=MM_X+civ.pos.x*SX, cy2=MM_Y+civ.pos.y*SY;
        r.drawQuadScreen(cx2-2,cy2-2,4,4,{0.8f,0.8f,0.8f,0.7f});
    }
    // Escape key
    if(!escapeKey.collected){
        float ekx=MM_X+escapeKey.pos.x*SX, eky=MM_Y+escapeKey.pos.y*SY;
        float pulse=0.5f+0.5f*sinf(runTime*4.f);
        r.drawQuadScreen(ekx-3,eky-3,6,6,{0.2f,0.9f,0.9f,pulse});
    }
    // Spawn
    if(hasEscapeKey&&player.vaultCracked){
        float sx2=MM_X+map.playerStart.x*SX, sy2=MM_Y+map.playerStart.y*SY;
        float p2=0.4f+0.6f*sinf(runTime*4.f);
        r.drawQuadScreen(sx2-4,sy2-4,8,8,{0.1f,0.9f,0.3f,p2});
    }
    // Player
    float px=MM_X+player.pos.x*SX, py=MM_Y+player.pos.y*SY;
    r.drawQuadScreen(px-3,py-3,6,6,C_YELLOW);
}

// ─── Menu screens ─────────────────────────────────────────────────────────────
void GameState::renderMenu(Renderer& r){
    r.drawQuadScreen(0,0,SCREEN_W,SCREEN_H,{0.04f,0.04f,0.06f,1});
    // Title
    r.drawText("HEIST",SCREEN_W/2.f-120,80,6.f,C_YELLOW);
    r.drawText("ROGUELIKE BANK ROBBERY",SCREEN_W/2.f-155,145,2.2f,C_LGRAY);
    r.drawText("Meta Level: "+std::to_string(meta.metaLevel)+"  |  XP: "+std::to_string(meta.metaXP)+"  |  Runs: "+std::to_string(meta.totalRuns),
               SCREEN_W/2.f-230,180,1.5f,C_CYAN);
    r.drawText("WARNING: Death resets ALL progress!",SCREEN_W/2.f-195,210,1.5f,C_RED);

    float btnX=SCREEN_W/2.f-120, btnY=260;
    drawButton(r,btnX,btnY,240,50,"SELECT MAP",false,{0.15f,0.25f,0.35f,1});
    if(buttonHit(btnX,btnY,240,50)){ SND(SND_CLICK,0.5f); screen=SCR_MAP_SELECT; }
    btnY+=65;
    drawButton(r,btnX,btnY,240,50,"LOADOUT",false,{0.15f,0.25f,0.2f,1});
    if(buttonHit(btnX,btnY,240,50)){ SND(SND_CLICK,0.5f); screen=SCR_LOADOUT; }
    btnY+=65;
    drawButton(r,btnX,btnY,240,50,"SKILL TREE",false,{0.2f,0.15f,0.28f,1});
    if(buttonHit(btnX,btnY,240,50)){ SND(SND_CLICK,0.5f); screen=SCR_SKILL_TREE; }
    btnY+=65;
    drawButton(r,btnX,btnY,240,50,"QUICK RUN",false,{0.2f,0.3f,0.15f,1});
    if(buttonHit(btnX,btnY,240,50)){
        SND(SND_CLICK,0.5f);
        selectedTheme=meta.unlockedThemes.empty()?0:meta.unlockedThemes[0];
        startRun();
    }

    // Best score
    r.drawText("Best Score: "+std::to_string(meta.bestScore),SCREEN_W/2.f-110,SCREEN_H-50,1.8f,C_GOLD);
    r.drawText("Total Cash: $"+std::to_string(meta.totalCash),SCREEN_W/2.f-120,SCREEN_H-28,1.5f,C_YELLOW);
}

void GameState::renderMapSelect(Renderer& r){
    r.drawQuadScreen(0,0,SCREEN_W,SCREEN_H,{0.04f,0.04f,0.07f,1});
    r.drawText("SELECT HEIST TARGET",SCREEN_W/2.f-165,30,3.f,C_WHITE);
    r.drawText("WARNING: Death resets ALL progress!",SCREEN_W/2.f-195,70,1.5f,C_RED);

    float startX=60, cardW=200, cardH=240, gap=20;
    int cols=3;
    for(int i=0;i<(int)THEME_DEFS.size();i++){
        auto& td=THEME_DEFS[i];
        bool unlocked=std::find(meta.unlockedThemes.begin(),meta.unlockedThemes.end(),i)!=meta.unlockedThemes.end();
        float cx=startX+(i%cols)*(cardW+gap);
        float cy=100+(i/cols)*(cardH+gap);
        bool sel=(selectedTheme==i);

        Color bg={0.1f,0.1f,0.14f,1};
        if(sel) bg={0.18f,0.18f,0.28f,1};
        if(!unlocked) bg={0.07f,0.07f,0.07f,1};

        r.drawQuadScreen(cx,cy,cardW,cardH,bg);
        r.drawQuadScreen(cx,cy,cardW,4,sel?C_YELLOW:td.accentColor);

        Color tc=unlocked?C_WHITE:C_DGRAY;
        r.drawText(td.name,cx+8,cy+12,1.6f,tc);
        if(!unlocked){
            r.drawText("LOCKED",cx+50,cy+100,2.f,C_DGRAY);
            r.drawText("Meta Lv "+std::to_string(td.unlockLevel),cx+40,cy+130,1.4f,C_DGRAY);
        }else{
            r.drawText(td.desc,cx+6,cy+40,1.1f,{0.7f,0.7f,0.7f,1});
            r.drawText("Diff: "+std::to_string((int)(td.difficultyMult*100))+"%",cx+6,cy+80,1.3f,
                       td.difficultyMult>2.f?C_RED:(td.difficultyMult>1.5f?C_ORANGE:C_GREEN));
            r.drawText("Loot: "+std::to_string((int)(td.lootMultiplier*100))+"%",cx+6,cy+100,1.3f,C_YELLOW);
            r.drawText("Guards: "+std::to_string(td.guardCount),cx+6,cy+120,1.3f,C_RED);
            r.drawText("Floors: "+std::to_string(td.maxFloors),cx+6,cy+140,1.3f,C_CYAN);
            r.drawText("Vaults: "+std::to_string(td.vaultRooms),cx+6,cy+160,1.3f,C_BLUE);
        }
        if(unlocked&&buttonHit(cx,cy,cardW,cardH)){
            SND(SND_CLICK,0.5f); selectedTheme=i;
        }
    }

    drawButton(r,SCREEN_W/2.f-180,SCREEN_H-80,240,55,"START HEIST",false,{0.15f,0.3f,0.15f,1});
    if(buttonHit(SCREEN_W/2.f-180,SCREEN_H-80,240,55)){
        SND(SND_CLICK,0.7f); startRun();
    }
    drawButton(r,SCREEN_W/2.f+80,SCREEN_H-80,160,55,"BACK",false);
    if(buttonHit(SCREEN_W/2.f+80,SCREEN_H-80,160,55)){ SND(SND_CLICK,0.4f); screen=SCR_MENU; }
}

void GameState::renderLoadout(Renderer& r){
    r.drawQuadScreen(0,0,SCREEN_W,SCREEN_H,{0.04f,0.04f,0.07f,1});
    r.drawText("LOADOUT",SCREEN_W/2.f-90,20,3.5f,C_WHITE);

    // Tabs
    static const char* TABS[]={"WEAPONS","ITEMS","ABILITIES"};
    for(int t=0;t<3;t++){
        float tx=60+t*220;
        drawButton(r,tx,70,200,36,TABS[t],loadoutTab==t,
                   loadoutTab==t?Color{0.2f,0.3f,0.5f,1}:Color{0.15f,0.15f,0.22f,1});
        if(buttonHit(tx,70,200,36)){ SND(SND_CLICK,0.3f); loadoutTab=t; }
    }

    float listY=125;
    if(loadoutTab==0){
        // Weapons
        for(int i=0;i<(int)meta.unlockedWeapons.size();i++){
            int wi=meta.unlockedWeapons[i];
            auto& w=WEAPON_DEFS[wi];
            bool sel=(loadoutWeapon==wi);
            float ry=listY+i*52;
            r.drawQuadScreen(60,ry,SCREEN_W-120,46,sel?Color{0.15f,0.25f,0.35f,1}:Color{0.1f,0.1f,0.15f,1});
            r.drawText(w.name,70,ry+4,1.8f,sel?C_YELLOW:w.color);
            r.drawText(w.desc,70,ry+24,1.3f,C_LGRAY);
            r.drawText("Dmg:"+std::to_string((int)w.damage)+" Rng:"+std::to_string((int)w.range)
                       +" Rate:"+std::to_string((int)(1.f/w.fireRate*10)/10.f),
                       SCREEN_W/2.f,ry+10,1.3f,C_GRAY);
            r.drawText(w.silenced?"SILENT":"LOUD",SCREEN_W-130,ry+14,1.3f,w.silenced?C_GREEN:C_ORANGE);
            if(buttonHit(60,ry,SCREEN_W-120,46)){ SND(SND_CLICK,0.4f); loadoutWeapon=wi; }
        }
    }else if(loadoutTab==1){
        // Items: pick up to 4
        r.drawText("Select up to 4 items:",60,listY,1.5f,C_LGRAY);
        listY+=25;
        for(int i=0;i<(int)meta.unlockedItems.size();i++){
            int ii=meta.unlockedItems[i];
            auto& item=ITEM_DEFS[ii];
            bool sel=std::find(loadoutItems.begin(),loadoutItems.end(),ii)!=loadoutItems.end();
            float ry=listY+i*46;
            r.drawQuadScreen(60,ry,SCREEN_W-120,40,sel?Color{0.12f,0.22f,0.15f,1}:Color{0.1f,0.1f,0.14f,1});
            r.drawText(item.name,70,ry+4,1.6f,sel?C_GREEN:item.color);
            r.drawText(item.desc,70,ry+22,1.2f,C_LGRAY);
            if(buttonHit(60,ry,SCREEN_W-120,40)){
                SND(SND_CLICK,0.3f);
                auto it=std::find(loadoutItems.begin(),loadoutItems.end(),ii);
                if(it!=loadoutItems.end()) loadoutItems.erase(it);
                else if((int)loadoutItems.size()<4) loadoutItems.push_back(ii);
            }
        }
    }else{
        // Abilities: pick up to 4
        r.drawText("Select up to 4 abilities:",60,listY,1.5f,C_LGRAY);
        listY+=25;
        for(int i=0;i<(int)meta.unlockedAbilities.size();i++){
            int ai=meta.unlockedAbilities[i];
            auto& ab=ABILITY_DEFS[ai];
            bool sel=std::find(loadoutAbilities.begin(),loadoutAbilities.end(),ai)!=loadoutAbilities.end();
            float ry=listY+i*46;
            r.drawQuadScreen(60,ry,SCREEN_W-120,40,sel?Color{0.15f,0.15f,0.28f,1}:Color{0.1f,0.1f,0.14f,1});
            r.drawText(ab.name,70,ry+4,1.6f,sel?C_CYAN:ab.color);
            r.drawText(ab.desc+" | CD:"+std::to_string((int)ab.cooldown)+"s",70,ry+22,1.2f,C_LGRAY);
            if(buttonHit(60,ry,SCREEN_W-120,40)){
                SND(SND_CLICK,0.3f);
                auto it=std::find(loadoutAbilities.begin(),loadoutAbilities.end(),ai);
                if(it!=loadoutAbilities.end()) loadoutAbilities.erase(it);
                else if((int)loadoutAbilities.size()<4) loadoutAbilities.push_back(ai);
            }
        }
    }

    drawButton(r,SCREEN_W-180,SCREEN_H-70,160,50,"BACK",false);
    if(buttonHit(SCREEN_W-180,SCREEN_H-70,160,50)){ SND(SND_CLICK,0.4f); screen=SCR_MENU; }
}

// ─── Skyrim-style skill tree ──────────────────────────────────────────────────
void GameState::renderSkillTree(Renderer& r){
    r.drawQuadScreen(0,0,SCREEN_W,SCREEN_H,{0.03f,0.03f,0.05f,1});
    r.drawText("SKILL TREE",SCREEN_W/2.f-100,15,3.f,C_WHITE);
    r.drawText("XP: "+std::to_string(meta.metaXP)+"  |  Meta Level: "+std::to_string(meta.metaLevel),
               SCREEN_W/2.f-160,55,1.8f,C_CYAN);

    // Tab bar
    static const char* STABS[]={"WEAPONS","ITEMS","ABILITIES","LEVELS","STATS"};
    for(int t=0;t<5;t++){
        float tx=20+t*175;
        Color tc=(skillTab==(SkillTreeTab)t)?Color{0.2f,0.2f,0.45f,1}:Color{0.12f,0.12f,0.18f,1};
        drawButton(r,tx,78,165,32,STABS[t],skillTab==(SkillTreeTab)t,tc);
        if(buttonHit(tx,78,165,32)){ SND(SND_CLICK,0.3f); skillTab=(SkillTreeTab)t; }
    }

    // Tree area
    const float TX=30, TY=120, TW=SCREEN_W-60, TH=SCREEN_H-170;
    r.drawQuadScreen(TX,TY,TW,TH,{0.06f,0.06f,0.09f,1});

    auto& nodes=getNodes(skillTab);

    // Draw connections first (lines from parent to child)
    for(int i=0;i<(int)nodes.size();i++){
        if(nodes[i].parentIdx<0) continue;
        auto& parent=nodes[nodes[i].parentIdx];
        auto& child=nodes[i];
        float px2=TX+parent.treeX*TW, py2=TY+parent.treeY*TH;
        float cx2=TX+child.treeX*TW,  cy2=TY+child.treeY*TH;
        bool connected=(parent.unlocked&&child.unlocked);
        bool available=(parent.unlocked&&!child.unlocked&&child.cost<=meta.metaXP);
        Color lc=connected?Color{0.4f,0.7f,0.4f,0.8f}:
                 (available?Color{0.7f,0.7f,0.2f,0.5f}:Color{0.2f,0.2f,0.2f,0.5f});
        // Draw a thick line as a series of quads
        Vec2 d={cx2-px2,cy2-py2};
        float len=v2len(d);
        if(len>0){
            Vec2 dn=v2norm(d);
            (void)dn; // direction used implicitly via d below
            int segs=(int)(len/6)+1;
            for(int s=0;s<segs;s++){
                float t=(float)s/segs;
                float nx2=px2+d.x*t-1, ny2=py2+d.y*t-1;
                r.drawQuadScreen(nx2,ny2,3,3,lc);
            }
        }
    }

    // Draw nodes
    const float NODE_R=22;
    for(int i=0;i<(int)nodes.size();i++){
        auto& n=nodes[i];
        float nx2=TX+n.treeX*TW, ny2=TY+n.treeY*TH;
        bool avail=canUnlockNode(skillTab,i);
        bool hover=(mouseX>=nx2-NODE_R&&mouseX<=nx2+NODE_R&&mouseY>=ny2-NODE_R&&mouseY<=ny2+NODE_R);

        // Node circle
        Color bc= n.unlocked?Color{0.2f,0.5f,0.2f,1}:
                  (avail?Color{0.4f,0.35f,0.1f,1}:Color{0.12f,0.12f,0.18f,1});
        if(hover&&avail){ bc.r+=0.15f; bc.g+=0.15f; }
        r.drawQuadScreen(nx2-NODE_R,ny2-NODE_R,NODE_R*2,NODE_R*2,bc);
        // Border
        Color border=n.unlocked?C_GREEN:(avail?C_YELLOW:C_DGRAY);
        r.drawQuadScreen(nx2-NODE_R,ny2-NODE_R,NODE_R*2,2,border);
        r.drawQuadScreen(nx2-NODE_R,ny2+NODE_R-2,NODE_R*2,2,border);
        r.drawQuadScreen(nx2-NODE_R,ny2-NODE_R,2,NODE_R*2,border);
        r.drawQuadScreen(nx2+NODE_R-2,ny2-NODE_R,2,NODE_R*2,border);

        // Text inside node
        float textScale=n.name.size()>8?1.0f:1.2f;
        float textW=n.name.size()*textScale*5.5f;
        r.drawText(n.name,nx2-textW*0.5f,ny2-10,textScale,n.unlocked?C_WHITE:C_LGRAY);
        if(!n.unlocked&&n.cost>0)
            r.drawText(std::to_string(n.cost)+"xp",nx2-14,ny2+6,1.1f,avail?C_YELLOW:C_GRAY);
        else if(n.unlocked)
            r.drawText("OWNED",nx2-16,ny2+6,1.0f,C_GREEN);

        // Tooltip on hover
        if(hover){
            float ttx=nx2+NODE_R+4, tty=ny2-30;
            if(ttx+200>SCREEN_W) ttx=nx2-NODE_R-204;
            r.drawQuadScreen(ttx,tty,200,70,{0.05f,0.05f,0.1f,0.95f});
            r.drawQuadScreen(ttx,tty,200,2,border);
            r.drawText(n.name,ttx+5,tty+6,1.4f,C_WHITE);
            r.drawText(n.desc,ttx+5,tty+26,1.1f,C_LGRAY);
            if(!n.unlocked) r.drawText("Cost: "+std::to_string(n.cost)+" XP",ttx+5,tty+46,1.2f,avail?C_YELLOW:C_RED);
            else r.drawText("Unlocked",ttx+5,tty+46,1.2f,C_GREEN);

            if(avail&&buttonHit((int)(nx2-NODE_R),(int)(ny2-NODE_R),(int)(NODE_R*2),(int)(NODE_R*2)))
                unlockNode(skillTab,i);
        }
    }

    // Back button
    drawButton(r,SCREEN_W-180,SCREEN_H-55,160,46,"BACK",false);
    if(buttonHit(SCREEN_W-180,SCREEN_H-55,160,46)){ SND(SND_CLICK,0.4f); screen=SCR_MENU; }
}

void GameState::renderResults(Renderer& r){
    r.drawQuadScreen(0,0,SCREEN_W,SCREEN_H,{0.03f,0.06f,0.03f,1});
    r.drawText("HEIST COMPLETE",SCREEN_W/2.f-200,50,4.5f,C_GREEN);
    r.drawText("Cash:    $"+std::to_string(runCash),200,170,2.5f,C_YELLOW);
    r.drawText("Score:   "+std::to_string(runScore),200,210,2.5f,C_WHITE);
    r.drawText("XP Earned: +"+std::to_string(xpEarned),200,250,2.5f,C_CYAN);
    r.drawText("Total XP: "+std::to_string(meta.metaXP),200,290,2.5f,C_GREEN);
    r.drawText("Ghost Run: "+(std::string)(player.detected?"No":"YES! +5000 bonus"),
               200,330,2.f,player.detected?C_LGRAY:C_CYAN);
    if(runScore>=meta.bestScore) r.drawText("NEW BEST SCORE!",200,370,2.5f,C_GOLD);
    r.drawText("Time: "+std::to_string((int)runTime)+"s",200,405,2.f,C_LGRAY);
    r.drawText("Meta Lv: "+std::to_string(meta.metaLevel),200,435,2.f,C_GREEN);
    drawButton(r,SCREEN_W/2.f-120,510,240,55,"CONTINUE",false,{0.15f,0.3f,0.5f,1});
    if(buttonHit(SCREEN_W/2.f-120,510,240,55)){ SND(SND_CLICK,0.5f); screen=SCR_MENU; }
}

void GameState::renderDeadScreen(Renderer& r){
    r.drawQuadScreen(0,0,SCREEN_W,SCREEN_H,{0.08f,0.02f,0.02f,1});
    r.drawText("YOU DIED",SCREEN_W/2.f-180,80,5.f,C_RED);
    r.drawText(deathReason,SCREEN_W/2.f-200,185,2.f,C_LGRAY);
    r.drawText("Cash collected: $"+std::to_string(runCash),200,250,2.5f,C_YELLOW);
    r.drawText("XP Earned: +"+std::to_string(xpEarned),200,290,2.5f,C_CYAN);
    r.drawQuadScreen(SCREEN_W/2.f-250,340,500,60,{0.2f,0.0f,0.0f,0.8f});
    r.drawText("ALL PROGRESS LOST!",SCREEN_W/2.f-170,350,2.5f,C_RED);
    r.drawText("Starting fresh...",SCREEN_W/2.f-130,375,1.8f,C_ORANGE);
    r.drawText("Every mistake is a lesson.",SCREEN_W/2.f-195,430,2.f,C_LGRAY);
    drawButton(r,SCREEN_W/2.f-120,490,240,55,"ACCEPT FATE",false,{0.35f,0.05f,0.05f,1});
    if(buttonHit(SCREEN_W/2.f-120,490,240,55)){
        SND(SND_CLICK,0.5f);
        resetMeta();
        screen=SCR_MENU;
    }
}

void GameState::renderPauseMenu(Renderer& r){
    r.drawQuadScreen(0,0,SCREEN_W,SCREEN_H,{0,0,0,0.55f});
    r.drawText("PAUSED",SCREEN_W/2.f-80,100,4.5f,C_WHITE);
    drawButton(r,SCREEN_W/2.f-120,220,240,55,"RESUME (ESC)",false);
    if(buttonHit(SCREEN_W/2.f-120,220,240,55)){ SND(SND_CLICK,0.4f); screen=SCR_PLAYING; }
    drawButton(r,SCREEN_W/2.f-120,295,240,55,"QUIT TO MENU",false,{0.28f,0.08f,0.08f,1});
    if(buttonHit(SCREEN_W/2.f-120,295,240,55)){
        SND(SND_CLICK,0.4f);
        endRun(false,"Abandoned run");
        screen=SCR_MENU;
    }
    // Volume
    r.drawText("Master Volume: "+std::to_string((int)(g_sound.masterVol*100))+"%",
               SCREEN_W/2.f-120,370,1.5f,C_LGRAY);
    drawButton(r,SCREEN_W/2.f-120,395,115,36,"-",false);
    if(buttonHit(SCREEN_W/2.f-120,395,115,36)){
        g_sound.masterVol=std::max(0.f,g_sound.masterVol-0.1f);
        for(int i=0;i<8;i++) alSourcef(g_sound.sources[i],AL_GAIN,g_sound.masterVol);
    }
    drawButton(r,SCREEN_W/2.f+5,395,115,36,"+",false);
    if(buttonHit(SCREEN_W/2.f+5,395,115,36)){
        g_sound.masterVol=std::min(1.f,g_sound.masterVol+0.1f);
        for(int i=0;i<8;i++) alSourcef(g_sound.sources[i],AL_GAIN,g_sound.masterVol);
    }
}

void GameState::render(Renderer& r){
    r.setCamera(camX,camY);
    switch(screen){
    case SCR_MENU:        r.setCamera(0,0); renderMenu(r);       break;
    case SCR_MAP_SELECT:  r.setCamera(0,0); renderMapSelect(r);  break;
    case SCR_LOADOUT:     r.setCamera(0,0); renderLoadout(r);    break;
    case SCR_SKILL_TREE:  r.setCamera(0,0); renderSkillTree(r);  break;
    case SCR_PLAYING:     renderPlaying(r);                       break;
    case SCR_PAUSE:       renderPlaying(r); r.flush(); r.setCamera(0,0); renderPauseMenu(r); break;
    case SCR_RESULTS:     r.setCamera(0,0); renderResults(r);    break;
    case SCR_DEAD:        r.setCamera(0,0); renderDeadScreen(r); break;
    }
    r.flush();
}