#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <iostream>
#include <locale>

class Daemon;
namespace Config {
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 720;
    constexpr float GRAVITY = 1200.0f;        
    constexpr float FRICTION_DAMPING = 0.95f;
    constexpr float Daemon_Y_Buffer = 20.0f;
    constexpr float Daemon_Slot_Spacing = 10.0f;

    
    constexpr int numberofrowsofpegs = 10;
    constexpr float PegspacingY = 48.0f; 
    constexpr float PegspacingX = 64.0f; 

    inline std::vector<float> basketmults = { 1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 5.0f};

    inline float GAME_SPEED = 0.30f;
    inline float PIN_BOUNCYNESS = 0.20f; 

    constexpr float scoreBlockY = 455.0f;
    //credits
    constexpr float walletY = 530.0f;
    constexpr float walletX = 830.0f;
    constexpr int extraballsreward = 80;

    //shop
    constexpr float shopitemsYbuffer = 100.0f;
    constexpr float shopitemtotalWidth = 640.0f;
    constexpr float shopitemtotalHeight = 80.0f;
    constexpr float shopbuyitembuttonWidth = 140.0f;


    //map
    constexpr int maxmapcolumns = 16;
    constexpr int maxnodespermapcolumn = 4;
    constexpr int maxmapconnections = 4;
    constexpr int totalmapcolumns = 16;
    constexpr float mapscrollspeed = 50.0f;

    //consumables


    
    constexpr float exponentialmapscoregrowth = 0.30f;
    //colors
    const Color COLOR_BG = { 4, 8, 12, 255 };
    const Color COLOR_GRID_LINE = { 0, 75, 50, 255 };
    const Color COLOR_GRID_LINE_DARKER = {0,45,20,225};
    const Color COLOR_NODE = { 0, 255, 180, 255 };
    const Color COLOR_PROBE = { 0, 240, 255, 255 };     
    const Color COLOR_UI_GREEN = { 150, 255, 50, 255 };
    const Color COLOR_UI_AMBER = { 255, 130, 0, 255 };
    const Color COLOR_BASKET = { 12, 32, 42, 255 };
    const Color COLOR_SHARD_BORDER = { 30, 50, 70, 255 };
    const Color MAGENTA_DAEMON = { 198, 52, 249, 255};
    const Color OTHER_COLOR_FOR_DAEMONS = { 250, 15, 82, 255 };
    const Color COLOR_OVERCLOCKED = { 117, 1, 137, 255 };

    
    //shop
    const Color colorBg        = { 4, 2, 8, 255 };
    const Color colorButtonBg  = { 24, 24, 24, 255 };
    const Color colorRedAlert  = { 220, 40, 40, 255 };
}

enum State {
        MainMenu,
        LOST,
        GAME,
        SETTINGS,
        SHOP,
        MAP
    };

namespace Resources{
    inline Sound hoversound;
    inline Sound winsound;
    inline Sound movesound;
}

struct smartbool { //from my other project
    enum State {
        False,
        NewTrue,
        True
    };

    State state = False;

    smartbool() = default;
    smartbool(bool value) { set(value); }
    void operator=(bool value) {this->set(value);}
    operator bool() const {return state != False; }
    bool is_new_true() const { return state == NewTrue; }
    
    void set(bool value) {
        if (value) {
            if (state == False) state = NewTrue;
        } else {
            state = False;
        }
    }
    void update() {
        if (state == NewTrue) {
            state = True;
        }
    }
};



enum ModifierType { MOD_NONE = 0, MOD_BOOST, MOD_GLITCH , MOD_CLONE};
enum DaemonTriggersType { PASSIVE, BASKET, PINS, COLLISION };
#include "daemons.hpp"


struct Node {
    Vector2 position;
    float baseRadius;
    float currentRadius;
    float pulseAnimTimer;
    ModifierType modifier;
};

struct Basket {
    Rectangle bounds;
    std::string name;
    float multiplier;
}; 

struct DaemonSlot {
    float x, y, width, height;
    std::string name;
    std::string status;
    std::string description;
    Color accentColor;
    bool activeGlitch;
    int fillPct;
};

struct Probe {
    int id;               
    Vector2 position;
    Vector2 velocity;
    float radius;
    int hitCount;         
    long double rawPayloadBytes;
    float bufferRate;     
    int lastHitNodeIndex; 
};

struct FadeLine {
    Vector2 start;
    Vector2 end;
    Color color;
    float alpha;
    float fadeSpeed;
};

struct CashoutParticle {
    Vector2 position;
    std::string text;
    float lifetime;
    Color color;
};

struct DaemonsDisplayInfo {
    std::vector<Daemon> daemons;
    int page;
};

struct PendingTrigger {
    int daemonIndex;
    Vector2 probePos;
    int frameDelay;
};

struct ShopState {
    int slots[5] = { -1, -1, -1, -1, -1 };
    bool sold[5] = { false, false, false, false, false };
    bool needsRefresh;
    int rerolls = 1;
    smartbool hoverStates[5];
};

struct GameEngine {
    std::vector<Probe> activeProbes;
    std::vector<Node> nodes;
    std::vector<Basket> baskets;
    std::vector<Daemon> daemons;
    std::vector<FadeLine> fadingLines;
    std::vector<PendingTrigger> triggerQueue;
    std::vector<CashoutParticle> particles;
    Vector2 centerApexPegPos;

    int remainingBalls;
    int nextProbeId = 0;
    std::vector<int> recycledProbeIds;
    const int latencyCap = 15;

    std::string calculationLog;
    float turretBarrelFlash;
};

struct GameState{
    int balance;
    bool allowduplicateshopitems = false;
    State gamestate = SHOP;
};

struct LevelState{
    int MAX_LAUNCH_CAPACITY = 10;
    long double TARGET_QUOTA_BYTES; 
    long double scoredbytes;
    int reward;
};

inline DaemonsDisplayInfo activedaemoninfo;
inline GameEngine engine;
inline GameState gamestate;
inline ShopState shopstate;
inline LevelState levelstate;


enum TransitionPhase {
    TRANS_NONE,
    TRANS_GLITCH_OUT,
    TRANS_GLITCH_IN
};

struct TransitionState {
    TransitionPhase phase = TRANS_NONE;
    float timer = 0.0f;
    float duration = 0.16f;
    State pendingState;
};

struct EnergyTrailPoint {
    Vector2 pos;
    float life;
};

struct EnergyOrbInstance {
    bool travelling = false;
    bool bursting = false;
    Vector2 startPos;
    Vector2 endPos;
    float timer = 0.0f;
    float duration = 0.55f;
    float burstTimer = 0.0f;
    float burstDuration = 0.3f;
    int value = 0;
    std::vector<EnergyTrailPoint> trail;
    float trailSpawnTimer = 0.0f;
};

inline std::vector<EnergyOrbInstance> activeOrbs;

inline TransitionState transition;
inline RenderTexture2D sceneTarget;