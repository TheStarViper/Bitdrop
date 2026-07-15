#include "daemons.hpp"
#include "raylib.h"
#include "variables.hpp"
#include "button.hpp"
#include <cmath>
#include <utility>
#include <sstream>
#include <ctime>

void DrawCyberpunkEmptySlot(int slotIndex) {
    float x = 830.0f;
    float targetY = Config::Daemon_Y_Buffer + (slotIndex - 1) * (Config::Daemon_Slot_Spacing+76);
    DrawRectangle(x, targetY, 420, 76, Color{ 10, 16, 26, 100 });
    DrawRectangleLinesEx({ x, targetY, 420, 76 }, 1.0f, Color{ 40, 60, 80, 150 });

    DrawLineEx({ x, targetY }, { x + 10, targetY }, 1.5f, Color{ 80, 110, 130, 120 });
    DrawLineEx({ x, targetY }, { x, targetY + 10 }, 1.5f, Color{ 80, 110, 130, 120 });

    std::string emptyText = "[ EMPTY_SLOT_0" + std::to_string(slotIndex) + " ]";
    DrawText(emptyText.c_str(), x + 20, targetY + (76 / 2) - 6, 12, Color{ 60, 90, 110, 180 });
}

void PrepDrawCyberpunkDaemonSlots(){
    static bool isInitialized = false;
    if (!isInitialized) {
        activedaemoninfo.daemons.push_back(engine.daemons[0]);
        activedaemoninfo.daemons.push_back(engine.daemons[1]);
        activedaemoninfo.daemons.push_back(engine.daemons[2]);
        activedaemoninfo.daemons.push_back(engine.daemons[3]);
        activedaemoninfo.daemons.push_back(engine.daemons[4]);
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
    const int MAX_TOTAL_SLOTS = 5;
    for (int slotNum = 1; slotNum <= MAX_TOTAL_SLOTS; slotNum++) {
        bool slotOccupied = false;
        for (const auto& daemon : activedaemoninfo.daemons) {
            if (daemon.slot == slotNum) {
                slotOccupied = true;
                break;
            }
        }
        if (!slotOccupied) {
            DrawCyberpunkEmptySlot(slotNum);
        }
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
        if (lines.empty()) lines.push_back("ERROR THIS IS EMPTY");
        float textLineHeight = (font.baseSize > 0) ? font.baseSize * ((float)fontSize / font.baseSize) : fontSize;
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

        Color baseBg = Color{20, 32, 42, 255};
        int targetSlot = -1;

        
        // Up Button
        if (DrawButton(rUp, ButtonType::ArrowUp, alpha, baseBg, Config::COLOR_GRID_LINE, borderCol, textCol)) {
            if (d.slot > 0) targetSlot = d.slot - 1;
        }

        // Down Button
        if (DrawButton(rDown, ButtonType::ArrowDown, alpha, baseBg, Config::COLOR_GRID_LINE, borderCol, textCol)) {
            targetSlot = d.slot + 1;
        }

        // Sell Button
        std::string sellPriceText = "$" + std::to_string(d.getsellval());
        Color sellNormalBg = Color{45, 15, 20, 255};
        Color sellHoverBg = Color{180, 40, 40, 255};
        
        if (DrawButton(rSell, ButtonType::TextGeneric, alpha, sellNormalBg, sellHoverBg, amberCol, textCol, sellPriceText.c_str(), 11)) {
            if (daemonidx < activedaemoninfo.daemons.size()) {
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
    //2x points every 5 hits
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

void schrodingers_basket(Daemon& self, Probe& probe) {
    // "schrodingers_basket" - score has a 50% chance of 3x mult and a 50% chance of .5x mult
    addfadeline(self, probe);
    if ((rand() % 100) < 50) {
        probe.rawPayloadBytes *= 3.0f;
    } else {
        probe.rawPayloadBytes *= 0.5f;
    }
}

void dark_web_node(Daemon& self, Probe& probe) {
    // "dark_web_node" - score has a 5% chance of 15x score
    if ((rand() % 100) < 5) {
        addfadeline(self, probe);
        probe.rawPayloadBytes *= 20; // 15x Hyper-Crit
    }
}

void kill_switch(Daemon& self, Probe& probe) {
    // last ball you score does x4 mult
    if (engine.activeProbes.size() == 1&&engine.remainingBalls==0) {
        addfadeline(self, probe);
        probe.rawPayloadBytes *= 4.0f;
    }
}

void initdaemons(){
    float slotYStart = 15.0f;
    float slotSpacing = 10.0f;
    engine.daemons = {
        //             name,                   subtitle,      description,          color ,levels, price, icon, trigger type, function pointer
        Daemon("TESTDAEMON_WITH ABILITY", "SECURE // SYNCED", "111x DATA", Config::COLOR_PROBE, 3,900,&ICON_PADLOCK,PASSIVE,testdaemon),
        Daemon("TESTDAEMON2 WITH ABILITY", "STANDBY RUNTIME", "+10mb per pin hit", Config::COLOR_UI_GREEN, 3,900,&ICON_PADLOCK,PINS,testdaemon2),
        Daemon("Loyalty Points", "CRITICAL OVERLOAD", "2x points every 5 hits", Config::COLOR_UI_AMBER, 3,900,&ICON_PADLOCK,PINS,loyalty_points),
        Daemon("Mesh Network", "SECURE // SYNCED", "1.5x score for each ball in play", Config::MAGENTA_DAEMON, 3,900,&ICON_PADLOCK,PASSIVE,mesh_network),
        Daemon("Schrodinger's Basket", "????????", "score has either gets 2.5x points or .5x points on score", Config::OTHER_COLOR_FOR_DAEMONS, 3,900,&ICON_PADLOCK,PASSIVE,schrodingers_basket),
        Daemon("Dark Web Node", "danger awaits", "5% chance to score 15x data", Config::MAGENTA_DAEMON, 3,900,&ICON_PADLOCK,PASSIVE,dark_web_node),
        Daemon("Kill Switch", "FAILSAFE", "4x output multiplier if down to last active unit", Config::COLOR_UI_AMBER, 3,900,&ICON_PADLOCK,PASSIVE,kill_switch)
    };
}

//
//overclocking requires overclocking shard 1x then 2x then 3x and so on
//empty slots
//locked slots
//daemon ideas:
// "Mitosis" - score is no longer split between daughter probes
// "einstein's cradle" - when two balls hit each other +20
// "cold storage" - space bar sets bullet time and clicking a ball freezes it's position and drops another ball, if that ball collides into the frozen ball before scoring multiply both balls by 3x
// bouncy
// somethin else 