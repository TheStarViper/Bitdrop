#include "daemons.hpp"
#include "raylib.h"
#include "variables.hpp"
#include <cmath>
#include <utility>
#include <sstream>
#include <ctime>

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
        float paddingX = 12.0f;
        float paddingY = 14.0f;
        
        Font font = GetFontDefault();
        float fontSize = 11.0f;
        float spacing = 1.0f;
        std::string text = d.GetDesc();
        
        float maxTextWidth = boxW - (paddingX * 2.0f);
        std::vector<std::string> lines;
        std::string currentLine = "";
        std::string word = "";
        std::stringstream ss(text);
        
        while (ss >> word) {
            std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
            Vector2 size = MeasureTextEx(font, testLine.c_str(), fontSize, spacing);
            
            if (size.x > maxTextWidth) {
                if (!currentLine.empty()) lines.push_back(currentLine);
                currentLine = word;
            } else {
                currentLine = testLine;
            }
        }
        if (!currentLine.empty()) lines.push_back(currentLine);
        if (lines.empty()) lines.push_back("");
        float textLineHeight = (font.baseSize > 0) ? font.baseSize * (fontSize / font.baseSize) : fontSize;
        float totalTextHeight = (lines.size() * textLineHeight) + ((lines.size() - 1) * (textLineHeight * 0.5f)); 
        float boxH = totalTextHeight + (paddingY * 2.0f);
        
        float boxX = d.x - boxW - 10.0f; 
        float boxY = d.y + (d.height - boxH) / 2.0f;

        DrawRectangle(boxX, boxY, boxW, boxH, { 6, 12, 22, 250 });
        DrawRectangleLinesEx({ boxX, boxY, boxW, boxH }, 1.0f, Config::COLOR_UI_GREEN);
        DrawRectangle(boxX + 1, boxY + 1, 3, boxH - 2, Config::COLOR_UI_GREEN); 

        float currentY = boxY + paddingY;
        for (const auto& line : lines) {
            DrawTextEx(font, line.c_str(), { boxX + paddingX, currentY }, fontSize, spacing, Config::COLOR_PROBE);
            currentY += textLineHeight * 1.5f; 
        }
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


void ProcessLineFades(GameEngine& eng) {
    float dt = GetFrameTime(); 
    GameEngine* engPtr = &eng;

    for (int i = engPtr->fadingLines.size() - 1; i >= 0; i--) {
        engPtr->fadingLines[i].alpha -= engPtr->fadingLines[i].fadeSpeed * dt;

        if (engPtr->fadingLines[i].alpha <= 0.0f) {
            engPtr->fadingLines.erase(engPtr->fadingLines.begin() + i);
        }
    }
}

void DrawFadingLines(const GameEngine& eng) {
    for (const auto& line : eng.fadingLines) {
        Color fadedColor = Fade(line.color, line.alpha);
        DrawLineV(line.start, line.end, fadedColor); 
    }
}

void addfadeline(Daemon& self, Probe& probe){
    int random_num = self.y + GetRandomValue(1, 76); 
    FadeLine line;
    line.start = probe.position;
    line.end = { (float)self.x, (float)random_num };
    line.color = self.GetColor();
    line.alpha = 1.0f;
    line.fadeSpeed = 2.0f;
    engine.fadingLines.push_back(line);
}

//Daemons
void testdaemon(Daemon& self, Probe& probe) {
    addfadeline(self,probe);
    probe.rawPayloadBytes *= 111;
}

void testdaemon2(Daemon& self, Probe& probe) {
    addfadeline(self,probe);
    probe.rawPayloadBytes +=10240;
    //probe.rawPayloadBytes *=1024342212312331212312312312321352342342354234234322113201232133122133123.0L; //testing large number
}

void loyalty_points(Daemon& self, Probe& probe){
    static int counter = 0;
    if (counter<=4){counter++;}
    if (counter ==5){
        addfadeline(self,probe);
        probe.rawPayloadBytes *=2;
        counter = 0;
    }
}

void mesh_network(Daemon& self, Probe& probe){
    // "Mesh Network" - score is multiplied by 1.5x for each probe in play instead of 1.2x
    int multiplier = engine.activeProbes.size()-1;
    if (multiplier>0){
        addfadeline(self,probe);
        probe.rawPayloadBytes *=std::pow(1.5, multiplier);
    }
}

void initdaemons(){
    float slotYStart = 15.0f;
    float slotSpacing = 10.0f;
    engine.daemons = {
        Daemon("TESTDAEMON_WITH ABILITY", "SECURE // SYNCED", "111x DATA", Config::COLOR_PROBE, 3,900,&ICON_PADLOCK,PASSIVE,testdaemon),
        Daemon("TESTDAEMON2 WITH ABILITY", "STANDBY RUNTIME", "+10kb per pin hit", Config::COLOR_UI_GREEN, 3,900,&ICON_PADLOCK,PINS,testdaemon2),
        Daemon("Loyalty Points", "CRITICAL OVERLOAD", "2x points every 5 hits", Config::COLOR_UI_AMBER, 3,900,&ICON_PADLOCK,PINS,loyalty_points),
        Daemon("Mesh Network", "SECURE // SYNCED", "1.5x score for each ball in play", Config::MAGENTA_DAEMON, 3,900,&ICON_PADLOCK,PASSIVE,mesh_network),
        Daemon("MALWARE_SINK.IO", "HONEYPOT ACTIVE", "PLACEHOLDER LORUM IPSUM WHATEVER HERE", Config::OTHER_COLOR_FOR_DAEMONS, 3,900,&ICON_PADLOCK,PASSIVE,loyalty_points)
    };
}

//animate scoring so it goes onto a belt and goes through the daemons
//overclocking requires overclocking shard 1x then 2x then 3x and so on
//empty slots
//locked slots
//daemon ideas:
// "Mitosis" - score is no longer split between daughter probes
// "einstein's cradle" - when two balls hit each other +20
// "cold storage" - space bar sets bullet time and clicking a ball freezes it's position and drops another ball, if that ball collides into the frozen ball before scoring multiply both balls by 3x
// bouncy
// somethin else 