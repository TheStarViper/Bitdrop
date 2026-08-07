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
        { "welcome-quota", "Each level has a score quota for this level it's 200MB so score the quota to cash out" },
        { "basket_score", "Nice hit! Probes convert to credits the instant they land in a basket, based on that basket's multiplier." },
        { "shop_intro", "Spend your credits here. Daemons give passive or triggered bonuses; consumables are one-time board effects." },
        { "consumable_target", "This consumable needs a target. Click a pin on the board, then hit CONFIRM to lock it in." },
        { "map_intro", "Choose your next node. Higher-value nodes usually carry more risk or a tougher quota." },
        { "encrypted_node", "This node is encrypted. You'll need a Decrypt consumable to see what's actually there before committing." }
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

    Rectangle dismissBtn = { boxX + boxW - 110.0f, boxY + boxH - 34.0f, 94.0f, 24.0f };
    if (DrawButton(dismissBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, Config::COLOR_UI_GREEN, WHITE, "GOT IT", 12)) {
        hintQueue.pop_front();
        TriggerGlitchAt({ boxX, boxY, boxW, boxH }, 0.12f);
        playsoundsmart(transitionsound, 0.2f, 1.5f);
    }
}
