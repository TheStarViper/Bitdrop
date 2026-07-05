#include "raylib.h"
#include "main.hpp"
#include <vector>
#include <cmath>
#include <string>
#include <iomanip>
#include <sstream>
#include "daemons.hpp"

#if defined(PLATFORM_WEB)
    #include <emscripten.h>
#endif

   
const int MAX_LAUNCH_CAPACITY = 45;     
const int STARTING_CREDITS = 85000;

const float KINETIC_RESTITUTION = 0.20f; 
const float FRICTION_DAMPING = 0.95f;  

const double TARGET_QUOTA_BYTES = 52428800.0; 

std::string FormatByteSize(double bytes) {
    if (bytes < 1024.0) return "0 KB";
    const char* suffixes[] = { "KB", "MB", "GB", "TB" , "EB", "ZB", "YB", "RB", "QB"};
    int i = 0;
    double size = bytes / 1024.0;
    while (size >= 1024.0 && i < 8) {
        size /= 1024.0;
        i++;
    }
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << size << " " << suffixes[i];
    return stream.str();
}

GameEngine engine;


void InitGame() {
    engine.globalDataHackedBytes = 0.0;
    engine.remainingBalls = MAX_LAUNCH_CAPACITY;
    engine.turretBarrelFlash = 0.0f;
    engine.calculationLog = "CORE ARMED: DATA METERS ROUTED TO KB MINIMUMS";
    engine.daemons.clear();
    int totalRows = 10;
    float startY = 145.0f;
    float spacingY = 48.0f; 
    float spacingX = 64.0f; 

    for (int r = 0; r < totalRows; ++r) {
        int cols = 1 + r; 
        float startX = 400.0f - ((cols - 1) * spacingX) / 2.0f;
        for (int c = 0; c < cols; ++c) {
            Node n;
            n.position = { startX + (c * spacingX), startY + (r * spacingY) };
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

    int finalRowCols = 10;
    float finalRowStartX = 400.0f - ((finalRowCols - 1) * spacingX) / 2.0f;
    float basketY = 600.0f; 
    float basketW = spacingX - 8.0f; 

    for (int i = 0; i < finalRowCols + 1; ++i) {
        Basket b;
        float bx = (finalRowStartX - (spacingX / 2.0f)) + (i * spacingX) - (basketW / 2.0f);
        b.bounds = { bx, basketY, basketW, 20.0f };
        
        int factor = (i == 0 || i == finalRowCols) ? 1 : (i % 2 == 0) ? 2 : 4;
        if (i == finalRowCols / 2) factor = 8; 
        
        b.name = "PORT_" + std::to_string(i + 1);
        b.multiplier = factor * 0.5f;
        engine.baskets.push_back(b);
    }

    float slotYStart = 15.0f;
    float slotHeight = 76.0f;
    float slotSpacing = 10.0f;
    
    
    engine.daemons = {
        Daemon(830.0f, slotYStart, 420.0f, slotHeight, "NETRUNNER_DECK_01", "SECURE // SYNCED", "PLACEHOLDER LORUM IPSUM WHATEVER HERE", Config::COLOR_PROBE, 3),
        Daemon(830.0f, slotYStart + (slotHeight + slotSpacing), 420.0f, slotHeight, "ICEBREAKER", "STANDBY RUNTIME", "PLACEHOLDER LORUM IPSUM WHATEVER HERE", Config::COLOR_UI_GREEN, 3),
        Daemon(830.0f, slotYStart + 2*(slotHeight + slotSpacing), 420.0f, slotHeight, "OVERCLOCK_BUFFER", "CRITICAL OVERLOAD", "PLACEHOLDER LORUM IPSUM WHATEVER HERE", Config::COLOR_UI_AMBER, 3),
        Daemon(830.0f, slotYStart + 3*(slotHeight + slotSpacing), 420.0f, slotHeight, "BLACK_WALL_GATE", "RESTRICTED THREAD", "PLACEHOLDER LORUM IPSUM WHATEVER HERE", Config::COLOR_BASKET, 3),
        Daemon(830.0f, slotYStart + 4*(slotHeight + slotSpacing), 420.0f, slotHeight, "MALWARE_SINK.IO", "HONEYPOT ACTIVE", "PLACEHOLDER LORUM IPSUM WHATEVER HERE", Config::OTHER_COLOR_FOR_DAEMONS, 3)
    };

    engine.daemons[0].SetFillPct(85);
    engine.daemons[1].SetFillPct(40);
    engine.daemons[2].SetFillPct(100); engine.daemons[2].SetOverclock(5);
    engine.daemons[3].SetFillPct(15);
    engine.daemons[4].SetFillPct(65);
}

void InjectProbeFromTurret() {
    if (engine.remainingBalls <= 0) return; 

    engine.remainingBalls--;
    engine.turretBarrelFlash = 0.12f;

    Probe p;
    p.id = MAX_LAUNCH_CAPACITY - engine.remainingBalls;
    float variance = (float)GetRandomValue(-12, 12);
    p.position = { engine.centerApexPegPos.x + variance, engine.centerApexPegPos.y - 80.0f };
    p.velocity = { (float)GetRandomValue(-8, 8), 100.0f };
    p.radius = 9.0f;
    p.hitCount = 0;
    p.rawPayloadBytes = 0.0;
    p.bufferRate = 1.0f;
    p.lastHitNodeIndex = -1; 

    engine.activeProbes.push_back(p);
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
                    p1.velocity.x -= p * normal.x * KINETIC_RESTITUTION;
                    p1.velocity.y -= p * normal.y * KINETIC_RESTITUTION;
                    p2.velocity.x += p * normal.x * KINETIC_RESTITUTION;
                    p2.velocity.y += p * normal.y * KINETIC_RESTITUTION;
                }
            }
        }
    }

    for (size_t i = 0; i < engine.activeProbes.size(); i++) {
        Probe& p = engine.activeProbes[i];
        
        p.velocity.y += Config::GRAVITY * scaledDt;
        p.position.x += p.velocity.x * scaledDt;
        p.position.y += p.velocity.y * scaledDt;

        p.velocity.x *= std::pow(FRICTION_DAMPING, scaledDt * 60.0f); 

        if (p.position.x - p.radius < 10.0f) {
            p.position.x = 10.0f + p.radius;
            p.velocity.x *= -KINETIC_RESTITUTION;
        }
        if (p.position.x + p.radius > 790.0f) {
            p.position.x = 790.0f - p.radius;
            p.velocity.x *= -KINETIC_RESTITUTION;
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
                p.velocity.x -= (1.0f + KINETIC_RESTITUTION) * dot * normal.x;
                p.velocity.y -= (1.0f + KINETIC_RESTITUTION) * dot * normal.y;
                p.velocity.x += (float)GetRandomValue(-10, 10) * 0.25f;
                if ((int)nIdx != p.lastHitNodeIndex) {
                    p.lastHitNodeIndex = (int)nIdx; 
                    node.pulseAnimTimer = 1.0f;
                    
                    p.hitCount++;
                    
                    double calculatedByteBump = 1024.0;
                    if (node.modifier == MOD_BOOST) calculatedByteBump *= 2.5;
                    else if (node.modifier == MOD_GLITCH) calculatedByteBump *= ((float)GetRandomValue(5, 50) * 0.2f);
                    
                    p.rawPayloadBytes += calculatedByteBump;
                    p.bufferRate += (node.modifier == MOD_GLITCH ? 0.35f : 0.12f);

                    if (node.modifier == MOD_CLONE) {
                        float pushOffset = p.radius + node.baseRadius + 4.0f; 
                        float speedSnap = fabsf(p.velocity.x) > 10.0f ? fabsf(p.velocity.x) : 80.0f;

                        Probe cloneL = p;
                        cloneL.id = p.id * 100 + GetRandomValue(1, 99);
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

        bool absorbed = false;
        for (const auto& basket : engine.baskets) {
            if (CheckCollisionCircleRec(p.position, p.radius, basket.bounds)) {
                double localizedFinalBytesYield = p.rawPayloadBytes * basket.multiplier;
                
                engine.globalDataHackedBytes += localizedFinalBytesYield;

                CashoutParticle cp;
                cp.position = { p.position.x - 25.0f, basket.bounds.y - 25.0f };
                cp.text = "+" + FormatByteSize(localizedFinalBytesYield);
                cp.lifetime = 1.2f;
                cp.color = Config::COLOR_UI_GREEN;
                engine.particles.push_back(cp);

                engine.calculationLog = "DECRYPTED SECTOR LINK YIELDING " + FormatByteSize(localizedFinalBytesYield);
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
}

void UpdateDrawFrame(void) {
    Vector2 currentMousePos = GetMousePosition();

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
    
    if (IsKeyPressed(KEY_SPACE)) InjectProbeFromTurret();
    UpdatePhysics(GetFrameTime());

    BeginDrawing();
    ClearBackground(Config::COLOR_BG);

    DrawLineEx({ 810, 0 }, { 810, 720 }, 2.0f, Config::COLOR_SHARD_BORDER);
    DrawLineEx({ 0, 630 }, { 810, 630 }, 2.0f, Config::COLOR_SHARD_BORDER);

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

        if (CheckCollisionPointCircle(currentMousePos, node.position, node.baseRadius + 24.0f)) {
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
        
        float boxW = 54.0f;
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
    static int localSelectedDaemonIndex = -1;
    float scaledDt = GetFrameTime() * Config::GAME_SPEED;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mPos = GetMousePosition();
        bool clickedCard = false;
        
        for (size_t i = 0; i < engine.daemons.size(); i++) {
            auto& d = engine.daemons[i];
            if (CheckCollisionPointRec(mPos, { d.x, d.y, d.width, d.height })) {
                localSelectedDaemonIndex = (int)i;
                clickedCard = true;
                break;
            }
        }
        if (!clickedCard) {
            localSelectedDaemonIndex = -1;
        }
    }
    for (size_t i = 0; i < engine.daemons.size(); i++) {
        bool isSelected = (localSelectedDaemonIndex == (int)i);
        engine.daemons[i].UpdateExpansion(scaledDt, isSelected);
        engine.daemons[i].ExecuteRoutine(engine, scaledDt);
    }
    for (size_t i = 0; i < engine.daemons.size(); i++) {
        bool isSelected = (localSelectedDaemonIndex == (int)i); 
        DrawCyberpunkDaemonSlot(engine.daemons[i], currentMousePos, isSelected);
    }
    
    std::string coreTelemetry = "FLIGHT CONCURRENT ARCH: " + std::to_string(engine.activeProbes.size()) + " UNITS  ||  " + engine.calculationLog;
    DrawText(coreTelemetry.c_str(), 400 - (MeasureText(coreTelemetry.c_str(), 13) / 2), 652, 13, Config::COLOR_PROBE);

    std::string bottomTip = "SYS ENG: [CLICK PIN] CHANGE MODIFIERS // [SPACEBAR] INJECT LOAD PACKETS";
    DrawText(bottomTip.c_str(), 400 - (MeasureText(bottomTip.c_str(), 11) / 2), 685, 11, Config::COLOR_GRID_LINE);

    float scoreBlockY = 455.0f;
    bool targetMet = (engine.globalDataHackedBytes >= TARGET_QUOTA_BYTES);
    
    std::string quotaString = "TARGET QUOTA: " + FormatByteSize(TARGET_QUOTA_BYTES);
    DrawText(quotaString.c_str(), 835, scoreBlockY, 13, targetMet ? Config::COLOR_UI_GREEN : Config::COLOR_UI_AMBER);

    DrawText("DATA HACKED PROGRESSION TIER:", 835, scoreBlockY + 24, 12, { 130, 160, 180, 255 });
    std::string dataProgressText = FormatByteSize(engine.globalDataHackedBytes) + " / " + FormatByteSize(TARGET_QUOTA_BYTES);
    DrawText(dataProgressText.c_str(), 835, scoreBlockY + 40, 24, targetMet ? Config::COLOR_UI_GREEN : WHITE);

    float walletY = 565.0f;
    DrawRectangle(830, walletY, 420, 65, { 16, 22, 12, 240 });
    DrawRectangleLines(830, walletY, 420, 65, Config::COLOR_SHARD_BORDER);
    DrawText("ACCOUNT STANDALONE BALANCE LEDGER (FROZEN):", 845, walletY + 10, 11, Config::COLOR_NODE);
    std::string walletStr = "CREDITS: ฿ " + std::to_string(STARTING_CREDITS);
    DrawText(walletStr.c_str(), 845, walletY + 26, 22, Config::COLOR_UI_GREEN);

    EndDrawing();
}

int main() { 
    InitWindow(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, "BITDROP");
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