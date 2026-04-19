#pragma once
#include "game.h"
#include "map.h"
#include "sound.h"

class Player {
public:
    Vec2  pos;
    Vec2  vel;
    float angle;
    float hp, maxHp;
    float speed;
    float stealthRadius;
    bool  crouching;
    bool  ghostStep;
    bool  cloaked;
    float cloakTimer;
    bool  sprinting;
    float sprintTimer;
    bool  timeSlow;
    float timeSlowTimer;
    bool  adrenaline;
    float adrenalineTimer;
    bool  wallHack;
    float wallHackTimer;
    bool  dodgeRolling;
    float dodgeRollTimer;
    float dodgeAngle;
    bool  berserk;
    float berserkTimer;
    bool  meditating;
    float meditateTimer;
    float meditateTick;
    bool  disguised;
    float disguiseTimer;
    bool  nightVision;
    float nightVisionTimer;
    bool  overcharge;
    float overchargeTimer;

    // Trap ability
    std::vector<BearTrap> bearTraps;

    int   cash;
    bool  vaultCracked;
    bool  escaped;
    bool  alive;
    bool  detected;
    float footstepTimer;

    // Loadout
    int   weaponIdx;
    int   currentAmmo;   // current clip
    bool  reloading;
    float reloadTimer;
    std::vector<int> itemInventory;
    std::vector<int> abilityLoadout;
    std::vector<float> abilityCooldowns;
    int   itemSelected;
    std::vector<int> itemCounts;

    // Stats from meta
    float bonusMaxHp,bonusSpeed,bonusStealth,bonusLuck,cashMult;
    float bonusDamage,bonusReload,bonusClipSize,bonusCooldown;

    float shootCooldown;
    bool  prevMouseLeft;  // for single-shot click detection on non-auto

    void init(const MetaData& meta, int weapon, std::vector<int> items, std::vector<int> abilities);
    void update(float dt, const Map& map, bool keys[], double mx, double my,
                bool mouseLeft, bool mouseRight,
                std::vector<Bullet>& bullets, std::vector<Particle>& particles);
    void useItem(int slot, std::vector<Particle>& particles,
                 std::vector<SecurityCam>& cams, std::vector<Guard>& guards,
                 std::vector<Civilian>& civs);
    void useAbility(int slot);
    Rect getRect() const { return {pos.x-10,pos.y-10,20,20}; }
    void takeDamage(float dmg, std::vector<Particle>& particles);

    float noiseRadius() const;
    float effectiveDamage() const;

private:
    void move(float dt, const Map& map, bool keys[]);
    void shoot(float dt, bool firing, bool prevFiring,
               std::vector<Bullet>& bullets, std::vector<Particle>& particles);
    void resolveCollision(const Map& map);
};

// ─── Implementation ──────────────────────────────────────────────────────────

void Player::init(const MetaData& meta, int weapon,
                  std::vector<int> items, std::vector<int> abilities){
    bonusMaxHp    = meta.bonusMaxHP;
    bonusSpeed    = meta.bonusSpeed;
    bonusStealth  = meta.bonusStealth;
    bonusLuck     = meta.bonusLuck;
    cashMult      = meta.bonusCashMult;
    bonusDamage   = meta.bonusDamage;
    bonusReload   = meta.bonusReload;
    bonusClipSize = meta.bonusClipSize;
    bonusCooldown = meta.bonusCooldown;

    maxHp    = 120.f + bonusMaxHp;   // base 120 (more than before)
    hp       = maxHp;
    speed    = 155.f + bonusSpeed;
    stealthRadius = 120.f - bonusStealth;

    weaponIdx   = weapon;
    const WeaponDef& wd = WEAPON_DEFS[weaponIdx];
    currentAmmo = (int)(wd.clipSize * (1.f + bonusClipSize));
    reloading   = false; reloadTimer = 0;

    itemInventory  = items;
    abilityLoadout = abilities;
    float cdMult   = 1.f - bonusCooldown;
    abilityCooldowns.assign(abilities.size(), 0.f);
    itemCounts.assign(ITEM_DEFS.size(), 0);
    for(int i:items) if(i<(int)ITEM_DEFS.size())
        itemCounts[i] = ITEM_DEFS[i].defaultCount;

    cash=0; vaultCracked=false; escaped=false; alive=true; detected=false;
    crouching=false; ghostStep=false; cloaked=false; cloakTimer=0;
    sprinting=false; sprintTimer=0; timeSlow=false; timeSlowTimer=0;
    adrenaline=false; adrenalineTimer=0; wallHack=false; wallHackTimer=0;
    dodgeRolling=false; dodgeRollTimer=0; dodgeAngle=0;
    berserk=false; berserkTimer=0; meditating=false; meditateTimer=0;
    disguised=false; disguiseTimer=0; nightVision=false; nightVisionTimer=0;
    overcharge=false; overchargeTimer=0;
    shootCooldown=0; angle=0; vel={0,0}; itemSelected=0;
    prevMouseLeft=false; footstepTimer=0;
    bearTraps.clear();
}

float Player::noiseRadius() const {
    float r=stealthRadius;
    if(crouching) r*=0.5f;
    if(sprinting) r*=1.8f;
    if(ghostStep||berserk==false&&crouching) {}
    if(ghostStep)  r=0.f;
    if(berserk)    r*=2.f;
    return r;
}

float Player::effectiveDamage() const {
    float base = WEAPON_DEFS[weaponIdx].damage + bonusDamage;
    if(overcharge) base *= 1.75f;
    if(berserk)    base *= 2.0f;
    return base;
}

void Player::takeDamage(float dmg, std::vector<Particle>& particles){
    if(adrenaline||dodgeRolling||!alive) return;
    hp -= dmg;
    SND(SND_PLAYER_HURT, 0.7f);
    // Blood particles
    for(int i=0;i<8;i++){
        Particle p;
        p.pos=pos;
        float a=(float)(rand()%628)/100.f;
        p.vel={cosf(a)*(80+rand()%120),(float)(sinf(a)*(80+rand()%120))};
        p.life=p.maxLife=0.5f; p.size=3+rand()%4;
        p.color={0.8f,0.1f,0.1f,0.9f};
        particles.push_back(p);
    }
    if(hp<=0){ hp=0; alive=false; SND(SND_PLAYER_DEATH,0.9f); }
}

void Player::resolveCollision(const Map& map){
    float r=10.f;
    int tx=(int)(pos.x/TILE_SIZE), ty=(int)(pos.y/TILE_SIZE);
    for(int dy=-1;dy<=1;dy++){
        for(int dx=-1;dx<=1;dx++){
            int nx=tx+dx, ny=ty+dy;
            if(map.isSolid(nx,ny)){
                float wx=nx*TILE_SIZE, wy=ny*TILE_SIZE;
                float cx2=wx+TILE_SIZE/2.f, cy2=wy+TILE_SIZE/2.f;
                float overlapX=r+TILE_SIZE/2.f-fabsf(pos.x-cx2);
                float overlapY=r+TILE_SIZE/2.f-fabsf(pos.y-cy2);
                if(overlapX>0&&overlapY>0){
                    if(overlapX<overlapY) pos.x+=(pos.x<cx2?-overlapX:overlapX);
                    else                  pos.y+=(pos.y<cy2?-overlapY:overlapY);
                }
            }
        }
    }
}

void Player::move(float dt, const Map& map, bool keys[]){
    if(meditating) return; // can't move while meditating

    float spd=speed;
    if(crouching) spd*=0.45f;
    if(sprinting&&sprintTimer>0) spd*=2.0f;
    if(dodgeRolling) spd=speed*2.5f;
    if(berserk) spd*=1.5f;
    if(timeSlow){} // player unaffected

    Vec2 dir={0,0};
    if(keys[GLFW_KEY_W]||keys[GLFW_KEY_UP])    dir.y-=1;
    if(keys[GLFW_KEY_S]||keys[GLFW_KEY_DOWN])  dir.y+=1;
    if(keys[GLFW_KEY_A]||keys[GLFW_KEY_LEFT])  dir.x-=1;
    if(keys[GLFW_KEY_D]||keys[GLFW_KEY_RIGHT]) dir.x+=1;
    float len=v2len(dir);
    if(len>0){ dir.x/=len; dir.y/=len; }

    if(dodgeRolling) dir={cosf(dodgeAngle),sinf(dodgeAngle)};

    bool moving=(len>0||dodgeRolling);
    pos.x+=dir.x*spd*dt; resolveCollision(map);
    pos.y+=dir.y*spd*dt; resolveCollision(map);

    // Footstep sounds
    if(moving){
        footstepTimer-=dt;
        if(footstepTimer<=0){
            float interval=crouching?0.5f:0.25f;
            footstepTimer=interval;
            if(crouching) SND(SND_FOOTSTEP_CROUCH,0.2f);
            else SND(SND_FOOTSTEP,0.3f,0.9f+((float)rand()/RAND_MAX)*0.2f);
        }
    }
}

void Player::shoot(float dt, bool firing, bool prevFiring,
                   std::vector<Bullet>& bullets, std::vector<Particle>& particles){
    if(reloading){
        reloadTimer-=dt;
        if(reloadTimer<=0){
            reloading=false;
            const WeaponDef& w=WEAPON_DEFS[weaponIdx];
            currentAmmo=(int)(w.clipSize*(1.f+bonusClipSize));
            SND(SND_RELOAD,0.5f);
        }
        return;
    }

    shootCooldown-=dt;
    const WeaponDef& w=WEAPON_DEFS[weaponIdx];

    // Auto vs single-shot
    bool shouldFire=false;
    if(w.isAutomatic){ shouldFire=firing; }
    else             { shouldFire=(firing&&!prevFiring); }

    if(!shouldFire||shootCooldown>0||currentAmmo<=0) return;
    if(currentAmmo<=0){ reloading=true; reloadTimer=w.reloadTime*(1.f-bonusReload); return; }

    shootCooldown=w.fireRate;
    currentAmmo--;

    // Shotgun: multiple pellets
    int pellets=(w.name=="Shotgun")?6:1;
    float spreadBase=w.silenced?0.04f:0.12f;
    if(crouching) spreadBase*=0.5f;

    for(int p=0;p<pellets;p++){
        float ba=angle+(float)(rand()%200-100)/100.f*spreadBase*(pellets>1?2.5f:1.f);
        Bullet b{};
        b.pos={pos.x+cosf(ba)*16, pos.y+sinf(ba)*16};
        b.vel={cosf(ba)*w.bulletSpeed, sinf(ba)*w.bulletSpeed};
        b.damage=effectiveDamage()/(float)pellets*pellets; // per pellet
        if(pellets>1) b.damage=effectiveDamage()/pellets*2.f;
        b.range=w.range; b.alive=true; b.fromPlayer=true;
        b.color=w.color;
        b.penetrating=(w.name=="Railgun");
        b.explosive=(w.name=="Grenade Launcher");
        b.tranq=(w.name=="Tranq Rifle");
        bullets.push_back(b);
    }

    // Sound
    if(w.silenced) SND(SND_GUNSHOT_SILENCED,0.5f,0.9f+((float)rand()/RAND_MAX)*0.2f);
    else           SND(SND_GUNSHOT,0.8f,0.9f+((float)rand()/RAND_MAX)*0.2f);

    // Muzzle flash
    int flashCount=w.silenced?3:8;
    for(int i=0;i<flashCount;i++){
        Particle pt{};
        pt.pos={pos.x+cosf(angle)*16.f, pos.y+sinf(angle)*16.f};
        float pa=angle+(float)(rand()%200-100)/200.f*0.8f;
        float ps=150+rand()%200;
        pt.vel={cosf(pa)*ps,sinf(pa)*ps};
        pt.life=pt.maxLife=0.08f; pt.size=2+rand()%4;
        pt.color=w.color;
        particles.push_back(pt);
    }
    if(!w.silenced){
        for(int i=0;i<6;i++){
            Particle pt{};
            pt.pos={pos.x+cosf(angle)*16.f, pos.y+sinf(angle)*16.f};
            float pa=angle+(float)(rand()%200-100)/200.f*1.f;
            float ps=80+rand()%120;
            pt.vel={cosf(pa)*ps,sinf(pa)*ps};
            pt.life=pt.maxLife=0.12f; pt.size=4+rand()%5; pt.color=C_ORANGE;
            particles.push_back(pt);
        }
    }

    // Auto-reload when empty
    if(currentAmmo==0){ reloading=true; reloadTimer=w.reloadTime*(1.f-bonusReload); }
}

void Player::useItem(int slot, std::vector<Particle>& particles,
                     std::vector<SecurityCam>& cams, std::vector<Guard>& guards,
                     std::vector<Civilian>& civs){
    if(slot>=(int)itemInventory.size()) return;
    int idx=itemInventory[slot];
    if(idx>=(int)ITEM_DEFS.size()) return;
    if(itemCounts[idx]<=0) return;
    itemCounts[idx]--;
    const ItemDef& item=ITEM_DEFS[idx];

    if(item.name=="Medkit"){
        hp=std::min(hp+75.f,maxHp);
        SND(SND_LOOT_PICKUP,0.5f);
        // Heal particles
        for(int i=0;i<20;i++){
            Particle p{}; p.pos=pos;
            float a=(float)(rand()%628)/100.f;
            p.vel={cosf(a)*60,sinf(a)*60};
            p.life=p.maxLife=0.6f; p.size=4; p.color={0.2f,0.9f,0.3f,0.8f};
            particles.push_back(p);
        }
    }
    else if(item.name=="Smoke Bomb"){
        for(int i=0;i<80;i++){
            Particle p{}; p.pos=pos;
            float a=(float)(rand()%628)/100.f;
            float spd=30+rand()%100;
            p.vel={cosf(a)*spd,sinf(a)*spd};
            p.life=p.maxLife=4.f; p.size=22+rand()%20;
            p.color={0.55f,0.55f,0.55f,0.35f};
            particles.push_back(p);
        }
    }
    else if(item.name=="EMP Grenade"){
        SND(SND_EMP,0.8f);
        float empR=280.f;
        for(auto& c:cams){
            if(v2len(v2sub(c.pos,pos))<empR){ c.disabled=true; c.disabledTimer=15.f; }
        }
        for(auto& g:guards){
            if(!g.alive) continue;
            if(v2len(v2sub(g.pos,pos))<empR){
                g.state=GS_SUSPICIOUS; g.stateTimer=5.f;
            }
        }
        for(int i=0;i<50;i++){
            Particle p{}; p.pos=pos;
            float a=(float)(rand()%628)/100.f; float spd=40+rand()%200;
            p.vel={cosf(a)*spd,sinf(a)*spd};
            p.life=p.maxLife=0.6f; p.size=3+rand()%5; p.color=C_CYAN;
            particles.push_back(p);
        }
    }
    else if(item.name=="Flashbang"){
        SND(SND_EXPLOSION,0.5f,2.0f);
        float fbR=200.f;
        for(auto& g:guards){
            if(!g.alive) continue;
            if(v2len(v2sub(g.pos,pos))<fbR&&g.state!=GS_DEAD){
                g.stateTimer=4.f;
                g.state=GS_SUSPICIOUS;
            }
        }
        for(int i=0;i<60;i++){
            Particle p{}; p.pos=pos;
            float a=(float)(rand()%628)/100.f;
            p.vel={cosf(a)*(100+rand()%300),sinf(a)*(100+rand()%300)};
            p.life=p.maxLife=0.4f; p.size=5+rand()%8; p.color=C_WHITE;
            particles.push_back(p);
        }
    }
    else if(item.name=="Cloak Suit"){
        cloaked=true; cloakTimer=5.f;
    }
    else if(item.name=="Decoy"){
        // Distract nearest guards
        for(auto& g:guards){
            if(!g.alive) continue;
            Vec2 d=v2sub(pos,g.pos);
            if(v2len(d)<350.f){
                g.lastKnownPlayerPos={pos.x+(float)(rand()%100-50),pos.y+(float)(rand()%100-50)};
                g.hasLastKnown=true;
                if(g.state==GS_PATROL){ g.state=GS_INVESTIGATE; g.investigateTimer=8.f; }
            }
        }
    }
    else if(item.name=="Gas Canister"){
        for(int i=0;i<70;i++){
            Particle p{}; p.pos=pos;
            float a=(float)(rand()%628)/100.f; float spd=30+rand()%80;
            p.vel={cosf(a)*spd,sinf(a)*spd};
            p.life=p.maxLife=6.f; p.size=20+rand()%18; p.color={0.2f,0.8f,0.1f,0.3f};
            particles.push_back(p);
        }
    }
    else if(item.name=="Disguise Kit"){
        disguised=true; disguiseTimer=15.f;
    }
    else if(item.name=="Jammer"){
        // Suppress radio calls: set radioTimer on all guards
        for(auto& g:guards) g.radioTimer=30.f;
    }
    else if(item.name=="Night Vision"){
        nightVision=true; nightVisionTimer=20.f;
    }
    else if(item.name=="C4"){
        // Placed as explosive that player can detonate (handled in gamestate)
        // For simplicity: immediate large explosion
        SND(SND_EXPLOSION,1.f);
        for(int i=0;i<100;i++){
            Particle p{}; p.pos=pos;
            float a=(float)(rand()%628)/100.f; float spd=50+rand()%400;
            p.vel={cosf(a)*spd,sinf(a)*spd};
            p.life=p.maxLife=1.2f; p.size=6+rand()%10;
            p.color=(i%2)?C_ORANGE:C_RED;
            particles.push_back(p);
        }
    }
}

void Player::useAbility(int slot){
    if(slot>=(int)abilityLoadout.size()) return;
    if(abilityCooldowns[slot]>0) return;
    int idx=abilityLoadout[slot];
    if(idx>=(int)ABILITY_DEFS.size()) return;
    const AbilityDef& ab=ABILITY_DEFS[idx];
    float cdMult=1.f-bonusCooldown;
    abilityCooldowns[slot]=ab.cooldown*cdMult;

    SND(SND_CLICK,0.3f,1.5f);

    if(ab.name=="Dodge Roll"){
        dodgeRolling=true; dodgeRollTimer=0.25f; dodgeAngle=angle;
    }else if(ab.name=="Sprint"){
        sprinting=true; sprintTimer=3.f;
    }else if(ab.name=="Wall Hack"){
        wallHack=true; wallHackTimer=5.f;
    }else if(ab.name=="Time Slow"){
        timeSlow=true; timeSlowTimer=3.f;
    }else if(ab.name=="Adrenaline"){
        adrenaline=true; adrenalineTimer=2.f;
    }else if(ab.name=="Ghost Step"){
        ghostStep=true; wallHackTimer=8.f; // reuse timer
    }else if(ab.name=="Flashstep"){
        // Teleport 120px in facing direction
        Vec2 dest={pos.x+cosf(angle)*120.f, pos.y+sinf(angle)*120.f};
        // Check not in wall
        if(!bearTraps.empty()&&false){} // placeholder
        pos=dest; resolveCollision(*((Map*)nullptr)); // NOTE: caller should pass map
        // Actually, we'll do a simple non-map version and let collision snap
    }else if(ab.name=="Overcharge"){
        overcharge=true; overchargeTimer=5.f;
    }else if(ab.name=="Vanish"){
        // handled externally in gamestate (drops aggro)
        cloaked=true; cloakTimer=3.f;
    }else if(ab.name=="Meditate"){
        meditating=true; meditateTimer=4.f; meditateTick=0;
    }else if(ab.name=="Berserk"){
        berserk=true; berserkTimer=6.f;
    }else if(ab.name=="Trap"){
        BearTrap bt; bt.pos=pos; bt.armed=true; bt.triggered=false; bt.disarmTimer=0;
        bearTraps.push_back(bt);
    }
}

void Player::update(float dt, const Map& map, bool keys[], double mx, double my,
                    bool mouseLeft, bool mouseRight,
                    std::vector<Bullet>& bullets, std::vector<Particle>& particles){
    if(!alive) return;

    // Face mouse
    float dx=(float)mx-pos.x, dy=(float)my-pos.y;
    angle=atan2f(dy,dx);

    crouching=(keys[GLFW_KEY_LEFT_CONTROL]||keys[GLFW_KEY_C]);

    // Timers
    if(cloakTimer>0){    cloakTimer-=dt; if(cloakTimer<=0){cloaked=false;cloakTimer=0;} }
    if(sprintTimer>0){   sprintTimer-=dt; if(sprintTimer<=0)sprinting=false; }
    if(timeSlowTimer>0){ timeSlowTimer-=dt; if(timeSlowTimer<=0)timeSlow=false; }
    if(adrenalineTimer>0){adrenalineTimer-=dt;if(adrenalineTimer<=0)adrenaline=false;}
    if(wallHackTimer>0){ wallHackTimer-=dt;
        if(wallHackTimer<=0){wallHack=false;ghostStep=false;} }
    if(dodgeRollTimer>0){dodgeRollTimer-=dt;if(dodgeRollTimer<=0)dodgeRolling=false;}
    if(berserkTimer>0){  berserkTimer-=dt; if(berserkTimer<=0)berserk=false;}
    if(overchargeTimer>0){overchargeTimer-=dt;if(overchargeTimer<=0)overcharge=false;}
    if(disguiseTimer>0){ disguiseTimer-=dt; if(disguiseTimer<=0)disguised=false;}
    if(nightVisionTimer>0){nightVisionTimer-=dt;if(nightVisionTimer<=0)nightVision=false;}
    if(meditateTimer>0){
        meditateTimer-=dt;
        meditateTick+=dt;
        if(meditateTick>=0.5f){
            meditateTick=0;
            hp=std::min(hp+10.f,maxHp);
            for(int i=0;i<5;i++){
                Particle p{}; p.pos=pos;
                float a=(float)(rand()%628)/100.f;
                p.vel={cosf(a)*40,sinf(a)*40};
                p.life=p.maxLife=0.5f; p.size=4; p.color=C_GREEN;
                particles.push_back(p);
            }
        }
        if(meditateTimer<=0) meditating=false;
    }

    for(auto& cd:abilityCooldowns) if(cd>0) cd-=dt;

    move(dt,map,keys);
    shoot(dt,mouseLeft,prevMouseLeft,bullets,particles);
    prevMouseLeft=mouseLeft;

    // Manual reload: R key
    if(keys[GLFW_KEY_R]&&!reloading){
        const WeaponDef& w=WEAPON_DEFS[weaponIdx];
        if(currentAmmo<(int)(w.clipSize*(1.f+bonusClipSize))){
            reloading=true;
            reloadTimer=w.reloadTime*(1.f-bonusReload);
        }
    }
}