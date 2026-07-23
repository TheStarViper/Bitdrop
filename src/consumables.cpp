#include "consumables.hpp"
#include "variables.hpp"
#include "button.hpp"
#include "audio.hpp"
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cctype>

namespace {
    constexpr int MAX_CONSUMABLE_SLOTS = 2;
    constexpr float SLOT_GAP = 8.0f;
    constexpr float SLOT_TOP_MARGIN = 12.0f;
    constexpr float PANEL_CUT = 14.0f;
    int pendingIndex = -1;

    void GetPanelPoints(Rectangle r, float cut, Vector2 out[6]) {
        out[0] = { r.x, r.y };
        out[1] = { r.x + r.width - cut, r.y };
        out[2] = { r.x + r.width, r.y + cut };
        out[3] = { r.x + r.width, r.y + r.height };
        out[4] = { r.x + cut, r.y + r.height };
        out[5] = { r.x, r.y + r.height - cut };
    }

    void DrawTechPanelFill(Rectangle r, float cut, Color fill) {
        Vector2 pts[6];
        GetPanelPoints(r, cut, pts);
        DrawTriangleFan(pts, 6, fill);
    }

    void DrawTechPanelOutline(Rectangle r, float cut, Color color, float thickness) {
        Vector2 pts[6];
        GetPanelPoints(r, cut, pts);
        for (int i = 0; i < 6; i++) {
            DrawLineEx(pts[i], pts[(i + 1) % 6], thickness, color);
        }
    }
}

ConsumableInventory activeconsumableinfo;

void Consumable::updatePosition() {
    float totalGapWidth = SLOT_GAP * (MAX_CONSUMABLE_SLOTS - 1);
    width = (420.0f - totalGapWidth) / MAX_CONSUMABLE_SLOTS;
    x = Config::walletX + (slot - 1) * (width + SLOT_GAP);
    y = Config::walletY + 65.0f + SLOT_TOP_MARGIN;
}

bool IsConsumablePending() {
    return pendingIndex != -1;
}

int GetPendingConsumableIndex() {
    return pendingIndex;
}

void CancelPendingConsumable() {
    pendingIndex = -1;
}

void ResolvePendingConsumable() {
    if (pendingIndex < 0 || pendingIndex >= (int)activeconsumableinfo.consumables.size()) {
        pendingIndex = -1;
        return;
    }
    auto& c = activeconsumableinfo.consumables[pendingIndex];
    if (c.useFn) c.useFn(c);
    activeconsumableinfo.consumables.erase(activeconsumableinfo.consumables.begin() + pendingIndex);
    pendingIndex = -1;
    for (size_t i = 0; i < activeconsumableinfo.consumables.size(); i++) {
        activeconsumableinfo.consumables[i].slot = (int)i + 1;
        activeconsumableinfo.consumables[i].updatePosition();
    }
}

void DrawConsumableEmptySlot(int slotIndex) {
    Consumable dummy("", "", BLANK, 0, ConsumableEffectType::INSTANT, nullptr);
    dummy.slot = slotIndex;
    dummy.updatePosition();

    Rectangle r = { dummy.x, dummy.y, dummy.width, dummy.height };
    DrawTechPanelFill(r, PANEL_CUT, Color{ 10, 16, 26, 90 });
    DrawTechPanelOutline(r, PANEL_CUT, Color{ 40, 60, 80, 150 }, 1.0f);

    std::string emptyText = "EMPTY";
    int tw = MeasureText(emptyText.c_str(), 10);
    DrawText(emptyText.c_str(), dummy.x + (dummy.width - tw) / 2.0f, dummy.y + dummy.height / 2.0f - 5, 10, Color{ 60, 90, 110, 180 });
}

int DrawConsumableSlot(Consumable& c, Vector2 mousePos, int idx, bool isSelected) {
    c.isHovered = CheckCollisionPointRec(mousePos, { c.x, c.y, c.width, c.height });

    if (c.isHovered && !c.wasHovered) {
        playsoundsmart(hoversound, .5f, 1.6f);
    }
    c.wasHovered = c.isHovered;

    bool isPendingThis = (pendingIndex == idx);
    bool otherPending = (pendingIndex != -1 && !isPendingThis);
    bool highlighted = isPendingThis || isSelected;

    Rectangle cardRect = { c.x, c.y, c.width, c.height };
    Color bg = c.isHovered ? Color{ 18, 28, 44, 255 } : Color{ 10, 16, 26, 240 };
    DrawTechPanelFill(cardRect, PANEL_CUT, bg);

    for (float ly = c.y + 6.0f; ly < c.y + c.height - 6.0f; ly += 5.0f) {
        DrawLineEx({ c.x + 6, ly }, { c.x + c.width - 6, ly }, 1.0f, Fade(BLACK, 0.05f));
    }

    Color accentBorder = highlighted ? Config::COLOR_UI_AMBER : (c.isHovered ? Config::COLOR_UI_GREEN : Config::COLOR_SHARD_BORDER);

    if (highlighted) {
        Rectangle glowRect = { c.x - 3, c.y - 3, c.width + 6, c.height + 6 };
        DrawTechPanelOutline(glowRect, PANEL_CUT + 3.0f, Fade(accentBorder, 0.3f), 4.0f);
    }
    DrawTechPanelOutline(cardRect, PANEL_CUT, accentBorder, highlighted ? 1.6f : 1.0f);

    float cornerLen = 12.0f;
    DrawLineEx({ c.x, c.y }, { c.x + cornerLen, c.y }, 2.0f, c.color);
    DrawLineEx({ c.x, c.y }, { c.x, c.y + cornerLen }, 2.0f, c.color);
    DrawLineEx({ c.x + c.width, c.y + c.height }, { c.x + c.width - cornerLen, c.y + c.height }, 2.0f, c.color);
    DrawLineEx({ c.x + c.width, c.y + c.height }, { c.x + c.width, c.y + c.height - cornerLen }, 2.0f, c.color);

    std::string displayName = c.name;
    for (auto& ch : displayName) ch = (char)toupper((unsigned char)ch);
    DrawText(displayName.c_str(), c.x + 14, c.y + 9, 12, c.color);

    std::string subtitle = (c.effectType == ConsumableEffectType::BOARD_TARGET) ? "TARGET REQUIRED" : "INSTANT USE";
    DrawText(subtitle.c_str(), c.x + 14, c.y + 25, 9, Color{ 130, 160, 180, 255 });

    DrawLineEx({ c.x + 12, c.y + 38 }, { c.x + c.width - 12, c.y + 38 }, 1.0f, Fade(c.color, 0.35f));

    float headerBottom = c.y + 44.0f;
    float bodyBottom = c.y + c.height - 34.0f;
    if (bodyBottom > headerBottom) {
        float iconCenterX = c.x + c.width / 2.0f;
        float iconCenterY = headerBottom + (bodyBottom - headerBottom) / 2.0f;
        float iconRadius = std::min(c.width * 0.32f, (bodyBottom - headerBottom) * 0.4f);

        DrawPoly({ iconCenterX, iconCenterY }, 6, iconRadius, 0.0f, Fade(c.color, 0.12f));
        DrawPolyLines({ iconCenterX, iconCenterY }, 6, iconRadius, 0.0f, Fade(c.color, 0.45f));

        if (!c.name.empty()) {
            std::string monogram(1, (char)toupper((unsigned char)c.name[0]));
            int monoSize = (int)(iconRadius * 1.1f);
            int monoW = MeasureText(monogram.c_str(), monoSize);
            DrawText(monogram.c_str(), iconCenterX - monoW / 2.0f, iconCenterY - monoSize / 2.0f, monoSize, Fade(c.color, 0.8f));
        }
    }

    int action = 0;
    float easeProgress = 1.0f - powf(1.0f - c.expansion, 3.0f);

    if (isPendingThis) {
        Rectangle cancelBtn = { c.x + 8, c.y + c.height - 24, c.width - 16, 16 };
        bool cancelClicked = DrawButton(cancelBtn, ButtonType::TextGeneric, 255, Color{ 45, 15, 20, 255 }, Color{ 180, 40, 40, 255 }, Config::COLOR_UI_AMBER, WHITE, "CANCEL", 10);
        if (cancelClicked) action = -1;
    } else if (c.expansion > 0.01f && !otherPending) {
        unsigned char alpha = (unsigned char)(easeProgress * 255);
        float slideOffset = (1.0f - easeProgress) * 12.0f;
        float bY = (c.y + c.height - 28.0f) + slideOffset;

        DrawLineEx({ c.x + 8, bY - 6 }, { c.x + c.width - 8, bY - 6 }, 1.0f, Fade(WHITE, 0.08f * easeProgress));

        Color amberCol = { Config::COLOR_UI_AMBER.r, Config::COLOR_UI_AMBER.g, Config::COLOR_UI_AMBER.b, alpha };
        Color textCol = { 255, 255, 255, alpha };

        Rectangle useBtn = { c.x + 8, bY, (c.width - 24) / 2.0f, 20 };
        Rectangle sellBtn = { useBtn.x + useBtn.width + 8, bY, (c.width - 24) / 2.0f, 20 };

        Color useNormalBg = Color{ 15, 35, 20, 255 };
        Color useHoverBg = Color{ 40, 140, 60, 255 };
        bool useClicked = DrawButton(useBtn, ButtonType::TextGeneric, alpha, useNormalBg, useHoverBg, Config::COLOR_UI_GREEN, textCol, "USE", 10);

        std::string sellText = "$" + std::to_string(c.sellValue);
        Color sellNormalBg = Color{ 45, 15, 20, 255 };
        Color sellHoverBg = Color{ 180, 40, 40, 255 };
        bool sellClicked = DrawButton(sellBtn, ButtonType::TextGeneric, alpha, sellNormalBg, sellHoverBg, amberCol, textCol, sellText.c_str(), 10);

        if (useClicked) action = 1;
        else if (sellClicked) action = 2;
    }

    if (c.isHovered && !c.description.empty()) {
        float boxW = 240.0f;
        float paddingX = 10.0f;
        float paddingY = 10.0f;
        float fontSize = 11.0f;
        float screenMargin = 8.0f;

        Font font = GetFontDefault();
        float maxTextWidth = boxW - (paddingX * 2.0f);
        std::vector<std::string> lines;
        std::string currentLine;
        std::string word;
        std::stringstream ss(c.description);

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
        float boxH = (lines.size() * lineHeight) + (paddingY * 2.0f) - (lineHeight - fontSize);

        float boxX = c.x;
        if (boxX + boxW > GetScreenWidth() - screenMargin) boxX = GetScreenWidth() - screenMargin - boxW;
        if (boxX < screenMargin) boxX = screenMargin;

        float boxY = c.y - boxH - 6.0f;
        if (boxY < screenMargin) boxY = c.y + c.height + 6.0f;
        if (boxY + boxH > GetScreenHeight() - screenMargin) boxY = GetScreenHeight() - screenMargin - boxH;

        DrawRectangle(boxX, boxY, boxW, boxH, Color{ 6, 12, 22, 250 });
        DrawRectangleLinesEx({ boxX, boxY, boxW, boxH }, 1.0f, Config::COLOR_UI_GREEN);

        float lineY = boxY + paddingY;
        for (const auto& line : lines) {
            DrawTextEx(font, line.c_str(), { boxX + paddingX, lineY }, fontSize, 1.0f, Config::COLOR_PROBE);
            lineY += lineHeight;
        }
    }

    return action;
}

void PrepDrawConsumableSlots() {
    static bool isInitialized = false;
    if (!isInitialized) {
        activeconsumableinfo.consumables.push_back(Consumable("Reroll", "Reroll all shop offers once", Config::COLOR_UI_AMBER, 120, ConsumableEffectType::INSTANT, UseRerollCharge));
        activeconsumableinfo.consumables.push_back(Consumable("Overclock", "Temporarily overclock a random daemon", Config::MAGENTA_DAEMON, 180, ConsumableEffectType::INSTANT, UseOverclockBooster));
        activeconsumableinfo.consumables.push_back(Consumable("Board Wipe", "Clear all active glitch modifiers from the board", Config::COLOR_UI_GREEN, 200, ConsumableEffectType::BOARD_TARGET, UseBoardWipeCharge));

        for (size_t i = 0; i < activeconsumableinfo.consumables.size(); i++) {
            activeconsumableinfo.consumables[i].slot = (int)i + 1;
            activeconsumableinfo.consumables[i].updatePosition();
        }
        isInitialized = true;
    }

    if (pendingIndex != -1 && IsKeyPressed(KEY_ESCAPE)) {
        pendingIndex = -1;
    }

    static int selectedIndex = -1;
    Vector2 mPos = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && pendingIndex == -1) {
        bool clickedCard = false;
        for (size_t i = 0; i < activeconsumableinfo.consumables.size(); i++) {
            auto& c = activeconsumableinfo.consumables[i];
            if (CheckCollisionPointRec(mPos, { c.x, c.y, c.width, c.height })) {
                selectedIndex = (int)i;
                clickedCard = true;
                break;
            }
        }
        if (!clickedCard) selectedIndex = -1;
    }

    float dt = GetFrameTime() * Config::GAME_SPEED;
    float expansionSpeed = 6.0f;

    int actedIndex = -1;
    int actedAction = 0;

    for (size_t i = 0; i < activeconsumableinfo.consumables.size(); i++) {
        auto& c = activeconsumableinfo.consumables[i];
        bool isSelected = (selectedIndex == (int)i);
        c.expansion = isSelected ? std::min(1.0f, c.expansion + dt * expansionSpeed)
                                  : std::max(0.0f, c.expansion - dt * expansionSpeed);

        int action = DrawConsumableSlot(c, mPos, (int)i, isSelected);
        if (action != 0 && actedIndex == -1) {
            actedIndex = (int)i;
            actedAction = action;
        }
    }

    for (int slotNum = (int)activeconsumableinfo.consumables.size() + 1; slotNum <= MAX_CONSUMABLE_SLOTS; slotNum++) {
        DrawConsumableEmptySlot(slotNum);
    }

    if (actedIndex != -1) {
        if (actedAction == -1) {
            pendingIndex = -1;
        } else {
            auto& c = activeconsumableinfo.consumables[actedIndex];
            if (actedAction == 2) {
                gamestate.balance += c.sellValue;
                activeconsumableinfo.consumables.erase(activeconsumableinfo.consumables.begin() + actedIndex);
                selectedIndex = -1;
            } else if (actedAction == 1) {
                if (c.effectType == ConsumableEffectType::BOARD_TARGET) {
                    pendingIndex = actedIndex;
                } else {
                    if (c.useFn) c.useFn(c);
                    activeconsumableinfo.consumables.erase(activeconsumableinfo.consumables.begin() + actedIndex);
                    selectedIndex = -1;
                }
            }
        }
    }

    for (size_t i = 0; i < activeconsumableinfo.consumables.size(); i++) {
        activeconsumableinfo.consumables[i].slot = (int)i + 1;
        activeconsumableinfo.consumables[i].updatePosition();
    }
}

void UseRerollCharge(Consumable&) {
}

void UseOverclockBooster(Consumable&) {
}

void UseBoardWipeCharge(Consumable&) {
}