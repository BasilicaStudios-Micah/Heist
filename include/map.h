#pragma once
#include "game.h"

struct Room {
    int x,y,w,h;
    RoomType type;
    int floor;         // which floor (0=ground)
    bool isCircular;   // lobby and vault rooms drawn as ellipses
    int  sectorId;     // server sector for cameras in this room
};

// ─── A* Pathfinding ──────────────────────────────────────────────────────────
struct AStarNode {
    int x,y,g,f;
    int parentX,parentY;
    bool operator>(const AStarNode& o) const { return f>o.f; }
};

static std::vector<Vec2i> astarFind(const int tiles[MAP_H][MAP_W], Vec2i start, Vec2i goal){
    // Bounds check
    auto inBounds=[](int x,int y){ return x>=0&&y>=0&&x<MAP_W&&y<MAP_H; };
    auto solid=[&](int x,int y)->bool{
        if(!inBounds(x,y)) return true;
        int t=tiles[y][x];
        return t==TILE_WALL||t==TILE_EMPTY||t==TILE_VAULT_DOOR;
    };
    if(solid(start.x,start.y)||solid(goal.x,goal.y)) return {};

    // dist heuristic
    auto h=[&](int x,int y){ return abs(x-goal.x)+abs(y-goal.y); };

    std::priority_queue<AStarNode,std::vector<AStarNode>,std::greater<AStarNode>> open;
    // visited: g cost
    std::vector<std::vector<int>> gMap(MAP_H,std::vector<int>(MAP_W,99999));
    std::vector<std::vector<Vec2i>> parent(MAP_H,std::vector<Vec2i>(MAP_W,{-1,-1}));
    gMap[start.y][start.x]=0;
    open.push({start.x,start.y,0,h(start.x,start.y),start.x,start.y});

    static const int DX[]={0,0,1,-1,1,1,-1,-1};
    static const int DY[]={1,-1,0,0,1,-1,1,-1};
    static const int DC[]={10,10,10,10,14,14,14,14};

    while(!open.empty()){
        auto cur=open.top(); open.pop();
        int cx=cur.x,cy=cur.y;
        if(cx==goal.x&&cy==goal.y){
            // Reconstruct
            std::vector<Vec2i> path;
            int rx=cx,ry=cy;
            while(!(rx==start.x&&ry==start.y)){
                path.push_back({rx,ry});
                auto p=parent[ry][rx];
                rx=p.x; ry=p.y;
            }
            std::reverse(path.begin(),path.end());
            return path;
        }
        if(cur.g > gMap[cy][cx]) continue;
        for(int d=0;d<8;d++){
            int nx=cx+DX[d],ny=cy+DY[d];
            if(!inBounds(nx,ny)||solid(nx,ny)) continue;
            // Diagonal: check both cardinals aren't solid
            if(d>=4){
                if(solid(cx+DX[d],cy)||solid(cx,cy+DY[d])) continue;
            }
            int ng=gMap[cy][cx]+DC[d];
            if(ng<gMap[ny][nx]){
                gMap[ny][nx]=ng;
                parent[ny][nx]={cx,cy};
                open.push({nx,ny,ng,ng+h(nx,ny)*10,cx,cy});
            }
        }
    }
    return {}; // no path
}

// ─── Map class ────────────────────────────────────────────────────────────────
class Map {
public:
    // Multi-floor: store tiles per floor
    int  tiles[MAX_FLOORS][MAP_H][MAP_W];
    int  activeFloor = 0;
    int  numFloors   = 1;

    // Convenience: current floor tiles pointer
    int (*curTiles)[MAP_W] = tiles[0];

    std::vector<Room> rooms;
    Vec2 playerStart;
    Vec2 vaultPos;
    Vec2 exitPos;
    int  themeIdx = 0;
    std::mt19937 rng;

    // Server sectors: sectorId -> list of server terminal indices
    std::vector<ServerTerminal> servers;
    int nextSectorId = 0;

    void generate(int seed, int themeIdx, int metaLevel);
    bool isSolid(int tx, int ty, int floor=-1) const;
    bool isSolid(float wx, float wy) const {
        return isSolid((int)(wx/TILE_SIZE),(int)(wy/TILE_SIZE));
    }
    bool lineOfSight(Vec2 a, Vec2 b) const;
    TileType getTile(int x,int y,int floor=-1) const;
    void setTile(int x,int y,int t,int floor=-1);

    std::vector<PatrolPoint> makePatrolPath(const Room& r, int count=4);
    std::vector<LootItem>    makeLoot(int metaLevel, float luckBonus);
    std::vector<SecurityCam> makeCameras(int count);
    std::vector<Guard>       makeGuards(int count, int metaLevel);
    std::vector<Civilian>    makeCivilians(int count);
    EscapeKey                makeEscapeKey();

    // Pathfinding wrapper
    std::vector<Vec2i> findPath(Vec2i start, Vec2i goal) const {
        return astarFind(tiles[activeFloor], start, goal);
    }

    void setFloor(int f){
        if(f<0||f>=numFloors) return;
        activeFloor=f;
        curTiles=tiles[f];
    }

    // Public roomCenter — needed by GameState for spawning and rendering
    Vec2 roomCenter(const Room& r) const {
        return { (r.x + r.w/2.f)*TILE_SIZE, (r.y + r.h/2.f)*TILE_SIZE };
    }

private:
    void fillFloor(int floor, int t);
    void carveRoom(const Room& r, int floor);
    void carveCorridor(int x1,int y1,int x2,int y2, int floor);
    Room makeRoom(RoomType type, int attempt, int floor);
    bool roomOverlaps(const Room& r) const;
    void placeStairsOrElevator(int floorA, int floorB, const Room& rA, const Room& rB) {}
};

// ─── Implementations ──────────────────────────────────────────────────────────

bool Map::isSolid(int tx,int ty,int floor) const {
    if(floor<0) floor=activeFloor;
    if(tx<0||ty<0||tx>=MAP_W||ty>=MAP_H) return true;
    int t=tiles[floor][ty][tx];
    return t==TILE_WALL||t==TILE_EMPTY||t==TILE_VAULT_DOOR;
}
TileType Map::getTile(int x,int y,int floor) const {
    if(floor<0) floor=activeFloor;
    if(x<0||y<0||x>=MAP_W||y>=MAP_H) return TILE_WALL;
    return (TileType)tiles[floor][y][x];
}
void Map::setTile(int x,int y,int t,int floor){
    if(floor<0) floor=activeFloor;
    if(x<0||y<0||x>=MAP_W||y>=MAP_H) return;
    tiles[floor][y][x]=t;
}
void Map::fillFloor(int floor,int t){
    for(int y=0;y<MAP_H;y++) for(int x=0;x<MAP_W;x++) tiles[floor][y][x]=t;
}
void Map::carveRoom(const Room& r, int floor){
    for(int y=r.y;y<r.y+r.h;y++)
        for(int x=r.x;x<r.x+r.w;x++)
            tiles[floor][y][x]=TILE_FLOOR;
}
void Map::carveCorridor(int x1,int y1,int x2,int y2,int floor){
    // L-shaped corridor (no doors)
    int mx=(x1<x2)?x1:x2, Mx=(x1>x2)?x1:x2;
    for(int x=mx;x<=Mx;x++){
        int t=tiles[floor][y1][x];
        if(t==TILE_WALL||t==TILE_EMPTY) tiles[floor][y1][x]=TILE_FLOOR;
    }
    int my=(y1<y2)?y1:y2, My=(y1>y2)?y1:y2;
    for(int y=my;y<=My;y++){
        int t=tiles[floor][y][x2];
        if(t==TILE_WALL||t==TILE_EMPTY) tiles[floor][y][x2]=TILE_FLOOR;
    }
}
bool Map::roomOverlaps(const Room& r) const {
    for(auto& e:rooms){
        if(e.floor!=r.floor) continue;
        if(r.x-2<e.x+e.w && r.x+r.w+2>e.x && r.y-2<e.y+e.h && r.y+r.h+2>e.y) return true;
    }
    return false;
}
Room Map::makeRoom(RoomType type, int attempt, int floor){
    Room r;
    r.type=type; r.floor=floor; r.isCircular=false; r.sectorId=-1;
    switch(type){
    case ROOM_LOBBY:
        r.w=12; r.h=10; r.isCircular=true;
        break;
    case ROOM_VAULT:
        r.w=10; r.h=10; r.isCircular=true;
        break;
    case ROOM_HALLWAY:{
        bool horiz = (rng()%2==0);
        r.w = horiz ? (12+rng()%10) : (4+rng()%3);
        r.h = horiz ? (3+rng()%2)   : (12+rng()%10);
        break;
    }
    case ROOM_SERVER:
        r.w=8+rng()%4; r.h=7+rng()%3;
        break;
    case ROOM_BARRACKS:
        r.w=10+rng()%4; r.h=8+rng()%3;
        break;
    case ROOM_STORAGE:
        r.w=8+rng()%5; r.h=7+rng()%4;
        break;
    case ROOM_STAIRS:
        r.w=5; r.h=5;
        break;
    case ROOM_ELEVATOR_SHAFT:
        r.w=5; r.h=5;
        break;
    default:
        r.w=7+rng()%7; r.h=5+rng()%6;
    }
    std::uniform_int_distribution<int> xd(2,MAP_W-r.w-3), yd(2,MAP_H-r.h-3);
    r.x=xd(rng); r.y=yd(rng);
    return r;
}

void Map::generate(int seed, int theme, int metaLevel){
    themeIdx=theme;
    rng.seed(seed);
    rooms.clear();
    servers.clear();
    nextSectorId=0;

    const ThemeDef& td=THEME_DEFS[theme];
    numFloors=td.maxFloors;
    for(int f=0;f<MAX_FLOORS;f++) fillFloor(f,TILE_WALL);
    activeFloor=0; curTiles=tiles[0];

    // ── Build floor 0 (ground) ───────────────────────────────────────────
    auto tryPlace=[&](RoomType type, int floor, int maxAttempts=120)->bool{
        for(int a=0;a<maxAttempts;a++){
            Room r=makeRoom(type,a,floor);
            if(!roomOverlaps(r)){
                carveRoom(r,floor);
                rooms.push_back(r);
                return true;
            }
        }
        return false;
    };

    // Lobby always first
    tryPlace(ROOM_LOBBY,0);
    rooms[0].sectorId=0;

    // Required rooms by theme
    int basicCount=td.targetRooms+metaLevel/4;
    basicCount=std::min(basicCount,16);

    // Hallways
    int hallCount=basicCount/3+1;
    for(int i=0;i<hallCount;i++) tryPlace(ROOM_HALLWAY,0);

    // Basic rooms
    for(int i=0;i<basicCount;i++) tryPlace(ROOM_BASIC,0);

    // Server rooms
    for(int i=0;i<td.serverRooms;i++){
        if(tryPlace(ROOM_SERVER,0)){
            Room& sr=rooms.back();
            sr.sectorId=nextSectorId;
            // Place server terminal in center
            Vec2 c=roomCenter(sr);
            ServerTerminal st;
            st.pos={c.x,c.y};
            st.destroyed=false;
            st.sectorId=nextSectorId;
            st.hp=80.f;
            servers.push_back(st);
            nextSectorId++;
        }
    }

    // Storage rooms
    for(int i=0;i<td.storageRooms;i++) tryPlace(ROOM_STORAGE,0);

    // Vault rooms
    for(int i=0;i<td.vaultRooms;i++) tryPlace(ROOM_VAULT,0);

    // Barracks
    if(td.hasBarracks){
        tryPlace(ROOM_BARRACKS,0);
        if(numFloors>1) tryPlace(ROOM_BARRACKS,0);
    }

    // ── Build upper floors ───────────────────────────────────────────────
    for(int f=1;f<numFloors;f++){
        int upperBasic=basicCount/2+2;
        for(int i=0;i<upperBasic;i++) tryPlace(ROOM_BASIC,f);
        for(int i=0;i<hallCount/2+1;i++) tryPlace(ROOM_HALLWAY,f);
        if(td.hasBarracks && f<numFloors-1) tryPlace(ROOM_BARRACKS,f);
        for(int i=0;i<td.vaultRooms;i++) tryPlace(ROOM_VAULT,f);
        for(int i=0;i<td.serverRooms;i++){
            if(tryPlace(ROOM_SERVER,f)){
                Room& sr=rooms.back();
                sr.sectorId=nextSectorId;
                Vec2 c=roomCenter(sr);
                ServerTerminal st; st.pos=c; st.destroyed=false;
                st.sectorId=nextSectorId; st.hp=80.f;
                servers.push_back(st);
                nextSectorId++;
            }
        }
    }

    // ── Connect rooms per floor with corridors ───────────────────────────
    for(int f=0;f<numFloors;f++){
        std::vector<int> floorRooms;
        for(int i=0;i<(int)rooms.size();i++)
            if(rooms[i].floor==f) floorRooms.push_back(i);
        for(int i=1;i<(int)floorRooms.size();i++){
            int a=floorRooms[i-1], b=floorRooms[i];
            Vec2 ca=roomCenter(rooms[a]), cb=roomCenter(rooms[b]);
            carveCorridor(ca.x/TILE_SIZE,ca.y/TILE_SIZE,
                          cb.x/TILE_SIZE,cb.y/TILE_SIZE,f);
        }
        // Extra loops
        if(floorRooms.size()>2){
            std::uniform_int_distribution<int> ri(0,(int)floorRooms.size()-1);
            for(int i=0;i<3;i++){
                int a=floorRooms[ri(rng)], b=floorRooms[ri(rng)];
                if(a==b) continue;
                Vec2 ca=roomCenter(rooms[a]), cb=roomCenter(rooms[b]);
                carveCorridor(ca.x/TILE_SIZE,ca.y/TILE_SIZE,
                              cb.x/TILE_SIZE,cb.y/TILE_SIZE,f);
            }
        }
    }

    // ── Stairs/elevator between floors ──────────────────────────────────
    if(numFloors>1){
        for(int f=0;f<numFloors-1;f++){
            // Find a basic room on each floor
            Room* rA=nullptr; Room* rB=nullptr;
            for(auto& r:rooms) if(r.floor==f   &&r.type==ROOM_BASIC){rA=&r;break;}
            for(auto& r:rooms) if(r.floor==f+1 &&r.type==ROOM_BASIC){rB=&r;break;}
            if(rA&&rB){
                Vec2 ca=roomCenter(*rA), cb=roomCenter(*rB);
                // Stairs marker on both floors at the room centers
                if(td.hasElevator){
                    int ex=ca.x/TILE_SIZE, ey=ca.y/TILE_SIZE;
                    tiles[f][ey][ex]=TILE_ELEVATOR;
                    tiles[f+1][ey][ex]=TILE_ELEVATOR;
                }else{
                    int ax=ca.x/TILE_SIZE, ay=ca.y/TILE_SIZE;
                    int bx=cb.x/TILE_SIZE, by=cb.y/TILE_SIZE;
                    tiles[f][ay][ax]=TILE_STAIRS_UP;
                    tiles[f+1][by][bx]=TILE_STAIRS_DOWN;
                }
            }
        }
    }

    // ── Designate lobby (first room) as player start ──────────────────
    Vec2 c0=roomCenter(rooms[0]);
    playerStart=c0;

    // ── Designate vault rooms ─────────────────────────────────────────
    bool firstVault=true;
    for(auto& r:rooms){
        if(r.type==ROOM_VAULT){
            Vec2 cv=roomCenter(r);
            if(firstVault){ vaultPos=cv; firstVault=false; }
            int vx=cv.x/TILE_SIZE, vy=cv.y/TILE_SIZE;
            tiles[r.floor][vy][vx]=TILE_VAULT_DOOR;

            // Exit near first vault
            if(r.floor==0){
                int ex=vx+2, ey=vy;
                if(ex<MAP_W && tiles[r.floor][ey][ex]==TILE_FLOOR){
                    tiles[r.floor][ey][ex]=TILE_EXIT;
                    exitPos={(float)ex*TILE_SIZE+TILE_SIZE/2.f,
                              (float)ey*TILE_SIZE+TILE_SIZE/2.f};
                }else{
                    exitPos=playerStart;
                }
            }
        }
    }
    if(firstVault) exitPos=playerStart; // fallback

    // ── Loot tiles in storage and basic rooms ─────────────────────────
    for(auto& r:rooms){
        if(r.type==ROOM_STORAGE || r.type==ROOM_BASIC){
            int loot_count=(r.type==ROOM_STORAGE)?(3+rng()%8):1;
            for(int l=0;l<loot_count;l++){
                std::uniform_int_distribution<int>
                    lx(r.x+1,r.x+r.w-2), ly(r.y+1,r.y+r.h-2);
                int x=lx(rng), y=ly(rng);
                if(tiles[r.floor][y][x]==TILE_FLOOR)
                    tiles[r.floor][y][x]=TILE_LOOT;
            }
        }
    }

    // ── Assign camera sectors to rooms ───────────────────────────────
    // If only 1 server, all cams are sector 0
    // Otherwise each room gets the sector of nearest server
    for(auto& r:rooms){
        if(r.sectorId<0 && !servers.empty()){
            Vec2 rc=roomCenter(r);
            float best=1e9; int bestSec=-1;
            for(auto& sv:servers){
                float d=v2len(v2sub(sv.pos,rc));
                if(d<best){ best=d; bestSec=sv.sectorId; }
            }
            r.sectorId=bestSec;
        }
    }
}

bool Map::lineOfSight(Vec2 a, Vec2 b) const {
    Vec2 d=v2sub(b,a);
    float dist=v2len(d);
    if(dist<1) return true;
    Vec2 step=v2scale(v2norm(d),(float)TILE_SIZE*0.45f);
    int steps=(int)(dist/(TILE_SIZE*0.45f))+1;
    Vec2 p=a;
    for(int i=0;i<steps;i++){
        if(isSolid((int)(p.x/TILE_SIZE),(int)(p.y/TILE_SIZE),activeFloor)) return false;
        p=v2add(p,step);
    }
    return true;
}

std::vector<PatrolPoint> Map::makePatrolPath(const Room& r,int count){
    std::vector<PatrolPoint> pts;
    std::uniform_real_distribution<float>
        ux(r.x+1.f,r.x+r.w-2.f), uy(r.y+1.f,r.y+r.h-2.f);
    for(int i=0;i<count;i++){
        float x=ux(rng)*TILE_SIZE+TILE_SIZE/2.f;
        float y=uy(rng)*TILE_SIZE+TILE_SIZE/2.f;
        pts.push_back({{x,y}});
    }
    return pts;
}

std::vector<LootItem> Map::makeLoot(int metaLevel, float luckBonus){
    std::vector<LootItem> loot;
    const ThemeDef& theme=THEME_DEFS[themeIdx];
    std::uniform_int_distribution<int> vd(200,800);
    std::uniform_real_distribution<float> lucky(0,1);
    static const char* LOOT_LABELS[]={"Cash","Jewels","Bonds","Gold","Artifact","Painting","Coins","Bearer Bond","Diamond"};
    static Color LOOT_COLORS[]={C_YELLOW,C_CYAN,{0.9f,0.9f,0.9f,1},C_GOLD,C_PURPLE,C_ORANGE,C_YELLOW,{0.8f,0.9f,0.8f,1},C_CYAN};
    int nc=9;
    for(int f=0;f<numFloors;f++){
        for(int y=0;y<MAP_H;y++){
            for(int x=0;x<MAP_W;x++){
                if(tiles[f][y][x]==TILE_LOOT){
                    int val=(int)(vd(rng)*theme.lootMultiplier*(1.f+luckBonus+metaLevel*0.05f));
                    bool bonus=lucky(rng)<(0.15f+luckBonus);
                    int li=rng()%nc;
                    loot.push_back({{x*(float)TILE_SIZE+TILE_SIZE/2.f,
                                     y*(float)TILE_SIZE+TILE_SIZE/2.f},
                                    val,false,LOOT_COLORS[li],LOOT_LABELS[li]});
                    if(bonus){
                        val=(int)(val*1.5f);
                        loot.push_back({{x*(float)TILE_SIZE+TILE_SIZE/2.f+16,
                                         y*(float)TILE_SIZE+TILE_SIZE/2.f},
                                        val,false,C_CYAN,"Vault Stash"});
                    }
                    tiles[f][y][x]=TILE_FLOOR;
                }
            }
        }
    }
    return loot;
}

std::vector<SecurityCam> Map::makeCameras(int count){
    std::vector<SecurityCam> cams;
    std::uniform_real_distribution<float> angd(0,2*(float)M_PI);

    // First: fixed vault cameras (one per vault room, points at vault door center)
    for(auto& r:rooms){
        if(r.type==ROOM_VAULT){
            Vec2 c=roomCenter(r);
            // Cam in corner pointing at vault door
            float cx2=c.x+r.w*TILE_SIZE*0.4f, cy2=c.y-r.h*TILE_SIZE*0.4f;
            float angle=atan2f(c.y-cy2,c.x-cx2);
            // Find sector for this room
            int sec=r.sectorId;
            cams.push_back({c, angle, 0.f, 0.3f, false,false,0,sec,true});
        }
    }

    // Then: sweeping cameras in server rooms, barracks, and basic rooms
    int placed=0;
    for(auto& r:rooms){
        if(placed>=count) break;
        if(r.type==ROOM_LOBBY||r.type==ROOM_VAULT||r.type==ROOM_HALLWAY||r.type==ROOM_STAIRS) continue;
        Vec2 c=roomCenter(r);
        int sec=r.sectorId>=0?r.sectorId:0;
        cams.push_back({c,angd(rng),0.8f,(float)M_PI*0.5f,false,false,0,sec,false});
        placed++;
    }
    return cams;
}

std::vector<Guard> Map::makeGuards(int count, int metaLevel){
    std::vector<Guard> guards;
    const ThemeDef& td=THEME_DEFS[themeIdx];
    count=std::max(count,td.guardCount);
    float diffMult=td.difficultyMult*(1.f+metaLevel*0.05f);
    int gi=0;
    for(auto& r:rooms){
        if((int)guards.size()>=count) break;
        if(r.type==ROOM_LOBBY) continue; // no guards in lobby

        // Barracks gets elite guards
        bool elite=(r.type==ROOM_BARRACKS);
        int perRoom=(elite)?4:1;

        for(int p=0;p<perRoom&&(int)guards.size()<count;p++){
            auto patrol=makePatrolPath(r,4);
            Vec2 startPos=patrol.empty()?roomCenter(r):patrol[0].pos;
            Guard g{};
            g.pos=startPos;
            g.angle=(float)(rng()%628)/100.f;
            g.maxHp=(elite?150.f:50.f)*diffMult;
            g.hp=g.maxHp;
            g.state=GS_PATROL;
            g.patrol=patrol;
            g.alive=true;
            g.visionRange=200.f*(1.f+metaLevel*0.02f);
            g.visionAngle=(float)M_PI*0.55f;
            g.weaponIdx=std::min((int)(metaLevel/5),(int)WEAPON_DEFS.size()-1);
            g.isElite=elite;
            g.isReinforcement=false;
            g.hearingRange=180.f*(1.f+metaLevel*0.02f);
            g.meleeCooldown=0;
            g.meleeDmg=elite?60.f:30.f;
            g.pathIdx=0;
            g.pathTimer=0;
            guards.push_back(g);
            gi++;
        }
    }
    // Fill remaining slots from non-lobby rooms
    if((int)guards.size()<count){
        for(auto& r:rooms){
            if((int)guards.size()>=count) break;
            if(r.type==ROOM_LOBBY||r.type==ROOM_VAULT) continue;
            auto patrol=makePatrolPath(r,4);
            Vec2 sp=patrol.empty()?roomCenter(r):patrol[0].pos;
            Guard g{};
            g.pos=sp; g.angle=0; g.maxHp=50.f*diffMult; g.hp=g.maxHp;
            g.state=GS_PATROL; g.patrol=patrol; g.alive=true;
            g.visionRange=200.f*(1.f+metaLevel*0.02f);
            g.visionAngle=(float)M_PI*0.55f;
            g.weaponIdx=std::min((int)(metaLevel/5),(int)WEAPON_DEFS.size()-1);
            g.isElite=false; g.isReinforcement=false;
            g.hearingRange=180.f*(1.f+metaLevel*0.02f);
            g.meleeCooldown=0; g.meleeDmg=30.f;
            g.pathIdx=0; g.pathTimer=0;
            guards.push_back(g);
        }
    }
    return guards;
}

std::vector<Civilian> Map::makeCivilians(int count){
    std::vector<Civilian> civs;
    // Only lobby and basic rooms have civilians
    std::vector<Room*> civRooms;
    for(auto& r:rooms)
        if((r.type==ROOM_LOBBY||r.type==ROOM_BASIC)&&r.floor==0)
            civRooms.push_back(&r);
    if(civRooms.empty()) return civs;
    std::uniform_real_distribution<float> rv(0,1);
    for(int i=0;i<count;i++){
        Room* r=civRooms[rng()%civRooms.size()];
        std::uniform_real_distribution<float>
            cx(r->x+1.f,r->x+r->w-2.f), cy(r->y+1.f,r->y+r->h-2.f);
        Civilian civ{};
        civ.pos={(cx(rng)*TILE_SIZE+TILE_SIZE/2.f),(cy(rng)*TILE_SIZE+TILE_SIZE/2.f)};
        civ.alive=true; civ.state=CIV_WANDER; civ.alarmRadius=220.f;
        civ.wanderTarget=civ.pos;
        civs.push_back(civ);
    }
    return civs;
}

EscapeKey Map::makeEscapeKey(){
    // Place in a random non-lobby, non-vault room on floor 0
    std::vector<Room*> candidates;
    for(auto& r:rooms)
        if(r.type!=ROOM_LOBBY&&r.type!=ROOM_VAULT&&r.floor==0)
            candidates.push_back(&r);
    Room* r=candidates.empty()?&rooms[0]:candidates[rng()%candidates.size()];
    Vec2 c=roomCenter(*r);
    c.x+=TILE_SIZE;
    return {c,false};
}