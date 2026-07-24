#include "raylib.h"
#include "main.hpp"
#include <vector>
#include <cmath>
#include <string>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include "daemons.hpp"
#include <algorithm>
#include "shop.hpp"
#include "map.hpp"
#include <algorithm>
#include "raymath.h"
#include "raymath.h"
#include "audio.hpp"
#include "consumables.hpp"
#include "payout.hpp"
#include "transition.hpp"
//Update 14:
//consumable that turns encrypted off selected map node
//move cursor over glitched node a bunch of times it gets stuck

//Update 15:
//screen shake
//more animations
//map cooler node graphics

//General
//more daemons
//clean up code for more efficiency


//Update eventually:
//make black market a normal shop and add nodes that are black market with special daemons

#if defined(PLATFORM_WEB)
    #include <emscripten.h>
#endif


std::string FormatByteSize(long double bytes) {
    if (bytes < 1) return "0 B";
    if (bytes < 1024.0) return std::to_string(bytes)+" B";
    const char* suffixes[] = {"MB", "GB", "TB", "PB", "EB", "ZB", "YB", "RB", "QB" };
    int i = 0;
    long double size = bytes / 1024.0;
    while (size >= 1024.0 && i < 8) {
        size /= 1024.0;
        i++;
    }
    std::stringstream stream;
    if (i == 8 && size >= 1024.0) {
        stream << std::scientific << std::setprecision(2) << size << " " << suffixes[i];
    } else {
        stream << std::fixed << std::setprecision(2) << size << " " << suffixes[i];
    }
    return stream.str();
}

std::string formatWithSpaces(long long int num) {
    std::string str = std::to_string(num);
    std::string result = "";
    int count = 0;

    for (int i = str.length() - 1; i >= 0; i--) {
        if (count == 3) {
            result += " ";
            count = 0;
        }
        result += str[i];
        count++;
    }

    std::reverse(result.begin(), result.end());
    return result;
}


void InitGame() {
    InitMap();
    sceneTarget = LoadRenderTexture(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT);
    levelstate.scoredbytes = 0.0;
    engine.remainingBalls = levelstate.MAX_LAUNCH_CAPACITY;
    engine.turretBarrelFlash = 0.0f;
    engine.calculationLog = "CORE ARMED: DATA METERS ROUTED TO KB MINIMUMS";
    engine.daemons.clear();
    float startY = 145.0f;

    for (int r = 0; r < Config::numberofrowsofpegs; ++r) {
        int cols = 1 + r; 
        float startX = 400.0f - ((cols - 1) * Config::PegspacingX) / 2.0f;
        for (int c = 0; c < cols; ++c) {
            Node n;
            n.position = { startX + (c * Config::PegspacingX), startY + (r * Config::PegspacingY) };
            n.baseRadius = 4.5f; 
            n.currentRadius = 4.5f;
            n.pulseAnimTimer = 0.0f;
            n.modifier = MOD_NONE;
            engine.nodes.push_back(n);

            if (r == 0 && c == 0) {
                engine.centerApexPegPos = n.position;
            }
        }
    }
    
    float finalRowStartX = 400.0f - (((Config::basketmults.size()-1)*2 - 1) * Config::PegspacingX) / 2.0f;
    float basketY = 600.0f; 
    float basketW = Config::PegspacingX - 8.0f; 

    for (int i = 0; i < (Config::basketmults.size()-1)*2 + 1; ++i) {
        Basket b;
        float bx = (finalRowStartX - (Config::PegspacingX / 2.0f)) + (i * Config::PegspacingX) - (basketW / 2.0f);
        b.bounds = { bx, basketY, basketW, 20.0f };
        
        int centerIndex = (Config::basketmults.size()-1);
        int distanceFromCenter = std::abs(i - centerIndex);

        size_t multiplierIndex = std::min(static_cast<size_t>(distanceFromCenter), Config::basketmults.size() - 1);
        
        b.name = "PORT_" + std::to_string(i + 1);
        b.multiplier = Config::basketmults[multiplierIndex];
        engine.baskets.push_back(b);
    }
    initdaemons();
}

int GetUniqueProbeId(GameEngine& eng) {
    if (!eng.recycledProbeIds.empty()) {
        int recycledId = eng.recycledProbeIds.back();
        eng.recycledProbeIds.pop_back();
        return recycledId;
    }
    return eng.nextProbeId++;
}

void RecycleProbeId(GameEngine& eng, int id) {
    eng.recycledProbeIds.push_back(id);
}

void InjectProbeFromTurret() {
    if (engine.remainingBalls <= 0) return;

    engine.remainingBalls--;
    engine.turretBarrelFlash = 0.12f;

    Probe p;
    p.id = GetUniqueProbeId(engine);
    
    float variance = (float)GetRandomValue(-12, 12);
    p.position = { engine.centerApexPegPos.x + variance, engine.centerApexPegPos.y - 80.0f };
    p.velocity = { (float)GetRandomValue(-8, 8), 100.0f };
    p.radius = 9.0f;
    p.hitCount = 0;
    p.rawPayloadBytes = 1024.0;
    p.bufferRate = 1.0f;
    p.lastHitNodeIndex = -1; 

    engine.activeProbes.push_back(p);
}

float displayedBalance = -1.0f;

void UpdateDisplayedBalance() {
    if (displayedBalance < 0.0f) {
        displayedBalance = (float)gamestate.balance;
        return;
    }

    float target = (float)gamestate.balance;
    float diff = target - displayedBalance;

    float speed = 4.0f + fabsf(diff) * 3.0f;
    displayedBalance += diff * Clamp(speed * GetFrameTime(), 0.0f, 1.0f);

    if (fabsf(target - displayedBalance) < 0.5f) {
        displayedBalance = target;
    }
}


void UpdatePhysics(float dt) {
    
    std::vector<Probe> clonesToSpawn;
    float scaledDt = dt * Config::GAME_SPEED;

    if (engine.turretBarrelFlash > 0.0f) engine.turretBarrelFlash -= scaledDt;

    for (auto& node : engine.nodes){
        if (node.pulseAnimTimer > 0.0f) {
            node.pulseAnimTimer -= dt * 6.0f;
            node.currentRadius = node.baseRadius + (sinf(node.pulseAnimTimer * 3.14159f) * 4.0f);
        } else {
            node.pulseAnimTimer = 0.0f;
            node.currentRadius = node.baseRadius;
        }
    }

    for (size_t i = 0; i < engine.particles.size();) {
        engine.particles[i].position.y -= 35.0f * scaledDt;
        engine.particles[i].lifetime -= scaledDt;
        if (engine.particles[i].lifetime <= 0) {
            engine.particles.erase(engine.particles.begin() + i);
        } else {
            i++;
        }
    }

    for (size_t i = 0; i < engine.activeProbes.size(); ++i) {
        for (size_t j = i + 1; j < engine.activeProbes.size(); ++j) {
            Probe& p1 = engine.activeProbes[i];
            Probe& p2 = engine.activeProbes[j];

            float dx = p2.position.x - p1.position.x;
            float dy = p2.position.y - p1.position.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            float minDist = p1.radius + p2.radius;

            if (dist < minDist && dist > 0.1f) {
                float overlap = minDist - dist;
                Vector2 normal = { dx / dist, dy / dist };

                p1.position.x -= normal.x * overlap * 0.5f;
                p1.position.y -= normal.y * overlap * 0.5f;
                p2.position.x += normal.x * overlap * 0.5f;
                p2.position.y += normal.y * overlap * 0.5f;

                float kx = p1.velocity.x - p2.velocity.x;
                float ky = p1.velocity.y - p2.velocity.y;
                float p = normal.x * kx + normal.y * ky;

                if (p > 0) {
                    p1.velocity.x -= p * normal.x * Config::PIN_BOUNCYNESS;
                    p1.velocity.y -= p * normal.y * Config::PIN_BOUNCYNESS;
                    p2.velocity.x += p * normal.x * Config::PIN_BOUNCYNESS;
                    p2.velocity.y += p * normal.y * Config::PIN_BOUNCYNESS;
                }
            }
        }
    }

    for (size_t i = 0; i < engine.activeProbes.size(); i++) {
        Probe& p = engine.activeProbes[i];
        
        p.velocity.y += Config::GRAVITY * scaledDt;
        p.position.x += p.velocity.x * scaledDt;
        p.position.y += p.velocity.y * scaledDt;

        p.velocity.x *= std::pow(Config::FRICTION_DAMPING, scaledDt * 60.0f); 

        if (p.position.x - p.radius < 10.0f) {
            p.position.x = 10.0f + p.radius;
            p.velocity.x *= -Config::PIN_BOUNCYNESS;
        }
        if (p.position.x + p.radius > 790.0f) {
            p.position.x = 790.0f - p.radius;
            p.velocity.x *= -Config::PIN_BOUNCYNESS;
        }

        for (size_t nIdx = 0; nIdx < engine.nodes.size(); nIdx++) {
            auto& node = engine.nodes[nIdx];
            float distX = p.position.x - node.position.x;
            float distY = p.position.y - node.position.y;
            float distance = std::sqrt(distX * distX + distY * distY);
            float minDist = p.radius + node.baseRadius;


            if (distance < minDist) {
                Vector2 normal = { distX / distance, distY / distance };

                if (normal.y > 0.2f) continue; 

                float overlap = minDist - distance;
                p.position.x += normal.x * overlap;
                p.position.y += normal.y * overlap;
                float dot = p.velocity.x * normal.x + p.velocity.y * normal.y;
                p.velocity.x -= (1.0f + Config::PIN_BOUNCYNESS) * dot * normal.x;
                p.velocity.y -= (1.0f + Config::PIN_BOUNCYNESS) * dot * normal.y;
                p.velocity.x += (float)GetRandomValue(-10, 10) * 0.25f;
                if ((int)nIdx != p.lastHitNodeIndex) {
                    p.lastHitNodeIndex = (int)nIdx; 
                    node.pulseAnimTimer = 1.0f;
                    
                    playsoundsmart(nodehitsound,.3,2.4); //node hit sound comment here so i can find easily
                    p.hitCount++;
                    for (size_t i = 0; i < activedaemoninfo.daemons.size(); i++) {
                        if (activedaemoninfo.daemons[i].triggertype == PINS){
                            activedaemoninfo.daemons[i].TriggerAction(p);
                        }
                    }
                    long double calculatedByteBump = 1024.0;
                    if (node.modifier == MOD_BOOST) calculatedByteBump *= 2.5;
                    else if (node.modifier == MOD_GLITCH) calculatedByteBump *= ((float)GetRandomValue(5, 50) * 0.2f);

                    p.rawPayloadBytes += calculatedByteBump;
                    p.bufferRate += (node.modifier == MOD_GLITCH ? 0.35f : 0.12f);

                    if (node.modifier == MOD_CLONE) {
                        float pushOffset = p.radius + node.baseRadius + 4.0f;
                        float speedSnap = fabsf(p.velocity.x) > 10.0f ? fabsf(p.velocity.x) : 80.0f;

                        Probe cloneL = p;
                        cloneL.id = GetUniqueProbeId(engine);
                        cloneL.position.x = node.position.x - pushOffset;
                        cloneL.velocity.x = -speedSnap;
                        cloneL.lastHitNodeIndex = (int)nIdx;
                        p.position.x = node.position.x + pushOffset;
                        p.velocity.x = speedSnap;
                        
                        clonesToSpawn.push_back(cloneL);
                        
                        engine.calculationLog = "THREAD SPLIT: DUAL TRAJECTORY CLONE INSTANTIATED";
                    }
                }
            }
        }

        bool absorbed = false; //DAEMON TRIGGER PASSIVE
        for (const auto& basket : engine.baskets) {
            if (CheckCollisionCircleRec(p.position, p.radius, basket.bounds)) {
                for (size_t i = 0; i < activedaemoninfo.daemons.size(); i++) {
                    if (activedaemoninfo.daemons[i].triggertype == PASSIVE || activedaemoninfo.daemons[i].triggertype == BASKET){
                        activedaemoninfo.daemons[i].TriggerAction(p);
                    }
                }
                long double localizedFinalBytesYield = std::round(p.rawPayloadBytes * basket.multiplier);
                levelstate.scoredbytes += localizedFinalBytesYield;

                CashoutParticle cp;
                cp.position = { p.position.x - 25.0f, basket.bounds.y - 25.0f };
                cp.text = "+" + FormatByteSize(localizedFinalBytesYield);
                cp.lifetime = 1.2f;
                cp.color = Config::COLOR_UI_GREEN;
                engine.particles.push_back(cp);

                engine.calculationLog = "DECRYPTED SECTOR LINK YIELDING " + FormatByteSize(localizedFinalBytesYield);
                RecycleProbeId(engine, engine.activeProbes[i].id);
                engine.activeProbes.erase(engine.activeProbes.begin() + i);
                i--;
                absorbed = true;
                break;
            }
        }

        if (absorbed) continue;

        if (p.position.y > 635.0f) {
            engine.activeProbes.erase(engine.activeProbes.begin() + i);
            i--;
        }
    }
    
    if (!clonesToSpawn.empty()) {
        engine.activeProbes.insert(engine.activeProbes.end(), clonesToSpawn.begin(), clonesToSpawn.end());
    }

    const static float waitabit = 1.5f; //target in seconds
    static float timetracker = 0.0f;
    static bool energyOrbsTriggered = false;

    if (levelstate.scoredbytes>=levelstate.TARGET_QUOTA_BYTES && engine.activeProbes.size()==0){
        timetracker += GetFrameTime();

        float waitabit = 1.6f + engine.remainingBalls * 0.12f;

        if (!energyOrbsTriggered) {
            energyOrbsTriggered = true;

            Vector2 walletTarget = { Config::walletX + 60.0f, Config::walletY + 30.0f };

            SpawnEnergyOrb({ 1080.0f, Config::scoreBlockY }, walletTarget, levelstate.reward, 0.0f);

            Vector2 launcherPos = {engine.centerApexPegPos.x,engine.centerApexPegPos.y-100};
            int cacheremainingballs = engine.remainingBalls;
            for (int i = 0; i < cacheremainingballs; i++) {
                SpawnEnergyOrb(launcherPos, walletTarget, Config::extraballsreward, 0.15f + i * 0.12f);
                engine.remainingBalls -=1;
            }
        }

        if (timetracker>=waitabit){
            engine.particles.clear();
            RequestGameStateChange(SHOP);
            timetracker = 0;
            energyOrbsTriggered = false;
            levelstate.scoredbytes = 0;
            engine.remainingBalls = levelstate.MAX_LAUNCH_CAPACITY;
        }
    }
    if (levelstate.scoredbytes<levelstate.TARGET_QUOTA_BYTES && engine.activeProbes.size()==0 && engine.remainingBalls==0){
        timetracker += GetFrameTime();
        if (timetracker>=1.5f){
            gamestate.gamestate = LOST;
            timetracker = 0;
        }
    }
    if (levelstate.scoredbytes<levelstate.TARGET_QUOTA_BYTES&&engine.activeProbes.size()==0&&engine.remainingBalls==0){ //loss condition
        timetracker += GetFrameTime(); 
        if (timetracker>=waitabit){
            gamestate.gamestate = LOST;
            timetracker = 0;
        }
    }
}


void UpdateDrawFrame(void) {
    Vector2 currentMousePos = GetMousePosition();
    if (gamestate.gamestate==GAME){
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 currentMousePos = GetMousePosition();
            for (auto& node : engine.nodes) {
                if (CheckCollisionPointCircle(currentMousePos, node.position, node.baseRadius + 24.0f)) {
                    int nextState = (int)node.modifier + 1;
                    node.modifier = (nextState > (int)MOD_CLONE) ? MOD_NONE : (ModifierType)nextState;
                    node.pulseAnimTimer = 1.0f;
                    
                    engine.calculationLog = "EASING PARAMETERS GENERATED";
                    break;
                }
            }
        }
        if (IsKeyPressed(KEY_SPACE)&&levelstate.scoredbytes<levelstate.TARGET_QUOTA_BYTES) InjectProbeFromTurret();
        UpdatePhysics(GetFrameTime());
        ProcessLineFades(engine);
    }

    BeginTextureMode(sceneTarget);
    ClearBackground(Config::COLOR_BG);
    UpdateTransition();
    UpdateDisplayedBalance();
    UpdateEnergyOrbs();
    for (auto& d : activedaemoninfo.daemons) {
        d.UpdateYAnim(GetFrameTime());
    }
    
    if (gamestate.gamestate==GAME){
        
        for (const auto& basket : engine.baskets) {
            DrawRectangleRec(basket.bounds, Config::COLOR_BASKET);
            DrawRectangleLinesEx(basket.bounds, 1.0f, Config::COLOR_GRID_LINE);
            std::string txt = std::to_string(basket.multiplier).substr(0, 3) + "x";
            DrawText(txt.c_str(), basket.bounds.x + ((basket.bounds.width - MeasureText(txt.c_str(), 10)) / 2), basket.bounds.y + 5, 10, Config::COLOR_UI_AMBER);
        }
        for (const auto& node : engine.nodes) { 
            Color basePinColor = Config::COLOR_NODE;
            if (node.modifier == MOD_BOOST) basePinColor = Config::COLOR_UI_GREEN;
            else if (node.modifier == MOD_GLITCH) basePinColor = { 255, 50, 140, 255 };
            else if (node.modifier == MOD_CLONE) basePinColor = { 200, 50, 255, 255 }; 

            if (node.pulseAnimTimer > 0.0f && node.modifier == MOD_NONE) basePinColor = Config::COLOR_PROBE;

            DrawCircleV(node.position, node.currentRadius, basePinColor);

            if (CheckCollisionPointCircle(currentMousePos, node.position, node.baseRadius  + 24.0f)) {
                DrawCircleLines(node.position.x, node.position.y, node.baseRadius + 12.0f, Config::COLOR_UI_AMBER);
            }
        }

        Vector2 turretPos = { engine.centerApexPegPos.x, engine.centerApexPegPos.y - 100.0f };
        DrawRectangle(turretPos.x - 30, turretPos.y, 60, 35, { 20, 32, 48, 255 });
        DrawRectangleLines(turretPos.x - 30, turretPos.y, 60, 35, Config::COLOR_GRID_LINE);
        
        Color muzzleFlashColor = (engine.turretBarrelFlash > 0.0f) ? Config::COLOR_UI_GREEN : Config::COLOR_BASKET;
        DrawRectangle(turretPos.x - 10, turretPos.y + 35, 20, 12, muzzleFlashColor);
        DrawRectangleLines(turretPos.x - 10, turretPos.y + 35, 20, 12, Config::COLOR_GRID_LINE);

        std::string countStr = "RESERVE: " + std::to_string(engine.remainingBalls);
        DrawText(countStr.c_str(), turretPos.x - (MeasureText(countStr.c_str(), 11)/2), turretPos.y + 11, 11, engine.remainingBalls > 0 ? Config::COLOR_PROBE : Config::COLOR_UI_AMBER);

        for (const auto& p : engine.activeProbes) {
            DrawCircleV(p.position, p.radius, WHITE);
            DrawCircleLines(p.position.x, p.position.y, p.radius + 1.0f, LIGHTGRAY);
            
            float boxW = 65.0f;
            float boxH = 15.0f;
            Vector2 boxPos = { p.position.x - (boxW / 2.0f), p.position.y - p.radius - 22.0f };
            
            DrawRectangle(boxPos.x, boxPos.y, boxW, boxH, { 6, 12, 22, 210 });
            DrawRectangleLines(boxPos.x, boxPos.y, boxW, boxH, Config::COLOR_UI_GREEN);
            DrawLine(p.position.x, boxPos.y + boxH, p.position.x, p.position.y - p.radius, Config::COLOR_UI_GREEN);
            
            std::string currentScoreFormatted = FormatByteSize(p.rawPayloadBytes);
            DrawText(currentScoreFormatted.c_str(), boxPos.x + (boxW - MeasureText(currentScoreFormatted.c_str(), 9))/2, boxPos.y + 3, 9, Config::COLOR_PROBE);
        }

        for (const auto& cp : engine.particles) {
            DrawText(cp.text.c_str(), cp.position.x, cp.position.y, 13, cp.color);
        }

        std::string coreTelemetry = "FLIGHT CONCURRENT ARCH: " + std::to_string(engine.activeProbes.size()) + " UNITS  ||  " + engine.calculationLog;
        DrawText(coreTelemetry.c_str(), 400 - (MeasureText(coreTelemetry.c_str(), 13) / 2), 652, 13, Config::COLOR_PROBE);

        std::string bottomTip = "SYS ENG: [CLICK PIN] CHANGE MODIFIERS // [SPACEBAR] INJECT LOAD PACKETS";
        DrawText(bottomTip.c_str(), 400 - (MeasureText(bottomTip.c_str(), 11) / 2), 685, 11, Config::COLOR_GRID_LINE);
        
        
        bool targetMet = (levelstate.scoredbytes >= levelstate.TARGET_QUOTA_BYTES);
        std::string quotaString = "TARGET QUOTA: " + FormatByteSize(levelstate.TARGET_QUOTA_BYTES);
        DrawText(quotaString.c_str(), 835, Config::scoreBlockY, 13, targetMet ? Config::COLOR_UI_GREEN : Config::COLOR_UI_AMBER);

        std::string rewardstr = "Reward Credits: $ " + formatWithSpaces(levelstate.reward);
        DrawText(rewardstr.c_str(), 1080, Config::scoreBlockY, 13, Config::COLOR_UI_GREEN);

        DrawText("DATA HACKED PROGRESSION TIER:", 835, Config::scoreBlockY + 24, 12, { 130, 160, 180, 255 });
        std::string dataProgressText = FormatByteSize(levelstate.scoredbytes) + " / " + FormatByteSize(levelstate.TARGET_QUOTA_BYTES);
        DrawText(dataProgressText.c_str(), 835, Config::scoreBlockY + 40, 24, targetMet ? Config::COLOR_UI_GREEN : WHITE);
        
        DrawLineEx({ 0, 630 }, { 810, 630 }, 2.0f, Config::COLOR_SHARD_BORDER);
        DrawLineEx({ 810, 0 }, { 810, 720 }, 2.0f, Config::COLOR_SHARD_BORDER);
        DrawFadingLines(engine);
    }
    if (gamestate.gamestate==SHOP){
        DrawLineEx({ 810, 0 }, { 810, 720 }, 2.0f, Config::COLOR_SHARD_BORDER);
        DrawLineEx({ 0, 530 }, { 810, 530 }, 2.0f, Config::COLOR_SHARD_BORDER);
        drawshop();
    }
    if (gamestate.gamestate==MAP){
        DrawMap();
    }
    DrawRectangle(Config::walletX, Config::walletY, 420, 65, { 16, 22, 12, 240 });
    DrawRectangleLines(Config::walletX, Config::walletY, 420, 65, Config::COLOR_SHARD_BORDER);
    DrawText("ACCOUNT STANDALONE BALANCE LEDGER:", Config::walletX+15, Config::walletY + 10, 11, Config::COLOR_NODE);
    std::string walletStr = "CREDITS: $ " + formatWithSpaces((long long)displayedBalance);
    DrawText(walletStr.c_str(), Config::walletX+15, Config::walletY + 26, 22, Config::COLOR_UI_GREEN);

    PrepDrawCyberpunkDaemonSlots();
    PrepDrawConsumableSlots();

    DrawEnergyOrbs();
    EndTextureMode();
    BeginDrawing();
        DrawGlitchedScene(sceneTarget);
    EndDrawing();
}

int main() { 
    InitWindow(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, "BITDROP");
    InitAudioDevice();
    init_sounds();
    InitGame();
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }
#endif
    CloseWindow();
    return 0;
}