#include "shop.hpp"
#include "raylib.h"
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <sstream>
#include <cctype>
#include "variables.hpp"
#include "button.hpp"
#include "main.hpp"
#include "audio.hpp"
#include "transition.hpp"
#include "consumables.hpp"

struct RerollGlitchState {
    float timer = 0.0f;
    float duration = 0.25f;
    bool active = false;
};
RerollGlitchState rerollGlitch;
struct ShopConsumableEntry {
    std::string name;
    std::string description;
    Color color;
    ConsumableEffectType effectType;
    void (*useFn)(Consumable&);
    int sellValue;
    int price;
};

static std::vector<ShopConsumableEntry> consumableShopPool = {
    { "Reroll", "Reroll all shop offers once", Config::COLOR_UI_AMBER, ConsumableEffectType::INSTANT, UseRerollCharge, 120, 250 },
    { "Overclock", "Temporarily overclock a random daemon", Config::MAGENTA_DAEMON, ConsumableEffectType::INSTANT, UseOverclockBooster, 180, 350 },
    { "Board Wipe", "Clear all active glitch modifiers from the board", Config::COLOR_UI_GREEN, ConsumableEffectType::BOARD_TARGET, UseBoardWipeCharge, 200, 400 },
    { "Fire Sale", "every daemon in your hand adds its full sell value to your balance", Config::COLOR_UI_AMBER, ConsumableEffectType::INSTANT, firesale, 60, 300 },
    { "Decrypt", "Select an encrypted node on the map to reveal it", Config::MAGENTA_DAEMON, ConsumableEffectType::BOARD_TARGET, UseDecryptNode, 150, 450 }
};

static int consumableShopSlots[4] = { -1, -1, -1, -1 };
static bool consumableSold[4] = { false, false, false, false };
static smartbool consumableHoverStates[4];

float GetRerollGlitchIntensity(void) {
    if (!rerollGlitch.active) return 0.0f;
    float t = rerollGlitch.timer / rerollGlitch.duration;
    if (t >= 1.0f) return 0.0f;
    return sinf(t * PI);
}

void UpdateRerollGlitch(void) {
    if (!rerollGlitch.active) return;
    rerollGlitch.timer += GetFrameTime();
    if (rerollGlitch.timer >= rerollGlitch.duration) {
        
        rerollGlitch.active = false;
        rerollGlitch.timer = 0.0f;
    }
}

void DrawShopItem(Vector2 pos, const Daemon& iteminfo, bool& isSlotSold, smartbool& hoverState) {
    const float gap = 8.0f;
    const float mainBoxWidth = Config::shopitemtotalWidth - Config::shopbuyitembuttonWidth - gap;
    Color mainColor   = iteminfo.GetColor();
    Color dimColor    = (Color){ static_cast<unsigned char>(mainColor.r * 0.4f), static_cast<unsigned char>(mainColor.g * 0.4f), static_cast<unsigned char>(mainColor.b * 0.4f), 255 };
    Color textDim     = (Color){ static_cast<unsigned char>(mainColor.r * 0.7f), static_cast<unsigned char>(mainColor.g * 0.7f), static_cast<unsigned char>(mainColor.b * 0.7f), 255 };

    Rectangle btnRect = { pos.x+Config::shopitemtotalWidth-Config::shopbuyitembuttonWidth, pos.y, Config::shopbuyitembuttonWidth, Config::shopitemtotalHeight };
    Rectangle mainRect = { pos.x, pos.y, mainBoxWidth, Config::shopitemtotalHeight };

    Vector2 mousePos = GetMousePosition();
    bool rawHover = CheckCollisionPointRec(mousePos, mainRect) && !isSlotSold;
    hoverState = rawHover;
    if (hoverState.is_new_true()) {
        playsoundsmart(hoversound,.1,1.6);
    }

    bool isHovered = hoverState;
    Rectangle innerBtn = { btnRect.x + 8, btnRect.y + 14, btnRect.width - 16, btnRect.height - 36 };
    bool hasFunds = (gamestate.balance >= iteminfo.price);
    bool buyClicked = false;
    unsigned char alpha = 255;

    if (isSlotSold) {
        Color soldBg = { 20, 20, 20, 255 };
        Color soldText = { 100, 40, 40, 255 };
        DrawRectangleRec(btnRect, Config::colorBg);
        DrawRectangleLinesEx(btnRect, 1, dimColor);
        DrawButton(innerBtn, ButtonType::TextGeneric, alpha, soldBg, soldBg, soldText, soldText, "SOLD", 18);
    } else {
        DrawRectangleRec(btnRect, Config::colorBg);
        DrawRectangleLinesEx(btnRect, 1, dimColor);

        if (hasFunds && activedaemoninfo.daemons.size()<5) {
            buyClicked = DrawButton(innerBtn, ButtonType::TextGeneric, alpha, Config::colorButtonBg, Config::COLOR_GRID_LINE, mainColor, mainColor, "BUY", 18);
        } else if (!hasFunds) {
            Color lockedBg = { 30, 30, 30, 255 };
            Color lockedText = { 65, 65, 65, 255 };
            DrawButton(innerBtn, ButtonType::TextGeneric, alpha, lockedBg, lockedBg, lockedText, lockedText, "BUY", 18);
            DrawText("INSUFFICIENT FUNDS", btnRect.x + (btnRect.width - MeasureText("INSUFFICIENT FUNDS", 10)) / 2, btnRect.y + 60, 10, Config::colorRedAlert);
        } else {
            Color lockedBg = { 30, 30, 30, 255 };
            Color lockedText = { 65, 65, 65, 255 };
            DrawButton(innerBtn, ButtonType::TextGeneric, alpha, lockedBg, lockedBg, lockedText, lockedText, "BUY", 18);
            DrawText("MAX SLOTS REACHED", btnRect.x + (btnRect.width - MeasureText("INSUFFICIENT FUNDS", 10)) / 2, btnRect.y + 60, 10, (Color){150,150,150,255});
        }
    }

    if (buyClicked && !isSlotSold && activedaemoninfo.daemons.size()<5) {
        Daemon stagingbuy = iteminfo;
        stagingbuy.slot = activedaemoninfo.daemons.size()+1;
        stagingbuy.updateYPosition();
        activedaemoninfo.daemons.push_back(stagingbuy);
        gamestate.balance -= iteminfo.price;

        isSlotSold = true;
    }

    Color mainFillColor = Config::colorBg;
    if (isHovered) {
        mainFillColor = (Color){
            static_cast<unsigned char>(Config::colorBg.r + (255 - Config::colorBg.r) * 0.15f),
            static_cast<unsigned char>(Config::colorBg.g + (255 - Config::colorBg.g) * 0.15f),
            static_cast<unsigned char>(Config::colorBg.b + (255 - Config::colorBg.b) * 0.15f),
            255
        };
    }

    DrawRectangleRec(mainRect, mainFillColor);
    DrawRectangleLinesEx(mainRect, 1, dimColor);
    if (isHovered) {
        DrawRectangleLinesEx(mainRect, 2, mainColor);
    }
    DrawRectangle(mainRect.x, mainRect.y, 10, 3, mainColor);
    DrawRectangle(mainRect.x, mainRect.y, 3, 10, mainColor);
    DrawRectangle(mainRect.x + mainRect.width - 10, mainRect.y + mainRect.height - 3, 10, 3, mainColor);
    const float targetIconSize = 48.0f;
    Vector2 iconPos = { mainRect.x + 14, mainRect.y + (mainRect.height - targetIconSize) / 2 };

    DrawRectangleLinesEx((Rectangle){ iconPos.x, iconPos.y, targetIconSize, targetIconSize }, 2, mainColor);

    if (iteminfo.iconMatrix != nullptr) {
        const IconGrid& grid = *iteminfo.iconMatrix;
        const float pixelScale = targetIconSize / 16.0f;

        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                if (grid[y][x]) {
                    DrawRectangle(
                        (int)roundf(iconPos.x + (x * pixelScale)),
                        (int)roundf(iconPos.y + (y * pixelScale)),
                        (int)ceilf(pixelScale),
                        (int)ceilf(pixelScale),
                        mainColor
                    );
                }
            }
        }
    }

    float textStartX = iconPos.x + targetIconSize + 16;

    DrawText(iteminfo.GetName().c_str(), textStartX, mainRect.y + 12, 20, mainColor);
    DrawText(iteminfo.GetDesc().c_str(), textStartX, mainRect.y + 40, 11, textDim);

    int lvlWidth = MeasureText(std::to_string(iteminfo.GetLevel()).c_str(), 13);
    DrawText(std::to_string(iteminfo.GetLevel()).c_str(), mainRect.x + mainRect.width - lvlWidth - 14, mainRect.y + 12, 13, textDim);

    char costTxt[16];
    sprintf(costTxt, "$%d", iteminfo.price);
    int costWidth = MeasureText(costTxt, 22);
    DrawText(costTxt, mainRect.x + mainRect.width - costWidth - 14, mainRect.y + 44, 22, mainColor);

    if (isSlotSold) {
        DrawRectangleRec(mainRect, (Color){ 10, 10, 10, 170 });
        int bannerWidth = MeasureText("OUT OF STOCK", 16);
        DrawRectangle(mainRect.x + 12, mainRect.y + (mainRect.height - 24) / 2, bannerWidth + 16, 24, (Color){ 45, 12, 12, 230 });
        DrawText("OUT OF STOCK", mainRect.x + 20, mainRect.y + (mainRect.height - 16) / 2, 16, Config::colorRedAlert);
    }

    hoverState.update();
}

void GenerateConsumableShopPool() {
    std::vector<int> pool;
    for (int i = 0; i < (int)consumableShopPool.size(); i++) {
        pool.push_back(i);
    }

    for (int i = 0; i < 4; i++) {
        consumableShopSlots[i] = -1;
        consumableSold[i] = false;
    }
    if (pool.empty()) return;

    for (int i = 0; i < 4; i++) {
        if (pool.empty()) break;
        int randomIndex = GetRandomValue(0, (int)pool.size() - 1);
        consumableShopSlots[i] = pool[randomIndex];
        pool.erase(pool.begin() + randomIndex);
    }
}

void GenerateShopPool() {
    std::vector<int> pool;

    for (size_t i = 0; i < engine.daemons.size(); ++i) {
        std::string name = engine.daemons[i].GetName();
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);

        if (name.find("TEST") != std::string::npos) continue;

        if (!gamestate.allowduplicateshopitems) {
            bool alreadyOwned = false;
            for (size_t j = 0; j < activedaemoninfo.daemons.size(); ++j) {
                std::string ownedName = activedaemoninfo.daemons[j].GetName();
                std::transform(ownedName.begin(), ownedName.end(), ownedName.begin(), ::toupper);

                if (name == ownedName) {
                    alreadyOwned = true;
                    break;
                }
            }
            if (alreadyOwned) continue;
        }

        pool.push_back(static_cast<int>(i));
    }

    for (int i = 0; i < 5; i++) {
        shopstate.slots[i] = -1;
        shopstate.sold[i] = false;
    }
    if (pool.empty()) return;

    for (int i = 0; i < 5; i++) {
        int randomIndex = GetRandomValue(0, static_cast<int>(pool.size()) - 1);
        shopstate.slots[i] = pool[randomIndex];
        if (!gamestate.allowduplicateshopitems && pool.size() > 1 && static_cast<int>(pool.size()) > (5 - i)) {
            pool.erase(pool.begin() + randomIndex);
        }
    }
}

void DrawShopConsumableItem(Rectangle slotRect, const ShopConsumableEntry& item, bool& isSlotSold, smartbool& hoverState) {
    Color mainColor = item.color;
    Color dimColor  = { (unsigned char)(mainColor.r * 0.4f), (unsigned char)(mainColor.g * 0.4f), (unsigned char)(mainColor.b * 0.4f), 255 };
    Color textDim   = { (unsigned char)(mainColor.r * 0.7f), (unsigned char)(mainColor.g * 0.7f), (unsigned char)(mainColor.b * 0.7f), 255 };

    Vector2 mousePos = GetMousePosition();
    bool rawHover = CheckCollisionPointRec(mousePos, slotRect) && !isSlotSold;
    hoverState = rawHover;
    if (hoverState.is_new_true()) {
        playsoundsmart(hoversound, .1, 1.6);
    }
    bool isHovered = hoverState;

    bool hasFunds = (gamestate.balance >= item.price);
    bool hasRoom = (int)activeconsumableinfo.consumables.size() < GetMaxConsumableSlots();
    bool canBuy = hasFunds && hasRoom && !isSlotSold;

    Color fillColor = Config::colorBg;
    if (isHovered) {
        fillColor = {
            (unsigned char)(Config::colorBg.r + (255 - Config::colorBg.r) * 0.15f),
            (unsigned char)(Config::colorBg.g + (255 - Config::colorBg.g) * 0.15f),
            (unsigned char)(Config::colorBg.b + (255 - Config::colorBg.b) * 0.15f),
            255
        };
    }

    float cut = slotRect.width * 0.18f;
    Vector2 pts[5] = {
        { slotRect.x, slotRect.y },
        { slotRect.x + slotRect.width - cut, slotRect.y },
        { slotRect.x + slotRect.width, slotRect.y + cut },
        { slotRect.x + slotRect.width, slotRect.y + slotRect.height },
        { slotRect.x, slotRect.y + slotRect.height }
    };

    DrawTriangleFan(pts, 5, fillColor);
    for (int i = 0; i < 5; i++) {
        DrawLineEx(pts[i], pts[(i + 1) % 5], isHovered ? 2.0f : 1.0f, isHovered ? mainColor : dimColor);
    }

    DrawRectangle(slotRect.x, slotRect.y + slotRect.height - 3, 10, 3, mainColor);
    DrawRectangle(slotRect.x, slotRect.y + slotRect.height - 10, 3, 10, mainColor);

    float badgeRadius = slotRect.width * 0.2f;
    Vector2 badgeCenter = { slotRect.x + slotRect.width / 2.0f, slotRect.y + slotRect.height * 0.32f };
    DrawPoly(badgeCenter, 4, badgeRadius, 45.0f, Fade(mainColor, 0.18f));
    DrawPolyLines(badgeCenter, 4, badgeRadius, 45.0f, Fade(mainColor, 0.6f));

    std::string monogram = item.name.empty() ? "" : std::string(1, (char)toupper((unsigned char)item.name[0]));
    int monoSize = (int)(badgeRadius * 0.9f);
    int monoW = MeasureText(monogram.c_str(), monoSize);
    DrawText(monogram.c_str(), badgeCenter.x - monoW / 2.0f, badgeCenter.y - monoSize / 2.0f, monoSize, mainColor);

    std::string displayName = item.name;
    for (auto& ch : displayName) ch = (char)toupper((unsigned char)ch);
    int nameFontSize = 11;
    int nameW = MeasureText(displayName.c_str(), nameFontSize);
    while (nameW > slotRect.width - 10 && nameFontSize > 7) {
        nameFontSize--;
        nameW = MeasureText(displayName.c_str(), nameFontSize);
    }
    float nameY = slotRect.y + slotRect.height * 0.58f;
    DrawText(displayName.c_str(), slotRect.x + (slotRect.width - nameW) / 2.0f, nameY, nameFontSize, mainColor);

    Rectangle priceBtn = { slotRect.x + 6, slotRect.y + slotRect.height - 28, slotRect.width - 12, 22 };
    char priceTxt[16];
    sprintf(priceTxt, "$%d", item.price);

    bool buyClicked = false;
    if (isSlotSold) {
        DrawButton(priceBtn, ButtonType::TextGeneric, 255, Color{ 20, 20, 20, 255 }, Color{ 20, 20, 20, 255 }, Color{ 100, 40, 40, 255 }, Color{ 100, 40, 40, 255 }, "OWNED", 11);
    } else if (canBuy) {
        buyClicked = DrawButton(priceBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, mainColor, mainColor, priceTxt, 12);
    } else {
        Color lockedBg = { 30, 30, 30, 255 };
        Color lockedText = { 65, 65, 65, 255 };
        DrawButton(priceBtn, ButtonType::TextGeneric, 255, lockedBg, lockedBg, lockedText, lockedText, priceTxt, 12);
    }

    if (buyClicked) {
        Consumable purchased(item.name, item.description, item.color, item.sellValue, item.effectType, item.useFn);
        if (TryAddConsumable(purchased)) {
            gamestate.balance -= item.price;
            isSlotSold = true;
        }
    }

    if (isHovered) {
        float boxW = 200.0f;
        float paddingX = 8.0f;
        float paddingY = 8.0f;
        float fontSize = 10.0f;
        float screenMargin = 8.0f;

        std::string status = item.description;
        if (isSlotSold) status += "  [ALREADY OWNED]";
        else if (!hasFunds) status += "  [INSUFFICIENT FUNDS]";
        else if (!hasRoom) status += "  [CONSUMABLE SLOTS FULL]";

        Font font = GetFontDefault();
        float maxTextWidth = boxW - (paddingX * 2.0f);
        std::vector<std::string> lines;
        std::string currentLine;
        std::string word;
        std::stringstream ss(status);

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

        float boxX = slotRect.x + slotRect.width / 2.0f - boxW / 2.0f;
        if (boxX + boxW > GetScreenWidth() - screenMargin) boxX = GetScreenWidth() - screenMargin - boxW;
        if (boxX < screenMargin) boxX = screenMargin;

        float boxY = slotRect.y - boxH - 8.0f;
        if (boxY < screenMargin) boxY = slotRect.y + slotRect.height + 8.0f;

        DrawRectangle(boxX, boxY, boxW, boxH, Color{ 6, 12, 22, 250 });
        DrawRectangleLinesEx({ boxX, boxY, boxW, boxH }, 1.0f, mainColor);

        float lineY = boxY + paddingY;
        for (const auto& line : lines) {
            DrawTextEx(font, line.c_str(), { boxX + paddingX, lineY }, fontSize, 1.0f, textDim);
            lineY += lineHeight;
        }
    }

    hoverState.update();
}

void drawshop() {
    UpdateRerollGlitch();
    DrawText("BLACK MARKET", 200, 25, 50, WHITE);

    if (!activedaemoninfo.daemons.empty() &&
        shopstate.slots[0] == -1 && shopstate.slots[1] == -1 &&
        shopstate.slots[2] == -1 && shopstate.slots[3] == -1 &&
        shopstate.slots[4] == -1) {
        GenerateShopPool();
    }
    if (consumableShopSlots[0] == -1 && consumableShopSlots[1] == -1 &&
        consumableShopSlots[2] == -1 && consumableShopSlots[3] == -1) {
        GenerateConsumableShopPool();
    }
    for (int i = 0; i < 5; ++i) {
        int daemonIdx = shopstate.slots[i];
        if (daemonIdx != -1) {
            DrawShopItem(
                (Vector2){ 75, Config::shopitemsYbuffer + (80 * i) },
                engine.daemons[daemonIdx],
                shopstate.sold[i],
                shopstate.hoverStates[i]
            );
        }
    }
    const int consumableSlotCount = 4;
    float footprintWidth = Config::shopitemtotalWidth;
    float rawSlotSize = (footprintWidth - Config::consumablesgap * (consumableSlotCount - 1)) / consumableSlotCount;
    float usedWidth = Config::consumableitemsize * consumableSlotCount + Config::consumablesgap * (consumableSlotCount - 1);
    float startX = 75 + (footprintWidth - usedWidth) / 2.0f;
    float consumableRowY = Config::shopitemsYbuffer + (80 * 5) + 50.0f;

    for (int i = 0; i < consumableSlotCount; ++i) {
        int poolIdx = consumableShopSlots[i];
        if (poolIdx == -1) continue;
        Rectangle slotRect = { startX + i * (Config::consumableitemsize + Config::consumablesgap), consumableRowY, Config::consumableitemsize, Config::consumableitemsize };
        DrawShopConsumableItem(slotRect, consumableShopPool[poolIdx], consumableSold[i], consumableHoverStates[i]);
    }
    float glitchIntensity = GetRerollGlitchIntensity();
    if (glitchIntensity > 0.0f) {
        int clipX = 70;
        int clipY = (int)Config::shopitemsYbuffer - 5;
        int clipW = Config::shopitemtotalWidth + 15;
        int clipH = (int)(consumableRowY + Config::consumableitemsize - clipY) + 10;

        BeginScissorMode(clipX, clipY, clipW, clipH);

        int barCount = (int)(glitchIntensity * 8.0f);
        for (int i = 0; i < barCount; i++) {
            int barY = GetRandomValue(clipY, clipY + clipH);
            int barHeight = GetRandomValue(3, 14);
            int xOffset = GetRandomValue(-15, 15);

            DrawRectangle(clipX + xOffset, barY, clipW, barHeight, (Color){ 0, 255, 120, (unsigned char)(180 * glitchIntensity) });
        }

        int sliceCount = (int)(glitchIntensity * 4.0f);
        for (int i = 0; i < sliceCount; i++) {
            int sliceY = GetRandomValue(clipY, clipY + clipH - 8);
            int sliceHeight = GetRandomValue(4, 10);
            int shift = GetRandomValue(4, 14);

            DrawRectangle(clipX + shift, sliceY, clipW, sliceHeight, Fade(RED, 0.35f * glitchIntensity));
            DrawRectangle(clipX - shift, sliceY, clipW, sliceHeight, Fade((Color){0,180,255,255}, 0.35f * glitchIntensity));
        }

        if (glitchIntensity > 0.5f) {
            DrawRectangle(clipX, clipY, clipW, clipH, Fade(WHITE, (glitchIntensity - 0.5f) * 0.4f));
        }

        EndScissorMode();
    }

    //next
    if (DrawButton({1045, Config::walletY - 77, 205, 65}, ButtonType::TextGeneric, 255, Config::COLOR_GRID_LINE, Config::COLOR_UI_AMBER, Config::COLOR_UI_GREEN, WHITE, "Next", 35)) {
        RequestGameStateChange(MAP);
        for (int i = 0; i < 5; i++) shopstate.slots[i] = -1;
        return;
    }
    
    const static int rerollsprice = 500;
    bool affordable = false;
    int currentrerollprice= 900+shopstate.rerolls*rerollsprice;
    if (currentrerollprice<=gamestate.balance){
        affordable = true;
    }
    std::string rerollstring ="Reroll $" + std::to_string(currentrerollprice);
    
    //reroll
    if (DrawButton({830, Config::walletY - 77, 205, 65},
                    ButtonType::TextGeneric, 255, 
                    (affordable) ? Config::COLOR_GRID_LINE: Config::COLOR_GRID_LINE_DARKER, 
                    (affordable) ? Config::COLOR_UI_AMBER : Config::COLOR_GRID_LINE_DARKER, 
                    Config::COLOR_UI_GREEN, WHITE, 
                    rerollstring.c_str(), 35)) {
        if (affordable){
            gamestate.balance -=currentrerollprice;
            shopstate.rerolls +=1;
            GenerateShopPool();
            GenerateConsumableShopPool();
            rerollGlitch.active = true;
            rerollGlitch.timer = 0.0f;
        }
    }
}
