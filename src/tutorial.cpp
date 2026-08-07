#include "tutorial.hpp"
#include "raylib.h"
#include "variables.hpp"
#include "button.hpp"
#include <vector>
#include <deque>
#include <sstream>

namespace {
    std::vector<TutHint> hints = {
        { "welcome_game", "SPACEBAR launches a data probe. Land it in a basket to cash out at that basket's multiplier before your reserve runs dry." },
        { "node_modifiers", "Pins can carry modifiers like Boost or Glitch. Buy a targeting consumable from the shop and click a pin to apply one." },
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

void triggerhint(const std::string& id){
    for (auto&h: hints){
        if (h.id ==id){
            if (h.shown) return;
            h.shown = true;
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

    float lineHeight = fontSize * 1.5f;
    float textBlockH = lines.size() * lineHeight;
    float boxH = textBlockH + paddingY * 2.0f + 40.0f;

    float boxX = Config::SCREEN_WIDTH / 2.0f - boxW / 2.0f;
    float boxY = Config::SCREEN_HEIGHT - boxH - 30.0f;

    DrawRectangle(boxX, boxY, boxW, boxH, Color{ 8, 16, 24, 245 });
    DrawRectangleLinesEx({ boxX, boxY, boxW, boxH }, 1.5f, Config::COLOR_UI_GREEN);
    DrawRectangle(boxX, boxY, 6, boxH, Config::COLOR_UI_GREEN);

    DrawText("TUTORIAL", boxX + paddingX, boxY + 10, 11, Config::COLOR_UI_AMBER);

    float lineY = boxY + 28.0f;
    for (const auto& line : lines) {
        DrawTextEx(font, line.c_str(), { boxX + paddingX, lineY }, fontSize, 1.0f, WHITE);
        lineY += lineHeight;
    }

    Rectangle dismissBtn = { boxX + boxW - 110.0f, boxY + boxH - 34.0f, 94.0f, 24.0f };
    if (DrawButton(dismissBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, Config::COLOR_UI_GREEN, WHITE, "GOT IT", 12)) {
        hintQueue.pop_front();
    }
}
