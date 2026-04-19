#pragma once
#include "game.h"
#include <cmath>
#include <vector>
#include <cstring>

// ─── Minimal procedural sound system using raw PCM → OpenAL ─────────────────
// Requires: libopenal-dev  (apt install libopenal-dev)
// Link with: -lopenal

#ifdef __APPLE__
  #include <OpenAL/al.h>
  #include <OpenAL/alc.h>
#else
  #include <AL/al.h>
  #include <AL/alc.h>
#endif

class SoundSystem {
public:
    static const int SAMPLE_RATE = 22050;
    ALCdevice*  device  = nullptr;
    ALCcontext* context = nullptr;
    ALuint      buffers[SND_COUNT];
    ALuint      sources[8];   // pooled sources
    int         nextSrc = 0;
    bool        ok = false;
    float       masterVol = 1.0f;

    void init(){
        device = alcOpenDevice(nullptr);
        if(!device) return;
        context = alcCreateContext(device, nullptr);
        if(!context){ alcCloseDevice(device); return; }
        alcMakeContextCurrent(context);
        alGenBuffers(SND_COUNT, buffers);
        alGenSources(8, sources);
        for(int i=0;i<8;i++) alSourcef(sources[i], AL_GAIN, masterVol);
        generateAll();
        ok = true;
    }

    void shutdown(){
        if(!ok) return;
        alDeleteSources(8, sources);
        alDeleteBuffers(SND_COUNT, buffers);
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context);
        alcCloseDevice(device);
    }

    void play(SoundID id, float vol=1.0f, float pitch=1.0f){
        if(!ok) return;
        ALuint src = sources[nextSrc % 8];
        nextSrc++;
        alSourceStop(src);
        alSourcei(src, AL_BUFFER, buffers[id]);
        alSourcef(src, AL_GAIN, vol * masterVol);
        alSourcef(src, AL_PITCH, pitch);
        alSourcei(src, AL_LOOPING, AL_FALSE);
        alSourcePlay(src);
    }

private:
    // ── Waveform helpers ────────────────────────────────────────────────────
    std::vector<int16_t> makeSine(float freq, float dur, float amp=0.5f, float decay=0){
        int n = (int)(SAMPLE_RATE * dur);
        std::vector<int16_t> buf(n);
        for(int i=0;i<n;i++){
            float t = (float)i / SAMPLE_RATE;
            float env = (decay>0) ? expf(-decay*t) : 1.0f;
            buf[i] = (int16_t)(amp * env * 32767.f * sinf(2*M_PI*freq*t));
        }
        return buf;
    }
    std::vector<int16_t> makeNoise(float dur, float amp=0.4f, float decay=8.0f){
        int n = (int)(SAMPLE_RATE * dur);
        std::vector<int16_t> buf(n);
        for(int i=0;i<n;i++){
            float t = (float)i/SAMPLE_RATE;
            float env = expf(-decay*t);
            float s = ((float)rand()/RAND_MAX*2.f-1.f);
            buf[i] = (int16_t)(amp * env * 32767.f * s);
        }
        return buf;
    }
    std::vector<int16_t> makeClick(float dur=0.04f){
        int n=(int)(SAMPLE_RATE*dur);
        std::vector<int16_t> buf(n);
        for(int i=0;i<n;i++){
            float t=(float)i/SAMPLE_RATE;
            float env=expf(-60.f*t);
            buf[i]=(int16_t)(0.6f*env*32767.f*((rand()%2)*2-1));
        }
        return buf;
    }
    std::vector<int16_t> makeBleep(float freq, float dur){
        int n=(int)(SAMPLE_RATE*dur);
        std::vector<int16_t> buf(n);
        for(int i=0;i<n;i++){
            float t=(float)i/SAMPLE_RATE;
            float env=expf(-5.f*t);
            buf[i]=(int16_t)(0.4f*env*32767.f*sinf(2*M_PI*freq*t));
        }
        return buf;
    }
    // Mix two buffers
    std::vector<int16_t> mix(const std::vector<int16_t>& a, const std::vector<int16_t>& b){
        size_t n=std::max(a.size(),b.size());
        std::vector<int16_t> out(n,0);
        for(size_t i=0;i<n;i++){
            int s=0;
            if(i<a.size()) s+=a[i];
            if(i<b.size()) s+=b[i];
            out[i]=(int16_t)std::max(-32767,std::min(32767,s));
        }
        return out;
    }

    void upload(SoundID id, const std::vector<int16_t>& data){
        alBufferData(buffers[id], AL_FORMAT_MONO16,
                     data.data(), (ALsizei)(data.size()*2), SAMPLE_RATE);
    }

    void generateAll(){
        // Gunshot: loud burst of noise
        upload(SND_GUNSHOT,
            mix(makeNoise(0.15f, 0.8f, 18.f), makeSine(120, 0.15f, 0.4f, 25)));
        // Silenced: softer thwip
        upload(SND_GUNSHOT_SILENCED,
            mix(makeNoise(0.07f, 0.3f, 35.f), makeSine(350, 0.06f, 0.15f, 60)));
        // Reload: metallic clicks
        {auto v=makeClick(); auto v2=makeClick();
         for(auto& s:v2) s>>=1;
         auto m=mix(v,v2); upload(SND_RELOAD,m);}
        // Footstep
        upload(SND_FOOTSTEP, makeNoise(0.06f, 0.25f, 30.f));
        upload(SND_FOOTSTEP_CROUCH, makeNoise(0.04f, 0.08f, 50.f));
        // Alarm: repeating beep
        {
            int n=(int)(SAMPLE_RATE*0.8f);
            std::vector<int16_t> buf(n,0);
            for(int i=0;i<n;i++){
                float t=(float)i/SAMPLE_RATE;
                float beep=sinf(2*M_PI*880*t)*0.5f * (fmodf(t,0.2f)<0.1f?1.f:0.f);
                buf[i]=(int16_t)(beep*32767.f);
            }
            upload(SND_ALARM,buf);
        }
        // Guard shout: noise burst at low freq
        upload(SND_GUARD_SHOUT,
            mix(makeNoise(0.35f, 0.6f, 5.f), makeSine(220, 0.3f, 0.3f, 8)));
        upload(SND_GUARD_SPOT, makeBleep(660, 0.15f));
        upload(SND_LOOT_PICKUP, makeBleep(1200, 0.1f));
        upload(SND_VAULT_OPEN,
            mix(makeBleep(440, 0.4f), makeBleep(660, 0.4f)));
        upload(SND_DOOR_OPEN, makeNoise(0.12f, 0.2f, 20.f));
        // Melee hit
        upload(SND_MELEE_HIT,
            mix(makeNoise(0.08f, 0.5f, 25.f), makeSine(180, 0.08f, 0.3f, 30)));
        // Player hurt
        upload(SND_PLAYER_HURT,
            mix(makeNoise(0.15f, 0.4f, 15.f), makeSine(160, 0.15f, 0.3f, 20)));
        upload(SND_PLAYER_DEATH,
            mix(makeNoise(0.5f, 0.7f, 4.f), makeSine(80, 0.5f, 0.5f, 6)));
        upload(SND_CIVILIAN_SCREAM,
            mix(makeSine(600, 0.5f, 0.5f, 3.f), makeSine(900, 0.4f, 0.3f, 4.f)));
        // Server destroy
        upload(SND_SERVER_DESTROY,
            mix(makeNoise(0.3f, 0.6f, 8.f), makeBleep(200, 0.3f)));
        // Explosion
        {
            auto n1=makeNoise(0.5f, 0.9f, 3.f);
            auto n2=makeNoise(0.5f, 0.7f, 5.f);
            upload(SND_EXPLOSION, mix(n1,n2));
        }
        // EMP: electrical buzz
        {
            int n=(int)(SAMPLE_RATE*0.4f);
            std::vector<int16_t> buf(n);
            for(int i=0;i<n;i++){
                float t=(float)i/SAMPLE_RATE;
                float env=expf(-8.f*t);
                float sq=sinf(2*M_PI*60*t)>0?1.f:-1.f;
                buf[i]=(int16_t)(0.4f*env*sq*32767.f);
            }
            upload(SND_EMP,buf);
        }
        upload(SND_CLICK, makeClick(0.03f));
    }
};

// Global sound system instance
inline SoundSystem g_sound;
inline void SND(SoundID id, float vol=1.f, float pitch=1.f){ g_sound.play(id,vol,pitch); }