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
#include "custom-polygon-generator.hpp"
#include "screenshake.hpp"

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
    int maxTargets = 1;
};

Texture2D GetShopItemSprite(bool pressed) {
    static Texture2D tex;
    static Texture2D texpressed;
    static bool loaded = false;
    if (!loaded) {
        tex = LoadTexture("assets/shop-item.png");
        texpressed = LoadTexture("assets/shop-item-pressed.png");
        loaded = true;
    }
    return !pressed ? tex : texpressed;
}

Texture2D GetConsumableSlotSprite() {
    static Texture2D tex;
    static bool loaded = false;
    if (!loaded) {
        tex = LoadTexture("assets/consumable-shop-item.png");
        loaded = true;
    }
    return tex;
}

static std::vector<ShopConsumableEntry> consumableShopPool = {
    { "Reroll", "Reroll all shop offers once", Config::COLOR_UI_AMBER, ConsumableEffectType::INSTANT, UseRerollCharge, 120, 250 },
    { "Overclock", "Temporarily overclock a random daemon", Config::MAGENTA_DAEMON, ConsumableEffectType::INSTANT, UseOverclockBooster, 180, 350 },
    { "Board Wipe", "Clear all active glitch modifiers from the board", Config::COLOR_UI_GREEN, ConsumableEffectType::BOARD_TARGET, UseBoardWipeCharge, 200, 400 },
    { "Fire Sale", "every daemon in your hand adds its full sell value to your balance", Config::COLOR_UI_AMBER, ConsumableEffectType::INSTANT, firesale, 60, 300 },
    { "Decrypt", "Select an encrypted node on the map to reveal it", Config::MAGENTA_DAEMON, ConsumableEffectType::BOARD_TARGET, UseDecryptNode, 150, 450 },
    { "Overclock Pin", "Set a pin's modifier to a flat boosted payout", Config::COLOR_UI_GREEN, ConsumableEffectType::BOARD_TARGET, SetNodeModifierBoost, 130, 260 },
    { "Volatile Pin", "Set a pin's modifier to an unstable, random payout", Config::COLOR_UI_AMBER, ConsumableEffectType::BOARD_TARGET, SetNodeModifierGlitch, 130, 260,2 },
    { "Mitosis Pin", "Set a pin's modifier to split probes into clones", Config::MAGENTA_DAEMON, ConsumableEffectType::BOARD_TARGET, SetNodeModifierClone, 150, 320,2 }
};

static int consumableShopSlots[4] = { -1, -1, -1, -1 };
static bool consumableSold[4] = { false, false, false, false };
static smartbool consumableHoverStates[4];

float GetRerollGlitchIntensity() {
    if (!rerollGlitch.active) return 0.0f;
    float t = rerollGlitch.timer / rerollGlitch.duration;
    if (t >= 1.0f) return 0.0f;
    return sinf(t * PI);
}

void UpdateRerollGlitch() {
    if (!rerollGlitch.active) return;
    rerollGlitch.timer += GetFrameTime();
    if (rerollGlitch.timer >= rerollGlitch.duration) {
        rerollGlitch.active = false;
        rerollGlitch.timer = 0.0f;
    }
}

void DrawSpriteWithHueShader(Texture2D texture, Rectangle srcRect, Rectangle destRect, Color baseColor, Shader shader, int hueLoc) {
    Vector3 hsv = ColorToHSV(baseColor);
    float hue = 0.2f + hsv.x / 360.0f;
    SetShaderValue(shader, hueLoc, &hue, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(shader);
    DrawTexturePro(texture, srcRect, destRect, { 0, 0 }, 0.0f, WHITE);
    EndShaderMode();
}

void HandleHoverSoundTrigger(smartbool& hoverState, bool rawHover, bool conditionsMet = true) {
    hoverState = rawHover;
    if (hoverState.is_new_true() && conditionsMet) {
        playsoundsmart(hoversound, 0.1f, 1.6f);
    }
}

std::string ToUpperString(std::string text) {
    for (auto& ch : text) {
        ch = (char)toupper((unsigned char)ch);
    }
    return text;
}

void DrawIconMatrix(const IconGrid& grid, Vector2 position, float targetSize, Color color) {
    const float pixelScale = targetSize / 16.0f;
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            if (grid[y][x]) {
                DrawRectangle(
                    (int)roundf(position.x + (x * pixelScale)),
                    (int)roundf(position.y + (y * pixelScale)),
                    (int)ceilf(pixelScale),
                    (int)ceilf(pixelScale),
                    color
                );
            }
        }
    }
}

static std::vector<std::string> WrapText(const std::string& text, Font font, float fontSize, float maxWidth) {
    std::vector<std::string> lines;
    std::string currentLine;
    std::string word;
    std::stringstream ss(text);

    while (ss >> word) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        Vector2 size = MeasureTextEx(font, testLine.c_str(), fontSize, 1.0f);
        if (size.x > maxWidth) {
            if (!currentLine.empty()) lines.push_back(currentLine);
            currentLine = word;
        } else {
            currentLine = testLine;
        }
    }
    if (!currentLine.empty()) lines.push_back(currentLine);
    return lines;
}

static void DrawConsumableTooltip(Rectangle slotRect, const ShopConsumableEntry& item, bool isSlotSold, bool hasFunds, bool hasRoom, Color mainColor, Color textDim) {
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
    std::vector<std::string> lines = WrapText(status, font, fontSize, maxTextWidth);

    float lineHeight = fontSize * 1.5f;
    float boxH = (lines.size() * lineHeight) + (paddingY * 2.0f) - (lineHeight - fontSize);

    float boxX = slotRect.x + slotRect.width / 2.0f - boxW / 2.0f;
    if (boxX + boxW > GetScreenWidth() - screenMargin) boxX = GetScreenWidth() - screenMargin - boxW;
    if (boxX < screenMargin) boxX = screenMargin;

    float boxY = slotRect.y - boxH - 8.0f;
    if (boxY < screenMargin) boxY = slotRect.y + slotRect.height + 8.0f;

    DrawRectangle((int)boxX, (int)boxY, (int)boxW, (int)boxH, Color{ 6, 12, 22, 250 });
    DrawRectangleLinesEx({ boxX, boxY, boxW, boxH }, 1.0f, mainColor);

    float lineY = boxY + paddingY;
    for (const auto& line : lines) {
        DrawTextEx(font, line.c_str(), { boxX + paddingX, lineY }, fontSize, 1.0f, textDim);
        lineY += lineHeight;
    }
}

void DrawShopConsumableItem(Rectangle slotRect, const ShopConsumableEntry& item, bool& isSlotSold, smartbool& hoverState) {
    Color mainColor = item.color;
    Color textDim   = { (unsigned char)(mainColor.r * 0.7f), (unsigned char)(mainColor.g * 0.7f), (unsigned char)(mainColor.b * 0.7f), 255 };

    Vector2 mousePos = GetMousePosition();
    bool rawHover = CheckCollisionPointRec(mousePos, slotRect) && !isSlotSold;
    bool isHovered = hoverState;
    bool hasFunds = (gamestate.balance >= item.price);
    bool hasRoom = (int)activeconsumableinfo.consumables.size() < GetMaxConsumableSlots();
    bool canBuy = hasFunds && hasRoom && !isSlotSold;
    HandleHoverSoundTrigger(hoverState, rawHover, hasRoom&&hasFunds);

    Texture2D slotSprite = GetConsumableSlotSprite();
    Rectangle spriteSrc = { 0, 0, (float)slotSprite.width, (float)slotSprite.height };
    DrawSpriteWithHueShader(slotSprite, spriteSrc, slotRect, mainColor, hueShader, hueLoc);


    Polygon itemPoly({
        { slotRect.x, slotRect.y },
        { slotRect.x, slotRect.y + 134 },
        { slotRect.x + slotRect.width, slotRect.y + 134 },
        { slotRect.x + slotRect.width, slotRect.y + 23 },
        { slotRect.x + 127, slotRect.y }
    });

    Polygon btnPoly({
        { slotRect.x + 4, slotRect.y + 136 },
        { slotRect.x + 4, slotRect.y + 158 },
        { slotRect.x + 10, slotRect.y + 164 },
        { slotRect.x + 134, slotRect.y + 164 },
        { slotRect.x + 142, slotRect.y + 158 },
        { slotRect.x + 142, slotRect.y + 136 }
    });

    bool isbtnhovered = btnPoly.CheckCollisionPoint(mousePos);

    if (itemPoly.CheckCollisionPoint(mousePos) && hasFunds && hasRoom && !isSlotSold) {
        itemPoly.SetFillColor(Fade(WHITE, 0.35f));
        itemPoly.Draw();
    }
    if (isbtnhovered && hasFunds && hasRoom && !isSlotSold) {
        btnPoly.SetFillColor(Fade(WHITE, 0.35f));
        btnPoly.Draw();
    }

    float badgeRadius = slotRect.width * 0.2f;
    Vector2 badgeCenter = { slotRect.x + slotRect.width / 2.0f, slotRect.y + slotRect.height * 0.32f };
    DrawPoly(badgeCenter, 4, badgeRadius, 45.0f, Fade(mainColor, 0.18f));
    DrawPolyLines(badgeCenter, 4, badgeRadius, 45.0f, Fade(mainColor, 0.6f));

    std::string monogram = item.name.empty() ? "" : std::string(1, (char)toupper((unsigned char)item.name[0]));
    int monoSize = (int)(badgeRadius * 0.9f);
    int monoW = MeasureText(monogram.c_str(), monoSize);
    DrawText(monogram.c_str(), badgeCenter.x - monoW / 2.0f, badgeCenter.y - monoSize / 2.0f, monoSize, mainColor);

    std::string displayName = ToUpperString(item.name);
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
    float txtW = MeasureText(priceTxt, 12);
    if (isSlotSold) {
        itemPoly.SetFillColor(Fade(BLACK, 0.75f));
        itemPoly.Draw();
        btnPoly.SetFillColor(Fade(BLACK, 0.75f));
        btnPoly.Draw();
        //DrawText("Owned", slotRect.x + slotRect.width/2-txtW/2, slotRect.y + slotRect.height - 20, 14, mainColor);
    } else if (!hasFunds) {
        DrawText(priceTxt, slotRect.x + slotRect.width/2-txtW/2, slotRect.y + slotRect.height - 20, 14, mainColor);
        itemPoly.SetFillColor(Fade(BLACK, 0.35f));
        itemPoly.Draw();
        btnPoly.SetFillColor(Fade(BLACK, 0.35f));
        btnPoly.Draw();
        Vector2 textSize = MeasureTextEx(GetFontDefault(), "$", 60.0f,2.0f);
        DrawText("$", slotRect.x + slotRect.width/2-textSize.x/2, slotRect.y + slotRect.height/2-textSize.y/2-20, 60, Config::colorRedAlert);
    } else if (!hasRoom){
        DrawText(priceTxt, slotRect.x + slotRect.width/2-txtW/2, slotRect.y + slotRect.height - 20, 14, mainColor);
        itemPoly.SetFillColor(Fade(BLACK, 0.35f));
        itemPoly.Draw();
        btnPoly.SetFillColor(Fade(BLACK, 0.35f));
        btnPoly.Draw();
        Vector2 textSize = MeasureTextEx(GetFontDefault(), "FULL", 50.0f,2.0f);
        DrawText("FULL", slotRect.x + slotRect.width/2-textSize.x/2, slotRect.y + slotRect.height/2-textSize.y/2-20, 50, (Color){150,150,150,255});
    } else{
        DrawText(priceTxt, slotRect.x + slotRect.width/2-txtW/2, slotRect.y + slotRect.height - 20, 14, mainColor);
        buyClicked = btnPoly.IsReleased(MOUSE_BUTTON_LEFT);
    }

    if (buyClicked) {
        Consumable purchased(item.name, item.description, item.color, item.sellValue, item.effectType, item.useFn);
        purchased.maxTargets = item.maxTargets;
        if (TryAddConsumable(purchased)) {
            gamestate.balance -= item.price;
            isSlotSold = true;
            TriggerGlitchAt(slotRect, 0.3f);
            screenshake(3.0f, 0.3f);
        }
    }

    if (isHovered&&hasFunds&&hasRoom) {
        DrawConsumableTooltip(slotRect, item, isSlotSold, hasFunds, hasRoom, mainColor, textDim);
    }
    hoverState.update();
}

void DrawShopItem(Vector2 pos, const Daemon& iteminfo, bool& isSlotSold, smartbool& hoverState) {
    Rectangle destRect = { pos.x, pos.y, Config::shopitemtotalWidth, Config::shopitemtotalHeight };
    Vector2 mousePos = GetMousePosition();


    Polygon btnPoly({
        { Config::shopitemsXbuffer + 692, pos.y + 2 + 15 },
        { Config::shopitemsXbuffer + 581, pos.y + 2 + 15 },
        { Config::shopitemsXbuffer + 559, pos.y + 2 + 36 },
        { Config::shopitemsXbuffer + 559, pos.y + 2 + 59 },
        { Config::shopitemsXbuffer + 671, pos.y + 2 + 59 },
        { Config::shopitemsXbuffer + 692, pos.y + 2 + 38 }
    });

    Polygon btnPolyPressed({
        { Config::shopitemsXbuffer + 692 - 8, pos.y + 2 + 15 + 7 },
        { Config::shopitemsXbuffer + 581 - 8, pos.y + 2 + 15 + 7 },
        { Config::shopitemsXbuffer + 559 - 8, pos.y + 2 + 36 + 7 },
        { Config::shopitemsXbuffer + 559 - 8, pos.y + 2 + 59 + 7 },
        { Config::shopitemsXbuffer + 671 - 8, pos.y + 2 + 59 + 7 },
        { Config::shopitemsXbuffer + 692 - 8, pos.y + 2 + 38 + 7 }
    });

    Polygon itemPoly({
        { Config::shopitemsXbuffer, pos.y },
        { Config::shopitemsXbuffer, pos.y + Config::shopitemtotalHeight },
        { Config::shopitemsXbuffer + 678, pos.y + Config::shopitemtotalHeight },
        { Config::shopitemsXbuffer + Config::shopitemtotalWidth, pos.y + 56 },
        { Config::shopitemsXbuffer + Config::shopitemtotalWidth, pos.y }
    });

    Polygon mainHoverPoly({
        { Config::shopitemsXbuffer, pos.y },
        { Config::shopitemsXbuffer, pos.y + Config::shopitemtotalHeight },
        { Config::shopitemsXbuffer + 371, pos.y + Config::shopitemtotalHeight },
        { Config::shopitemsXbuffer + 455, pos.y }
    });

    bool isBtnHovered = btnPoly.CheckCollisionPoint(mousePos);
    bool isMainHovered = mainHoverPoly.CheckCollisionPoint(mousePos);
    bool rawHover = (isBtnHovered || isMainHovered) && !isSlotSold;

    bool hasFunds = (gamestate.balance >= iteminfo.price);
    bool hasRoom = activedaemoninfo.daemons.size() < 5;
    bool buyClicked = false;

    HandleHoverSoundTrigger(hoverState, rawHover, hasFunds && hasRoom);

    bool isMouseDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    Texture2D shopitemsprite = GetShopItemSprite(isBtnHovered && isMouseDown && hasFunds && hasRoom);
    Rectangle srcRect = { 0, 0, (float)shopitemsprite.width, (float)shopitemsprite.height };

    Color mainColor = iteminfo.GetColor();
    DrawSpriteWithHueShader(shopitemsprite, srcRect, destRect, mainColor, hueShader, hueLoc);

    if (isBtnHovered && hasFunds && hasRoom) {
        if (isMouseDown) {
            btnPolyPressed.SetFillColor(Fade(WHITE, 0.35f));
            btnPolyPressed.Draw();
        } else {
            btnPoly.SetFillColor(Fade(WHITE, 0.35f));
            btnPoly.Draw();
        }
    }

    if (isMainHovered && hasFunds && hasRoom) {
        mainHoverPoly.SetFillColor(Fade(WHITE, 0.35f));
        mainHoverPoly.Draw();
    }

    const float targetIconSize = 48.0f;
    Vector2 iconPos = { destRect.x + 14, destRect.y + (destRect.height - targetIconSize) / 2.0f };

    if (iteminfo.iconMatrix != nullptr) {
        DrawIconMatrix(*iteminfo.iconMatrix, iconPos, targetIconSize, mainColor);
    }

    float textStartX = iconPos.x + targetIconSize + 16;
    DrawText(iteminfo.GetName().c_str(), textStartX, destRect.y + 12, 20, WHITE);
    DrawText(iteminfo.GetDesc().c_str(), textStartX, destRect.y + 40, 11, Fade(WHITE, 0.6f));

    std::string lvlStr = std::to_string(iteminfo.GetLevel());
    DrawText(lvlStr.c_str(), destRect.x + destRect.width - 160, destRect.y + 8, 13, Fade(WHITE, 0.5f));

    char costTxt[16];
    sprintf(costTxt, "$%d", iteminfo.price);
    int costWidth = MeasureText(costTxt, 22);
    DrawText(costTxt, destRect.x + destRect.width - 185 - costWidth, destRect.y + destRect.height - 35, 25, WHITE);

    if (isSlotSold) {
        itemPoly.SetFillColor(Fade(BLACK, 0.9f));
        itemPoly.Draw();
    } else if (!hasFunds || !hasRoom) {
        itemPoly.SetFillColor(Fade(BLACK, 0.7f));
        itemPoly.Draw();
        const char* reasonTxt = !hasFunds ? "INSUFFICIENT FUNDS" : "MAX SLOTS REACHED";
        DrawText(reasonTxt, destRect.x + Config::shopitemtotalWidth / 2 - 200, destRect.y + Config::shopitemtotalHeight / 2 - 15, 35, !hasFunds ? Config::colorRedAlert : Color{ 150, 150, 150, 255 });
    } else {
        buyClicked = btnPoly.IsReleased(MOUSE_BUTTON_LEFT) && !isSlotSold;
    }

    if (buyClicked) {
        Daemon stagingbuy = iteminfo;
        stagingbuy.slot = activedaemoninfo.daemons.size() + 1;
        stagingbuy.updateYPosition();
        activedaemoninfo.daemons.push_back(stagingbuy);
        gamestate.balance -= iteminfo.price;
        TriggerGlitchAt(destRect, 0.3f);
        screenshake(3.0f, 0.3f);
        isSlotSold = true;
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

    for (int i = 0; i < 4; i++) {
        shopstate.slots[i] = -1;
        shopstate.sold[i] = false;
    }
    if (pool.empty()) return;

    for (int i = 0; i < 4; i++) {
        int randomIndex = GetRandomValue(0, static_cast<int>(pool.size()) - 1);
        shopstate.slots[i] = pool[randomIndex];
        if (!gamestate.allowduplicateshopitems && pool.size() > 1 && static_cast<int>(pool.size()) > (5 - i)) {
            pool.erase(pool.begin() + randomIndex);
        }
    }
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
    for (int i = 0; i < 4; ++i) {
        int daemonIdx = shopstate.slots[i];
        if (daemonIdx != -1) {
            DrawShopItem(
                (Vector2){ Config::shopitemsXbuffer, Config::shopitemsYbuffer + (96 * i) },
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
    float startX = Config::shopitemsXbuffer + (footprintWidth - usedWidth) / 2.0f;
    float consumableRowY = Config::shopitemsYbuffer + (80 * 5) + 50.0f;

        
    static Texture2D consumablesbgshop = LoadTexture("assets/consumables-shop-bg.png");
    Rectangle srcRect = {0,0,544,61};
    const float baseWidth = 544.0f;
    const float baseHeight = 61.0f;

    float newWidth = static_cast<float>(Config::shopitemtotalWidth) + 10.0f;

    float scale = newWidth / baseWidth;
    float newHeight = baseHeight * scale;

    Rectangle destRect = {
        static_cast<float>(Config::shopitemsXbuffer),
        575.0f,
        newWidth,
        newHeight
    };
    DrawTexturePro(consumablesbgshop,srcRect, destRect, { 0, 0 }, 0.0f, WHITE);

    for (int i = 0; i < consumableSlotCount; ++i) {
        int poolIdx = consumableShopSlots[i];
        if (poolIdx == -1) continue;

        Rectangle slotRect = {
            Config::consumables_slots_locations[i],
            545,
            112 * scale,
            126 * scale
        };

        DrawShopConsumableItem(slotRect, consumableShopPool[poolIdx], consumableSold[i], consumableHoverStates[i]);
    }
    // float glitchIntensity = GetRerollGlitchIntensity();
    // if (glitchIntensity > 0.0f) {
    //     int clipX = 70;
    //     int clipY = (int)Config::shopitemsYbuffer - 5;
    //     int clipW = Config::shopitemtotalWidth + 15;
    //     int clipH = (int)(consumableRowY + Config::consumableitemsize - clipY) + 10;

    //     BeginScissorMode(clipX, clipY, clipW, clipH);

    //     int barCount = (int)(glitchIntensity * 8.0f);
    //     for (int i = 0; i < barCount; i++) {
    //         int barY = GetRandomValue(clipY, clipY + clipH);
    //         int barHeight = GetRandomValue(3, 14);
    //         int xOffset = GetRandomValue(-15, 15);

    //         DrawRectangle(clipX + xOffset, barY, clipW, barHeight, (Color){ 0, 255, 120, (unsigned char)(180 * glitchIntensity) });
    //     }

    //     int sliceCount = (int)(glitchIntensity * 4.0f);
    //     for (int i = 0; i < sliceCount; i++) {
    //         int sliceY = GetRandomValue(clipY, clipY + clipH - 8);
    //         int sliceHeight = GetRandomValue(4, 10);
    //         int shift = GetRandomValue(4, 14);

    //         DrawRectangle(clipX + shift, sliceY, clipW, sliceHeight, Fade(RED, 0.35f * glitchIntensity));
    //         DrawRectangle(clipX - shift, sliceY, clipW, sliceHeight, Fade((Color){0,180,255,255}, 0.35f * glitchIntensity));
    //     }

    //     if (glitchIntensity > 0.5f) {
    //         DrawRectangle(clipX, clipY, clipW, clipH, Fade(WHITE, (glitchIntensity - 0.5f) * 0.4f));
    //     }

    //     EndScissorMode();
    // }

    //next
    if (DrawButton({1045, Config::walletY - 77, 205, 65}, ButtonType::TextGeneric, 255, Config::COLOR_GRID_LINE, Config::COLOR_UI_AMBER, Config::COLOR_UI_GREEN, WHITE, "Next", 35)) {
        RequestGameStateChange(MAP);
        for (int i = 0; i < 5; i++) shopstate.slots[i] = -1;
        shopstate.rerolls =0;
        GenerateShopPool();
        GenerateConsumableShopPool();
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
            //rerollGlitch.active = true;
            //rerollGlitch.timer = 0.0f;
            playsoundsmart(transitionsound, .5,2);
            screenshake(3.0f, 0.5f);
            float clipX = 70;
            float clipY = Config::shopitemsYbuffer - 5;
            float clipW = Config::shopitemtotalWidth + 15;
            float clipH = (consumableRowY + Config::consumableitemsize - clipY) + 10;
            Rectangle bounds = { clipX,clipY,clipW,clipH };
            TriggerGlitchAt(bounds, 0.65f);
        }
    }
}
