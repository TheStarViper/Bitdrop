#include "tutorial.hpp"
#include "raylib.h"
#include "variables.hpp"
#include "button.hpp"
#include <vector>
#include <deque>
#include <sstream>
#include "transition.hpp"
#include "audio.hpp"

namespace {
    std::vector<TutHint> hints = {
        { "welcome", "SPACEBAR launches a ball. Balls have a base score of 1MB and hitting a pin adds 1MB, score data by landing in baskets and baskets multiply score by label" },
        { "welcome-quota", "Each level has a score quota, for this level it's 200MB so score the quota to cash out" },
        { "welcome-shop", "Congrats on beating your first level and welcome to the shop! Here is where you buy all sorts of goodies to try and progress furthur in the game!" },
        { "welcome-shop2", "In this top section are DAEMONS. Daemons are like jokers in balatro, there are a ton of DAEMONS to choose from in BITDROP which all help improve scoring or something else. You can read DAEMON descriptions for more info" },
        { "welcome-shop3", "In this bottom section are CONSUMABLES. These consumables can be used for all sorts of things like modifying pins or one time use consumables that give you a one time bonus!" },
        { "welcome-map", "Welcome to BITDROP! The first thing you want to do is you wanna click one of the nodes in the green column to enter a level." },
        { "welcome-map2", "Each node has a quota which is the score you have to score to hack the node successfully and a reward which is how much you get paid out. So keep these in mind when making a route!" },
        { "encrypted-node", "This node is encrypted so you don't know what is has in store for you! Choose at your own risk!" },
        { "multipleballs", "" },
        { "", "" },
        { "", "" },
        { "", "" },
        { "", "" },
        { "", "" },
        { "", "" },
        { "", "" },
        { "", "" },
        { "", "" },
    };
    std::deque<std::string> hintQueue;
}

bool seenhint(const std::string& id){
    for (auto&h: hints){
        if (h.id ==id){
            return h.shown;
        }
    }
    return true;
}

void triggerhint(const std::string& id, int x, int y, int width){
    for (auto&h: hints){
        if (h.id ==id){
            if (h.shown) return;
            h.shown = true;
            h.x = x;
            h.y = y;
            h.width = width;
            hintQueue.push_back(id);
            return;
        }
    }
}

void drawhint(){
    if (hintQueue.empty()) return;
    std::string currentId = hintQueue.front();
    const TutHint* hint = nullptr;
    for (auto& h : hints) {
        if (h.id == currentId) { hint = &h; break; }
    }
    if (!hint) {
        hintQueue.pop_front();
        return;
    }

    float boxW = 420.0f;
    boxW = hint->width;
    float paddingX = 16.0f;
    float paddingY = 14.0f;
    float fontSize = 13.0f;
    Font font = GetFontDefault();

    float maxTextWidth = boxW - (paddingX * 2.0f);
    std::vector<std::string> lines;
    std::string currentLine;
    std::string word;
    std::stringstream ss(hint->content);
    while (ss >> word) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        Vector2 size = MeasureTextEx(font, testLine.c_str(), fontSize, 1.0f);
        if (size.x > maxTextWidth) {
            if (!currentLine.empty()) lines.push_back(currentLine);
            currentLine = word;
        } else {
            currentLine = testLine;
        }
    }
    if (!currentLine.empty()) lines.push_back(currentLine);


    DrawRectangle(0, 0, Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, Color{0, 0, 0, 150});
    float lineHeight = fontSize * 1.5f;
    float textBlockH = lines.size() * lineHeight;
    float boxH = textBlockH + paddingY * 2.0f + 40.0f;

    float boxX = Config::SCREEN_WIDTH / 2.0f - boxW / 2.0f;
    float boxY = Config::SCREEN_HEIGHT - boxH - 30.0f;
    
    boxX = hint->x;
    boxY = hint->y;
    DrawRectangle(boxX, boxY, boxW, boxH, Color{ 8, 16, 24, 245 });
    DrawRectangleLinesEx({ boxX, boxY, boxW, boxH }, 1.5f, Config::COLOR_UI_GREEN);
    DrawRectangle(boxX, boxY, 6, boxH, Config::COLOR_UI_GREEN);

    DrawText("HINT:", boxX + paddingX, boxY + 10, 16, Config::COLOR_UI_AMBER);

    float lineY = boxY + 28.0f;
    for (const auto& line : lines) {
        DrawTextEx(font, line.c_str(), { boxX + paddingX, lineY+5 }, fontSize, 1.0f, WHITE);
        lineY += lineHeight;
    }

    PauseMusicStream(bgmusic);
    Rectangle dismissBtn = { boxX + boxW - 110.0f, boxY + boxH - 34.0f, 94.0f, 24.0f };
    if (DrawButton(dismissBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, Config::COLOR_UI_GREEN, WHITE, "GOT IT", 12)) {
        hintQueue.pop_front();
        TriggerGlitchAt({ boxX, boxY, boxW, boxH }, 0.12f);
        playsoundsmart(transitionsound, 0.2f, 1.5f);
        ResumeMusicStream(bgmusic);
    }
}
