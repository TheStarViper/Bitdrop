#include "esc-menu.hpp"
#include "raylib.h"
#include "variables.hpp"
#include "button.hpp"
#include "transition.hpp"

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
        esc_menu = false;
        releasedbuttoncheck = false;
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
    Rectangle backBtn = {buttonX, startY + (buttonH + buttonGap) * 4, buttonW, buttonH};

    if (DrawButton(newRunBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, Config::COLOR_UI_GREEN, WHITE, "New Run", 18)) {

    }

    if (DrawButton(mainMenuBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, Config::COLOR_UI_GREEN, WHITE, "Main Menu", 18)) {

    }

    if (DrawButton(settingsBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, Config::COLOR_UI_GREEN, WHITE, "Settings", 18)) {

    }

    if (DrawButton(statsBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, Config::COLOR_UI_GREEN, WHITE, "Stats", 18)) {

    }

    if (DrawButton(backBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_UI_AMBER, Config::COLOR_UI_AMBER, WHITE, "Back", 18)) {
        TriggerGlitchAt({ Config::esc_x, Config::esc_y, Config::esc_width, Config::esc_height }, 0.5f);
        esc_menu = false;
        releasedbuttoncheck = false;
    }
}