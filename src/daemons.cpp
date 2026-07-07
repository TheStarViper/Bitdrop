#include "daemons.hpp"
#include "raylib.h"
#include "variables.hpp"
#include <cmath>
#include <utility>


void PrepDrawCyberpunkDaemonSlots(){
    static bool isInitialized = false;
    if (!isInitialized) {
        activedaemoninfo.daemons = engine.daemons;
        activedaemoninfo.daemons[0].slot = 1;
        activedaemoninfo.daemons[1].slot = 2;
        activedaemoninfo.daemons[2].slot = 3;
        activedaemoninfo.daemons[3].slot = 4;
        activedaemoninfo.daemons[4].slot = 5;
        activedaemoninfo.daemons[0].updateYPosition();
        activedaemoninfo.daemons[1].updateYPosition();
        activedaemoninfo.daemons[2].updateYPosition();
        activedaemoninfo.daemons[3].updateYPosition();
        activedaemoninfo.daemons[4].updateYPosition();
        isInitialized = true;
    }

    static int localSelectedDaemonIndex = -1;
    float scaledDt = GetFrameTime() * Config::GAME_SPEED;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mPos = GetMousePosition();
        bool clickedCard = false;
        
        for (size_t i = 0; i < activedaemoninfo.daemons.size(); i++) {
            auto& d = activedaemoninfo.daemons[i];
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
    
    for (size_t i = 0; i < activedaemoninfo.daemons.size(); i++) {
        bool isSelected = (localSelectedDaemonIndex == (int)i);
        activedaemoninfo.daemons[i].ExecuteRoutine(engine, scaledDt);
        activedaemoninfo.daemons[i].UpdateExpansion(scaledDt, isSelected);
        DrawCyberpunkDaemonSlot(activedaemoninfo.daemons[i], GetMousePosition(), isSelected, (int)i,&localSelectedDaemonIndex);
        activedaemoninfo.daemons[i].TriggerAction();
    }
    
}

void DrawCyberpunkDaemonSlot(const Daemon& d, Vector2 mousePos, bool isSelected, int daemonidx, int* selectedDaemonIndex) {
    bool isHovered = CheckCollisionPointRec(mousePos, { d.x, d.y, d.width, d.height });
    Color currentBg = isHovered ? Color{ 16, 26, 42, 255 } : Color{ 10, 16, 26, 240 };
    Color borderColor = isSelected ? Config::COLOR_UI_AMBER : (isHovered ? Config::COLOR_UI_GREEN : Config::COLOR_SHARD_BORDER);
    
    Color levelcolor = (d.IsOverclocked()) ? Config::COLOR_OVERCLOCKED : d.GetColor();

    DrawRectangle(d.x, d.y, d.width, d.height, currentBg);
    DrawRectangleLinesEx({ d.x, d.y, d.width, d.height }, isSelected ? 1.5f : 1.0f, borderColor);
    
    DrawLineEx({ d.x, d.y }, { d.x + 20, d.y }, 2.5f, d.GetColor());
    DrawLineEx({ d.x, d.y }, { d.x, d.y + 20 }, 2.5f , d.GetColor());
    DrawLineEx({ d.x + d.width - 20, d.y + d.height }, { d.x + d.width, d.height + d.y }, 2.5f, d.GetColor());
    
    
    //level bar
    const int level_bar_x = d.x + d.width - 85;
    const int level_bar_y = d.y + 20;

    float barW = 70.0f;
    DrawRectangle(level_bar_x, level_bar_y, barW, 7, { 20, 35, 45, 255 });
    DrawRectangle(level_bar_x, level_bar_y, barW * (d.GetLevel() / (float)d.getmaxlevel()), 7, levelcolor);

    //overclocked indicator
    if (d.IsOverclocked()) {
        if (d.getoverclocklvl()>=2){
            std::string lvlnumstr = std::to_string(d.getoverclocklvl())+"x";
            DrawText(lvlnumstr.c_str(),d.x+ d.width - 190, level_bar_y-1, 9, Config::COLOR_OVERCLOCKED);
        }
        DrawText("OVERCLOCKED",d.x+ d.width - 172, level_bar_y-1, 9, Config::COLOR_OVERCLOCKED);
    } else {
        std::string lvlStr = "LVL " + std::to_string(d.GetLevel());
        DrawText(lvlStr.c_str(), d.x + d.width - 125, level_bar_y-2, 11, Config::COLOR_PROBE);
    }
    DrawRectangle(d.x + 3, d.y + 3, 5, d.height - 6, levelcolor);


    //titles and tech gibberish
    DrawText(d.GetName().c_str(), d.x + 20, d.y + 16, 15, d.GetColor());
    DrawText(d.status.c_str(), d.x + 20, d.y + 40, 11, { 110, 140, 160, 255 });


    if (isHovered) {
        float boxW = 280.0f;
        float boxH = 50.0f;
        float boxX = d.x - boxW - 10.0f; 
        float boxY = d.y + (d.height - boxH) / 2.0f;

        DrawRectangle(boxX, boxY, boxW, boxH, { 6, 12, 22, 250 });
        DrawRectangleLinesEx({ boxX, boxY, boxW, boxH }, 1.0f, Config::COLOR_UI_GREEN);
        DrawRectangle(boxX + 1, boxY + 1, 3, boxH - 2, Config::COLOR_UI_GREEN); 
        DrawText(d.GetDesc().c_str(), boxX + 12, boxY + 18, 11, Config::COLOR_PROBE);
    }

    float expansion = d.GetExpansion();
    
    if (expansion > 0.01f) {
        float easeProgress = 1.0f - powf(1.0f - expansion, 3.0f);
        
        unsigned char alpha = (unsigned char)(easeProgress * 255);
        Color textCol = { 255, 255, 255, alpha };
        Color amberCol = { Config::COLOR_UI_AMBER.r, Config::COLOR_UI_AMBER.g, Config::COLOR_UI_AMBER.b, alpha };
        Color borderCol = { Config::COLOR_SHARD_BORDER.r, Config::COLOR_SHARD_BORDER.g, Config::COLOR_SHARD_BORDER.b, alpha };

        float slideOffset = (1.0f - easeProgress) * 15.0f;
        float bY = (d.y + d.height - 32.0f) + slideOffset;
        
        Rectangle rUp = { d.x + d.width - 150, bY, 30, 22 };
        Rectangle rDown = { d.x + d.width - 115, bY, 30, 22 };
        Rectangle rSell = { d.x + d.width - 80, bY, 70, 22 };

        Vector2 mouse = GetMousePosition();

        Color upBg = CheckCollisionPointRec(mouse, rUp) ? Config::COLOR_GRID_LINE : Color{20, 32, 42, 255};
        upBg.a = alpha;
        DrawRectangleRec(rUp, upBg);
        DrawRectangleLinesEx(rUp, 1.0f, borderCol);

        
        Vector2 upV1 = { rUp.x + rUp.width / 2.0f,  rUp.y + 5.0f };
        Vector2 upV2 = { rUp.x + 8.0f,              rUp.y + rUp.height - 5.0f };
        Vector2 upV3 = { rUp.x + rUp.width - 8.0f,  rUp.y + rUp.height - 5.0f };
        DrawTriangle(upV1, upV2, upV3, textCol);


        Color downBg = CheckCollisionPointRec(mouse, rDown) ? Config::COLOR_GRID_LINE : Color{20, 32, 42, 255};
        downBg.a = alpha;
        DrawRectangleRec(rDown, downBg);
        DrawRectangleLinesEx(rDown, 1.0f, borderCol);

        
        Vector2 downV1 = { rDown.x + 8.0f,              rDown.y + 5.0f };
        Vector2 downV2 = { rDown.x + rDown.width / 2.0f,  rDown.y + rDown.height - 5.0f };
        Vector2 downV3 = { rDown.x + rDown.width - 8.0f,  rDown.y + 5.0f };
        DrawTriangle(downV1, downV2, downV3, textCol);

        std::string sellText = "$" + std::to_string(d.getsellval());
        
        Color sellBg = CheckCollisionPointRec(mouse, rSell) ? Color{180, 40, 40, 255} : Color{45, 15, 20, 255};
        sellBg.a = alpha;
        DrawRectangleRec(rSell, sellBg);
        DrawRectangleLinesEx(rSell, 1.0f, amberCol);
        DrawText("SELL", rSell.x + 6, rSell.y + 5, 11, amberCol);
        DrawText(sellText.c_str(), rSell.x + 36, rSell.y + 6, 10, textCol);

        int targetSlot = -1;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mouse, rUp) && d.slot > 0) {
                targetSlot = d.slot - 1;
            } else if (CheckCollisionPointRec(mouse, rDown)) {
                targetSlot = d.slot + 1;
            } else if (CheckCollisionPointRec(mouse, rSell) && daemonidx < activedaemoninfo.daemons.size()) {
                int cached_slot = d.slot;
                gamestate.balance += d.getsellval();
                std::swap(activedaemoninfo.daemons[daemonidx], activedaemoninfo.daemons.back());
                activedaemoninfo.daemons.pop_back();
                *selectedDaemonIndex = -1;
                for (auto& daemon : activedaemoninfo.daemons) {
                    if (daemon.slot > cached_slot) {
                        daemon.slot--;
                        daemon.updateYPosition();
                    }
                }
            }
        }

        if (targetSlot != -1) {
            for (size_t i = 0; i < activedaemoninfo.daemons.size(); ++i) {
                if (activedaemoninfo.daemons[i].slot == targetSlot) {
                    activedaemoninfo.daemons[i].slot = d.slot;
                    activedaemoninfo.daemons[daemonidx].slot = targetSlot;

                    activedaemoninfo.daemons[i].updateYPosition();
                    activedaemoninfo.daemons[daemonidx].updateYPosition();

                    std::swap(activedaemoninfo.daemons[daemonidx], activedaemoninfo.daemons[i]);
                    *selectedDaemonIndex = (int)i;
                    break;
                }
            }
        }
    }
}

void initdaemons(){
    float slotYStart = 15.0f;
    float slotSpacing = 10.0f;
    engine.daemons = {
        Daemon("NETRUNNER_DECK_01", "SECURE // SYNCED", "PLACEHOLDER LORUM IPSUM WHATEVER HERE", Config::COLOR_PROBE, 3,932,PASSIVE,testdaemon),
        Daemon("ICEBREAKER", "STANDBY RUNTIME", "PLACEHOLDER LORUM IPSUM WHATEVER HERE", Config::COLOR_UI_GREEN, 3,453,PASSIVE),
        Daemon("OVERCLOCK_BUFFER", "CRITICAL OVERLOAD", "PLACEHOLDER LORUM IPSUM WHATEVER HERE", Config::COLOR_UI_AMBER, 3, 4323,PASSIVE),
        Daemon("BLACK_WALL_GATE", "RESTRICTED THREAD", "PLACEHOLDER LORUM IPSUM WHATEVER HERE", Config::COLOR_BASKET, 3, 543,PASSIVE),
        Daemon("MALWARE_SINK.IO", "HONEYPOT ACTIVE", "PLACEHOLDER LORUM IPSUM WHATEVER HERE", Config::OTHER_COLOR_FOR_DAEMONS, 3, 123,PASSIVE)
    };  
}

void testdaemon(Daemon& self){
    TraceLog(LOG_INFO,"daemontest");
}
//overclocking requires overclocking shard 1x then 2x then 3x and so on
//empty slots
//locked slots
//daemon ideas:
// "Mitosis" - score is no longer split between daughter probes
// "Mesh Network" - score is multiplied by 1.5x for each probe in play instead of 1.2x
// "Loyalty Points" - score is multiplied by 2x every 5 hits
// "einstein's cradle" - when two balls hit each other +20
// bouncy
// somethin else 