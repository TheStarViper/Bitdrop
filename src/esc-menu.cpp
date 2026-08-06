#include "esc-menu.hpp"
#include "raylib.h"
#include "variables.hpp"
#include "button.hpp"
#include "transition.hpp"
#include "animation-timer.hpp"
#include "audio.hpp"
#include "map.hpp"
#include "formatting.hpp"
#include "variables.hpp"
#include "consumables.hpp"

struct SpeedNotch {
    float multiplier;
    float rawValue;
};

const std::vector<SpeedNotch>& GetGameSpeedNotches() {
    static const std::vector<SpeedNotch> notches = {
        {0.25f, 0.25f * Config::GAME_SPEED_BASE},
        {0.5f,  0.5f  * Config::GAME_SPEED_BASE},
        {1.0f,  1.0f  * Config::GAME_SPEED_BASE},
        {2.0f,  2.0f  * Config::GAME_SPEED_BASE},
        {3.0f,  3.0f  * Config::GAME_SPEED_BASE},
        {4.0f,  4.0f  * Config::GAME_SPEED_BASE},
        {5.0f,  5.0f  * Config::GAME_SPEED_BASE}
    };
    return notches;
}

int GetNearestNotchIndex(const std::vector<SpeedNotch>& notches, float rawValue) {
    int bestIndex = 0;
    float bestDist = fabsf(notches[0].rawValue - rawValue);
    for (int i = 1; i < (int)notches.size(); i++) {
        float dist = fabsf(notches[i].rawValue - rawValue);
        if (dist < bestDist) {
            bestDist = dist;
            bestIndex = i;
        }
    }
    return bestIndex;
}

void DrawNotchedSlider(Rectangle sliderBar, float* value, const std::vector<SpeedNotch>& notches, Vector2 mousePos) {
    DrawRectangleRec(sliderBar, Color{ 30, 40, 50, 255 });

    int notchCount = (int)notches.size();
    int currentIndex = GetNearestNotchIndex(notches, *value);

    for (int i = 0; i < notchCount; i++) {
        float t = (float)i / (float)(notchCount - 1);
        float tickX = sliderBar.x + sliderBar.width * t;
        DrawLineEx({ tickX, sliderBar.y - 4.0f }, { tickX, sliderBar.y + sliderBar.height + 4.0f }, 2.0f, Color{ 60, 80, 90, 255 });
    }

    float filledT = (float)currentIndex / (float)(notchCount - 1);
    DrawRectangle(sliderBar.x, sliderBar.y, sliderBar.width * filledT, sliderBar.height, Config::COLOR_UI_GREEN);

    float handleT = (float)currentIndex / (float)(notchCount - 1);
    float handleX = sliderBar.x + sliderBar.width * handleT;
    DrawCircleV({ handleX, sliderBar.y + sliderBar.height / 2.0f }, 8.0f, Config::COLOR_UI_GREEN);
    DrawCircleLines(handleX, sliderBar.y + sliderBar.height / 2.0f, 8.0f, WHITE);

    Rectangle handleHit = { sliderBar.x - 6, sliderBar.y - 8, sliderBar.width + 12, sliderBar.height + 16 };
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, handleHit)) {
        float rawT = Clamp((mousePos.x - sliderBar.x) / sliderBar.width, 0.0f, 1.0f);
        int hoveredIndex = (int)roundf(rawT * (notchCount - 1));
        hoveredIndex = (int)Clamp((float)hoveredIndex, 0.0f, (float)(notchCount - 1));
        *value = notches[hoveredIndex].rawValue;
    }

    for (int i = 0; i < notchCount; i++) {
        float t = (float)i / (float)(notchCount - 1);
        float tickX = sliderBar.x + sliderBar.width * t;
        float m = notches[i].multiplier;
        std::string label = (m == (int)m) ? std::to_string((int)m) + "x" : "." + std::to_string((int)(m * 100)) + "x";
        int labelW = MeasureText(label.c_str(), 10);
        Color labelColor = (i == currentIndex) ? Config::COLOR_UI_GREEN : Color{ 100, 120, 130, 255 };
        DrawText(label.c_str(), tickX - labelW / 2.0f, sliderBar.y + sliderBar.height + 8.0f, 10, labelColor);
    }
}

std::vector<StatEntry>& GetStatsStats() {
    static std::vector<StatEntry> statslist = {
        {"Total Earned", []() { return "$" + formatWithSpaces(stats.money_earned); }}, //FIX
        {"Balls Dropped", []() { return formatWithSpaces(stats.balls_dropped); }},
        {"Items Bought", []() { return "$" + formatWithSpaces(stats.items_bought); }},
        {"Highest Score", []() { return FormatByteSize(stats.highest_score); }}, // FIX
        {"Daemons Triggered", []() { return formatWithSpaces(stats.daemons_triggered); }} //FIX
    };
    return statslist;
}

void drawstatsmenu(){
    static float statsScrollOffset = 0.0f;
    
    DrawRectangle(Config::esc_x, Config::esc_y, Config::esc_width, Config::esc_height, Color{14, 20, 11, 255});
    DrawRectangleLines(Config::esc_x, Config::esc_y, Config::esc_width, Config::esc_height, Config::COLOR_SHARD_BORDER);

    std::string title = "STATS";
    int titleW = MeasureText(title.c_str(), 26);
    DrawText(title.c_str(), Config::esc_x + (Config::esc_width - titleW) / 2, Config::esc_y + 20, 26, Config::COLOR_UI_GREEN);
    DrawLineEx({ (float)Config::esc_x + 20, (float)Config::esc_y + 60 }, { (float)Config::esc_x + Config::esc_width - 20, (float)Config::esc_y + 60 }, 1.0f, Config::COLOR_SHARD_BORDER);

    float listX = Config::esc_x + 30.0f;
    float listY = Config::esc_y + 80.0f;
    float listW = Config::esc_width - 60.0f;
    float listH = Config::esc_height - 140.0f;
    float rowH = 30.0f;

    auto& registry = GetStatsStats();
    float contentH = registry.size() * rowH;
    float maxScroll = fmaxf(0.0f, contentH - listH);

    Rectangle listRect = { listX, listY, listW, listH };
    if (CheckCollisionPointRec(GetMousePosition(), listRect)) {
        statsScrollOffset -= GetMouseWheelMove() * 20.0f;
    }
    statsScrollOffset = Clamp(statsScrollOffset, 0.0f, maxScroll);

    BeginScissorMode((int)listX, (int)listY, (int)listW, (int)listH);
    float rowY = listY - statsScrollOffset;
    for (const auto& stat : registry) {
        if (rowY + rowH >= listY && rowY <= listY + listH) {
            DrawText(stat.label.c_str(), listX, rowY + 6, 14, Color{ 150, 180, 200, 255});
            std::string value = stat.getValue();
            int valW = MeasureText(value.c_str(), 14);
            DrawText(value.c_str(), listX + listW - valW, rowY + 6, 14, WHITE);
            DrawLineEx({ listX, rowY + rowH - 2 }, { listX + listW, rowY + rowH - 2 }, 1.0f, Fade(Config::COLOR_SHARD_BORDER, 0.5f));
        }
        rowY += rowH;
    }
    EndScissorMode();

    Rectangle backBtn = { listX, Config::esc_y + Config::esc_height - 54.0f, listW, 46.0f };
    if (DrawButton(backBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_UI_AMBER, Config::COLOR_UI_AMBER, WHITE, "Back", 18)) {
        esc_menu_state = MAIN;
    }
}


std::vector<SettingEntry>& GetSettingsSettings() {
    static std::vector<SettingEntry> settingslist = {
        { "Game Speed", SettingType::NOTCHED_SLIDER, nullptr, &Config::GAME_SPEED, 0.0f, 0.0f},
        {"Master Volume", SettingType::SLIDER, nullptr, &settings.masterVolume, 0.0f, 2.0f},
        {"Screen Shake", SettingType::TOGGLE, &settings.screenShakeEnabled},
    };
    return settingslist;
}

void drawsettingsmenu(){
    DrawRectangle(Config::esc_x, Config::esc_y, Config::esc_width, Config::esc_height, Color{14, 20, 11, 255});
    DrawRectangleLines(Config::esc_x, Config::esc_y, Config::esc_width, Config::esc_height, Config::COLOR_SHARD_BORDER);

    std::string title = "SETTINGS";
    int titleW = MeasureText(title.c_str(), 26);
    DrawText(title.c_str(), Config::esc_x + (Config::esc_width - titleW) / 2, Config::esc_y + 20, 26, Config::COLOR_UI_GREEN);
    DrawLineEx({ (float)Config::esc_x + 20, (float)Config::esc_y + 60 }, { (float)Config::esc_x + Config::esc_width - 20, (float)Config::esc_y + 60 }, 1.0f, Config::COLOR_SHARD_BORDER);

    float rowX = Config::esc_x + 30.0f;
    float rowY = Config::esc_y + 85.0f;
    float rowW = Config::esc_width - 60.0f;
    float rowH = 46.0f;
    float rowGap = 12.0f;

    Vector2 mousePos = GetMousePosition();
    auto& registry = GetSettingsSettings();

    for (const auto& setting : registry) {
        DrawText(setting.label.c_str(), rowX, rowY, 15, Color{ 150, 180, 200, 255 });

        if (setting.type == SettingType::TOGGLE && setting.boolValue) {
            Rectangle toggleBtn = { rowX + rowW - 70.0f, rowY - 6.0f, 70.0f, 26.0f };
            std::string toggleLabel = *setting.boolValue ? "ON" : "OFF";
            Color toggleColor = *setting.boolValue ? Config::COLOR_UI_GREEN : Color{ 90, 90, 90, 255 };
            if (DrawButton(toggleBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, toggleColor, toggleColor, toggleLabel.c_str(), 13)) {
                *setting.boolValue = !(*setting.boolValue);
            }
        }
        else if (setting.type == SettingType::SLIDER && setting.floatValue) {
            Rectangle sliderBar = { rowX, rowY + 20.0f, rowW, 8.0f };
            DrawRectangleRec(sliderBar, Color{ 30, 40, 50, 255 });

            float t = Clamp((*setting.floatValue - setting.minValue) / (setting.maxValue - setting.minValue), 0.0f, 1.0f);
            DrawRectangle(sliderBar.x, sliderBar.y, sliderBar.width * t, sliderBar.height, Config::COLOR_UI_GREEN);

            Rectangle handleHit = { sliderBar.x - 6, sliderBar.y - 8, sliderBar.width + 12, sliderBar.height + 16 };
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, handleHit)) {
                float newT = Clamp((mousePos.x - sliderBar.x) / sliderBar.width, 0.0f, 1.0f);
                *setting.floatValue = setting.minValue + newT * (setting.maxValue - setting.minValue);
            }

            std::string valText = std::to_string((int)(t * 100)) + "%";
            int valW = MeasureText(valText.c_str(), 12);
            DrawText(valText.c_str(), rowX + rowW - valW, rowY, 12, WHITE);
        }
        else if (setting.type == SettingType::NOTCHED_SLIDER && setting.floatValue) {
            Rectangle sliderBar = { rowX, rowY + 20.0f, rowW, 8.0f };
            DrawNotchedSlider(sliderBar, setting.floatValue, GetGameSpeedNotches(), mousePos);
            rowY += 14.0f;
        }

        rowY += rowH + rowGap;
    }

    Rectangle backBtn = { rowX, Config::esc_y + Config::esc_height - 54.0f, rowW, 46.0f };
    if (DrawButton(backBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_UI_AMBER, Config::COLOR_UI_AMBER, WHITE, "Back", 18)) {
        if (gamestate.gamestate ==MainMenu){
            TriggerGlitchAt({ Config::esc_x, Config::esc_y, Config::esc_width, Config::esc_height }, 0.16f);
            TimerStartOnce(&exitanimtimer, 0.1f);
            playsoundsmart(transitionsound, 0.2f, 1.0f);
            return;
        }
        esc_menu_state = MAIN;
    }
}


void drawescmenu(){
    DrawRectangle(0, 0, Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, Color{0, 0, 0, 200});
    DrawRectangle(Config::esc_x, Config::esc_y, Config::esc_width, Config::esc_height, Color{14, 20, 11, 255});
    DrawRectangleLines(Config::esc_x, Config::esc_y, Config::esc_width, Config::esc_height, Config::COLOR_SHARD_BORDER);
    
    //make sure the menu doesnt close instantly when you click on it make sure its a new click yk yk yk ok good cool
    static bool releasedbuttoncheck = false;
    
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
        releasedbuttoncheck = true;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&releasedbuttoncheck&&!CheckCollisionPointRec(GetMousePosition(), { Config::esc_x, Config::esc_y, Config::esc_width, Config::esc_height })) {
        if (!exitanimtimer.started) {
            TriggerGlitchAt({ Config::esc_x, Config::esc_y, Config::esc_width, Config::esc_height }, 0.16f);
            TimerStartOnce(&exitanimtimer, 0.1f);
            playsoundsmart(transitionsound, 0.2f, 1.0f);
        }
    }

    std::string title = "PAUSED";
    int titleW = MeasureText(title.c_str(), 26);
    DrawText(title.c_str(), Config::esc_x + (Config::esc_width - titleW) / 2, Config::esc_y + 20, 26, Config::COLOR_UI_GREEN);
    DrawLineEx({ (float)Config::esc_x + 20, (float)Config::esc_y + 60 }, { (float)Config::esc_x + Config::esc_width - 20, (float)Config::esc_y + 60 }, 1.0f, Config::COLOR_SHARD_BORDER);

    //put in variable config later 
    float buttonW = Config::esc_width - 60.0f;
    float buttonH = 46.0f;
    float buttonGap = 14.0f;
    float startY = Config::esc_y + 85.0f;
    float buttonX = Config::esc_x + 30.0f;

    Rectangle newRunBtn = {buttonX, startY, buttonW, buttonH};
    Rectangle mainMenuBtn = {buttonX, startY + (buttonH + buttonGap) * 1, buttonW, buttonH};
    Rectangle settingsBtn = {buttonX, startY + (buttonH + buttonGap) * 2, buttonW, buttonH};
    Rectangle statsBtn = {buttonX, startY + (buttonH + buttonGap) * 3, buttonW, buttonH};
    Rectangle backBtn = {buttonX, Config::esc_y + Config::esc_height - 54.0f, buttonW, buttonH};


    //new game so i can ctrl f this
    if (esc_menu_state==MAIN){
        if (DrawButton(newRunBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, Config::COLOR_UI_GREEN, WHITE, "New Run", 18)) {
            engine.daemons.clear();
            gamestate.balance = 0;
            GenerateTopologyMap();
            RequestGameStateChange(GAME);
            //fix this plz gotta figure out how to restart maybe use init game again idk
        }

        if (DrawButton(mainMenuBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, Config::COLOR_UI_GREEN, WHITE, "Main Menu", 18)) {
            RequestGameStateChange(MainMenu);
            TimerStartOnce(&exitanimtimer, 0.1f);
        }
        if (DrawButton(settingsBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, Config::COLOR_UI_GREEN, WHITE, "Settings", 18)) {
            esc_menu_state = SETTINGZ;
        }
        if (DrawButton(statsBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, Config::COLOR_UI_GREEN, WHITE, "Stats", 18)) {
            esc_menu_state = STATS;
        }
    }
    if (esc_menu_state == SETTINGZ) {
        drawsettingsmenu();
    } else if (esc_menu_state == STATS) {
        drawstatsmenu();
        return;
    }

    //exit and entry glitches
    static bool wasMenuOpen = false;

    if (esc_menu && !wasMenuOpen) {
        TriggerGlitchAt({ Config::esc_x, Config::esc_y, Config::esc_width, Config::esc_height }, 0.16f);
        playsoundsmart(transitionsound, 0.2f, 1.0f);
    }
    wasMenuOpen = esc_menu;
    
    if (DrawButton(backBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_UI_AMBER, Config::COLOR_UI_AMBER, WHITE, "Back", 18)) {
        if (!exitanimtimer.started) {
            TriggerGlitchAt({ Config::esc_x, Config::esc_y, Config::esc_width, Config::esc_height }, 0.16f);
            TimerStartOnce(&exitanimtimer, 0.1f);
            playsoundsmart(transitionsound, 0.2f, 1.0f);
        }
    }

    if (exitanimtimer.active) {
        TimerUpdate(&exitanimtimer);
        if (TimerJustFinished(&exitanimtimer)) {
            esc_menu = false;
            releasedbuttoncheck = false;
            exitanimtimer = {0};
            wasMenuOpen = false;
            esc_menu_state = MAIN;
        }
    }
}