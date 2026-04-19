#pragma once
#include "game.h"

// ─── stb_image single-header PNG loader ──────────────────────────────────────
// Include stb_image.h from your project root (download from github.com/nothings/stb)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ─── Embedded 8x8 bitmap font ────────────────────────────────────────────────
static const unsigned char FONT[96][8] = {
    {0,0,0,0,0,0,0,0},
    {0x18,0x18,0x18,0x18,0,0,0x18,0},
    {0x6C,0x6C,0x24,0,0,0,0,0},
    {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0},
    {0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0},
    {0,0x66,0x6C,0x18,0x30,0x66,0x46,0},
    {0x1C,0x36,0x1C,0x38,0x6F,0x66,0x3B,0},
    {0x18,0x18,0x30,0,0,0,0,0},
    {0x0E,0x1C,0x18,0x18,0x18,0x1C,0x0E,0},
    {0x70,0x38,0x18,0x18,0x18,0x38,0x70,0},
    {0,0x66,0x3C,0xFF,0x3C,0x66,0,0},
    {0,0x18,0x18,0x7E,0x18,0x18,0,0},
    {0,0,0,0,0,0x18,0x18,0x30},
    {0,0,0,0x7E,0,0,0,0},
    {0,0,0,0,0,0x18,0x18,0},
    {0x06,0x0C,0x18,0x30,0x60,0x40,0,0},
    {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0},
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0},
    {0x3C,0x66,0x06,0x0C,0x30,0x60,0x7E,0},
    {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0},
    {0x0C,0x1C,0x3C,0x6C,0x7E,0x0C,0x0C,0},
    {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0},
    {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0},
    {0x7E,0x06,0x0C,0x18,0x30,0x30,0x30,0},
    {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0},
    {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0},
    {0,0x18,0x18,0,0,0x18,0x18,0},
    {0,0x18,0x18,0,0,0x18,0x18,0x30},
    {0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0},
    {0,0,0x7E,0,0,0x7E,0,0},
    {0x60,0x30,0x18,0x0C,0x18,0x30,0x60,0},
    {0x3C,0x66,0x06,0x1C,0x18,0,0x18,0},
    {0x3C,0x66,0x6E,0x6E,0x60,0x62,0x3C,0},
    {0x18,0x3C,0x66,0x7E,0x66,0x66,0x66,0},
    {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0},
    {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0},
    {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0},
    {0x7E,0x60,0x60,0x78,0x60,0x60,0x7E,0},
    {0x7E,0x60,0x60,0x78,0x60,0x60,0x60,0},
    {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3C,0},
    {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0},
    {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0},
    {0x06,0x06,0x06,0x06,0x06,0x66,0x3C,0},
    {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0},
    {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0},
    {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0},
    {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0},
    {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0},
    {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0},
    {0x3C,0x66,0x66,0x66,0x66,0x3C,0x0E,0},
    {0x7C,0x66,0x66,0x7C,0x78,0x6C,0x66,0},
    {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0},
    {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0},
    {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0},
    {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0},
    {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0},
    {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0},
    {0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0},
    {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0},
    {0x1E,0x18,0x18,0x18,0x18,0x18,0x1E,0},
    {0x60,0x30,0x18,0x0C,0x06,0x03,0,0},
    {0x78,0x18,0x18,0x18,0x18,0x18,0x78,0},
    {0x08,0x1C,0x36,0x63,0,0,0,0},
    {0,0,0,0,0,0,0,0xFF},
    {0x18,0x18,0x0C,0,0,0,0,0},
    {0,0,0x3C,0x06,0x3E,0x66,0x3E,0},
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x7C,0},
    {0,0,0x3C,0x60,0x60,0x60,0x3C,0},
    {0x06,0x06,0x3E,0x66,0x66,0x66,0x3E,0},
    {0,0,0x3C,0x66,0x7E,0x60,0x3C,0},
    {0x0E,0x18,0x18,0x7E,0x18,0x18,0x18,0},
    {0,0,0x3E,0x66,0x66,0x3E,0x06,0x3C},
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0},
    {0x18,0,0x18,0x18,0x18,0x18,0x18,0},
    {0x06,0,0x06,0x06,0x06,0x06,0x06,0x3C},
    {0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0},
    {0x18,0x18,0x18,0x18,0x18,0x18,0x0E,0},
    {0,0,0x66,0x7F,0x7F,0x6B,0x63,0},
    {0,0,0x7C,0x66,0x66,0x66,0x66,0},
    {0,0,0x3C,0x66,0x66,0x66,0x3C,0},
    {0,0,0x7C,0x66,0x66,0x7C,0x60,0x60},
    {0,0,0x3E,0x66,0x66,0x3E,0x06,0x06},
    {0,0,0x7C,0x66,0x60,0x60,0x60,0},
    {0,0,0x3E,0x60,0x3C,0x06,0x7C,0},
    {0x18,0x18,0x7E,0x18,0x18,0x18,0x0E,0},
    {0,0,0x66,0x66,0x66,0x66,0x3E,0},
    {0,0,0x66,0x66,0x66,0x3C,0x18,0},
    {0,0,0x63,0x6B,0x7F,0x3E,0x36,0},
    {0,0,0x66,0x3C,0x18,0x3C,0x66,0},
    {0,0,0x66,0x66,0x3E,0x06,0x3C,0},
    {0,0,0x7E,0x0C,0x18,0x30,0x7E,0},
    {0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0},
    {0x18,0x18,0x18,0,0x18,0x18,0x18,0},
    {0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0},
    {0x76,0xDC,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
};

// ─── Sprite path table (matches SpriteID enum order) ─────────────────────────
static const char* SPRITE_PATHS[SPR_COUNT] = {
    "assets/tiles/floor.png",               // SPR_TILE_FLOOR
    "assets/tiles/wall.png",                // SPR_TILE_WALL
    "assets/tiles/door.png",                // SPR_TILE_DOOR
    "assets/tiles/wall_top.png",            // SPR_TILE_WALL_TOP
    "assets/functional/vault_door_closed.png",
    "assets/functional/vault_door_open.png",
    "assets/functional/camera_idle.png",
    "assets/functional/camera_alert.png",
    "assets/functional/exit_marker.png",
    "assets/functional/escape_key.png",
    "assets/functional/loot_cash.png",
    "assets/functional/loot_jewel.png",
    "assets/functional/loot_gold.png",
    "assets/functional/loot_art.png",
    "assets/functional/spawn_marker.png",
    "assets/enemies/guard_patrol.png",
    "assets/enemies/guard_suspicious.png",
    "assets/enemies/guard_alert.png",
    "assets/enemies/guard_dead.png",
    "assets/enemies/guard_reinforcement.png",
    "assets/player/player.png",
    "assets/player/player_crouch.png",
    "assets/player/player_cloaked.png",
};

// ─── Renderer class ───────────────────────────────────────────────────────────
class Renderer {
public:
    GLuint shaderProg = 0;
    GLuint vao = 0, vbo = 0;
    GLuint fontTex  = 0;

    GLuint spriteTex[SPR_COUNT];
    int    spriteW[SPR_COUNT], spriteH[SPR_COUNT];
    bool   spriteLoaded[SPR_COUNT];

    float camX = 0, camY = 0;
    int   screenW, screenH;

    struct Vert { float x,y,u,v,r,g,b,a; };
    std::vector<Vert> verts;

    void init(int w, int h);
    void beginFrame();
    void flush();
    void endFrame();
    void setCamera(float x, float y){ camX=x; camY=y; }

    void drawQuad(float x,float y,float w,float h, Color c,
                  float u0=0,float v0=0,float u1=1,float v1=1);
    void drawQuadScreen(float x,float y,float w,float h, Color c);
    // Centered, rotated sprite in world space
    void drawSprite(SpriteID id, float cx, float cy, float angle=0.f,
                    float scale=1.f, Color tint={1,1,1,1});
    // Top-left sprite in screen space
    void drawSpriteScreen(SpriteID id, float x, float y, float scale=1.f,
                          Color tint={1,1,1,1});
    void drawText(const std::string& s, float x, float y, float scale,
                  Color c, bool screenSpace=true);
    void drawCircle(float cx,float cy,float r, Color c,
                    bool screenSpace=false, int segs=20);
    void drawCone(float cx,float cy,float angle,float fov,float range, Color c);

private:
    void buildFontTexture();
    void loadSprites();
    void compileShaders();
    GLuint loadTexturePNG(const char* path, int& w, int& h);
};

// ─── GLSL shaders ─────────────────────────────────────────────────────────────
static const char* VS_SRC = R"(
#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aUV;
layout(location=2) in vec4 aColor;
out vec2 vUV;
out vec4 vColor;
uniform vec2 uCam;
uniform vec2 uScreen;
uniform bool uScreenSpace;
void main(){
    vec2 p = uScreenSpace ? aPos : (aPos - uCam);
    vec2 ndc = (p / uScreen) * 2.0 - 1.0;
    ndc.y = -ndc.y;
    gl_Position = vec4(ndc, 0.0, 1.0);
    vUV = aUV;
    vColor = aColor;
}
)";

static const char* FS_SRC = R"(
#version 330 core
in vec2 vUV;
in vec4 vColor;
out vec4 FragColor;
uniform sampler2D uTex;
uniform int uTexMode; // 0=solid color, 1=font alpha, 2=RGBA sprite
void main(){
    if(uTexMode == 1){
        float a = texture(uTex, vUV).r;
        FragColor = vec4(vColor.rgb, vColor.a * a);
    } else if(uTexMode == 2){
        vec4 t = texture(uTex, vUV);
        FragColor = t * vColor;          // tint: multiply sprite rgba by tint color
    } else {
        FragColor = vColor;
    }
}
)";

static GLuint compileShaderSrc(GLenum type, const char* src){
    GLuint s = glCreateShader(type);
    glShaderSource(s,1,&src,nullptr);
    glCompileShader(s);
    return s;
}

void Renderer::compileShaders(){
    GLuint vs = compileShaderSrc(GL_VERTEX_SHADER,   VS_SRC);
    GLuint fs = compileShaderSrc(GL_FRAGMENT_SHADER, FS_SRC);
    shaderProg = glCreateProgram();
    glAttachShader(shaderProg,vs);
    glAttachShader(shaderProg,fs);
    glLinkProgram(shaderProg);
    glDeleteShader(vs); glDeleteShader(fs);
}

void Renderer::buildFontTexture(){
    const int COLS=16, GW=8, GH=8;
    const int TW=COLS*GW, TH=6*GH;
    std::vector<unsigned char> buf(TW*TH,0);
    for(int c=0;c<96;c++){
        int col=c%COLS, row=c/COLS;
        for(int y=0;y<GH;y++){
            unsigned char bits=FONT[c][y];
            for(int x=0;x<GW;x++)
                if(bits&(0x80>>x)) buf[(row*GH+y)*TW+col*GW+x]=255;
        }
    }
    glGenTextures(1,&fontTex);
    glBindTexture(GL_TEXTURE_2D,fontTex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,TW,TH,0,GL_RED,GL_UNSIGNED_BYTE,buf.data());
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
}

GLuint Renderer::loadTexturePNG(const char* path, int& outW, int& outH){
    int w=0,h=0,ch=0;
    stbi_set_flip_vertically_on_load(0);
    unsigned char* data = stbi_load(path, &w, &h, &ch, 4);
    if(!data){ outW=0; outH=0; return 0; }
    GLuint tex=0;
    glGenTextures(1,&tex);
    glBindTexture(GL_TEXTURE_2D,tex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    stbi_image_free(data);
    outW=w; outH=h;
    return tex;
}

void Renderer::loadSprites(){
    for(int i=0;i<SPR_COUNT;i++){
        spriteLoaded[i]=false;
        spriteTex[i]=0;
        spriteW[i]=32; spriteH[i]=32;
        int w=0,h=0;
        GLuint tex=loadTexturePNG(SPRITE_PATHS[i],w,h);
        if(tex){
            spriteTex[i]=tex;
            spriteW[i]=w; spriteH[i]=h;
            spriteLoaded[i]=true;
        }
    }
}

void Renderer::init(int w, int h){
    screenW=w; screenH=h;
    compileShaders();
    buildFontTexture();
    loadSprites();
    glGenVertexArrays(1,&vao);
    glGenBuffers(1,&vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,131072*sizeof(Vert),nullptr,GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(Vert),(void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(Vert),(void*)(2*sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,4,GL_FLOAT,GL_FALSE,sizeof(Vert),(void*)(4*sizeof(float)));
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::beginFrame(){
    verts.clear();
    glClearColor(0.05f,0.05f,0.07f,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProg);
    glUniform2f(glGetUniformLocation(shaderProg,"uScreen"),(float)screenW,(float)screenH);
    glUniform2f(glGetUniformLocation(shaderProg,"uCam"),camX,camY);
    glUniform1i(glGetUniformLocation(shaderProg,"uTex"),0);
    glActiveTexture(GL_TEXTURE0);
}

// ─── Batching internals ───────────────────────────────────────────────────────
struct DrawCall2 {
    bool   screenSpace;
    int    texMode;
    GLuint tex;
    size_t start, count;
};
static std::vector<DrawCall2> sDC2;
static size_t s_bufCap = 131072*sizeof(Renderer::Vert);

static void pushQuad2(std::vector<Renderer::Vert>& v,
                      float x,float y,float w,float h,
                      float u0,float v0,float u1,float v1,
                      Color c, bool ss, int tm, GLuint tex){
    size_t start=v.size();
    auto p=[&](float px,float py,float pu,float pv){
        v.push_back({px,py,pu,pv,c.r,c.g,c.b,c.a});
    };
    p(x,  y,  u0,v0); p(x+w,y,  u1,v0); p(x+w,y+h,u1,v1);
    p(x,  y,  u0,v0); p(x+w,y+h,u1,v1); p(x,  y+h,u0,v1);
    if(!sDC2.empty()&&sDC2.back().screenSpace==ss&&sDC2.back().texMode==tm&&sDC2.back().tex==tex)
        sDC2.back().count+=6;
    else sDC2.push_back({ss,tm,tex,start,6});
}

static void pushRotated(std::vector<Renderer::Vert>& v,
                        float cx,float cy,float hw,float hh,float angle,
                        float u0,float v0,float u1,float v1,
                        Color c, bool ss, int tm, GLuint tex){
    float cosA=cosf(angle), sinA=sinf(angle);
    float ox[4]={-hw, hw, hw,-hw};
    float oy[4]={-hh,-hh, hh, hh};
    float fu[4]={u0,u1,u1,u0};
    float fv[4]={v0,v0,v1,v1};
    float rx[4],ry[4];
    for(int i=0;i<4;i++){
        rx[i]=cx+ox[i]*cosA-oy[i]*sinA;
        ry[i]=cy+ox[i]*sinA+oy[i]*cosA;
    }
    size_t start=v.size();
    auto push=[&](int i){ v.push_back({rx[i],ry[i],fu[i],fv[i],c.r,c.g,c.b,c.a}); };
    push(0); push(1); push(2);
    push(0); push(2); push(3);
    if(!sDC2.empty()&&sDC2.back().screenSpace==ss&&sDC2.back().texMode==tm&&sDC2.back().tex==tex)
        sDC2.back().count+=6;
    else sDC2.push_back({ss,tm,tex,start,6});
}

void Renderer::flush(){
    if(verts.empty()){ sDC2.clear(); return; }
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    size_t needed=verts.size()*sizeof(Vert);
    if(needed>s_bufCap){
        s_bufCap=needed*2;
        glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)s_bufCap,nullptr,GL_DYNAMIC_DRAW);
    }
    glBufferSubData(GL_ARRAY_BUFFER,0,(GLsizeiptr)needed,verts.data());
    GLint ssLoc=glGetUniformLocation(shaderProg,"uScreenSpace");
    GLint tmLoc=glGetUniformLocation(shaderProg,"uTexMode");
    for(auto& dc:sDC2){
        glUniform1i(ssLoc,dc.screenSpace?1:0);
        glUniform1i(tmLoc,dc.texMode);
        if(dc.texMode>0&&dc.tex) glBindTexture(GL_TEXTURE_2D,dc.tex);
        else                     glBindTexture(GL_TEXTURE_2D,0);
        glDrawArrays(GL_TRIANGLES,(GLint)dc.start,(GLsizei)dc.count);
    }
    verts.clear();
    sDC2.clear();
}

void Renderer::drawQuad(float x,float y,float w,float h,Color c,
                        float u0,float v0,float u1,float v1){
    pushQuad2(verts,x,y,w,h,u0,v0,u1,v1,c,false,0,0);
}
void Renderer::drawQuadScreen(float x,float y,float w,float h,Color c){
    pushQuad2(verts,x,y,w,h,0,0,1,1,c,true,0,0);
}

void Renderer::drawSprite(SpriteID id,float cx,float cy,float angle,
                          float scale,Color tint){
    if(id<0||id>=SPR_COUNT) return;
    float hw=spriteW[id]*0.5f*scale;
    float hh=spriteH[id]*0.5f*scale;
    if(spriteLoaded[id])
        pushRotated(verts,cx,cy,hw,hh,angle,0,0,1,1,tint,false,2,spriteTex[id]);
    else
        pushQuad2(verts,cx-hw,cy-hh,hw*2,hh*2,0,0,1,1,tint,false,0,0);
}

void Renderer::drawSpriteScreen(SpriteID id,float x,float y,float scale,Color tint){
    if(id<0||id>=SPR_COUNT) return;
    float w2=spriteW[id]*scale, h2=spriteH[id]*scale;
    if(spriteLoaded[id])
        pushQuad2(verts,x,y,w2,h2,0,0,1,1,tint,true,2,spriteTex[id]);
    else
        pushQuad2(verts,x,y,w2,h2,0,0,1,1,tint,true,0,0);
}

void Renderer::drawText(const std::string& s,float x,float y,float scale,
                        Color c,bool screenSpace){
    const int COLS=16, GW=8, GH=8;
    const float TW=128.f, TH=48.f;
    float cx2=x;
    for(char ch:s){
        int idx=(int)ch-32;
        if(idx<0||idx>=96){cx2+=GW*scale;continue;}
        int col=idx%COLS, row=idx/COLS;
        float u0=col*GW/TW, v0=row*GH/TH;
        float u1=u0+GW/TW,  v1=v0+GH/TH;
        pushQuad2(verts,cx2,y,GW*scale,GH*scale,u0,v0,u1,v1,c,screenSpace,1,fontTex);
        cx2+=GW*scale;
    }
}

void Renderer::drawCircle(float cx,float cy,float r,Color c,bool ss,int segs){
    for(int i=0;i<segs;i++){
        float a0=2*M_PI*i/segs, a1=2*M_PI*(i+1)/segs;
        size_t start=verts.size();
        verts.push_back({cx,cy,0,0,c.r,c.g,c.b,c.a});
        verts.push_back({cx+cosf(a0)*r,cy+sinf(a0)*r,0,0,c.r,c.g,c.b,c.a});
        verts.push_back({cx+cosf(a1)*r,cy+sinf(a1)*r,0,0,c.r,c.g,c.b,c.a});
        if(!sDC2.empty()&&sDC2.back().screenSpace==ss&&sDC2.back().texMode==0)
            sDC2.back().count+=3;
        else sDC2.push_back({ss,0,0,start,3});
    }
}

void Renderer::drawCone(float cx,float cy,float angle,float fov,float range,Color c){
    int segs=20; float half=fov*0.5f;
    for(int i=0;i<segs;i++){
        float t0=(float)i/segs, t1=(float)(i+1)/segs;
        float a0=angle-half+t0*fov, a1=angle-half+t1*fov;
        size_t start=verts.size();
        verts.push_back({cx,cy,0,0,c.r,c.g,c.b,c.a});
        verts.push_back({cx+cosf(a0)*range,cy+sinf(a0)*range,0,0,c.r,c.g,c.b,c.a});
        verts.push_back({cx+cosf(a1)*range,cy+sinf(a1)*range,0,0,c.r,c.g,c.b,c.a});
        if(!sDC2.empty()&&!sDC2.back().screenSpace&&sDC2.back().texMode==0)
            sDC2.back().count+=3;
        else sDC2.push_back({false,0,0,start,3});
    }
}

void Renderer::endFrame(){ flush(); }