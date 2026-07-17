#include "shop.hpp"
#include "raylib.h"
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include "variables.hpp"
#include "button.hpp"
#include "main.hpp"

void DrawShopItem(Vector2 pos, const Daemon& iteminfo, bool& isSlotSold) {
    const float gap = 8.0f;
    const float mainBoxWidth = Config::shopitemtotalWidth - Config::shopbuyitembuttonWidth - gap;
    Color mainColor   = iteminfo.GetColor();
    Color dimColor    = (Color){ static_cast<unsigned char>(mainColor.r * 0.4f), static_cast<unsigned char>(mainColor.g * 0.4f), static_cast<unsigned char>(mainColor.b * 0.4f), 255 };
    Color textDim     = (Color){ static_cast<unsigned char>(mainColor.r * 0.7f), static_cast<unsigned char>(mainColor.g * 0.7f), static_cast<unsigned char>(mainColor.b * 0.7f), 255 };
    
    Rectangle btnRect = { pos.x+Config::shopitemtotalWidth-Config::shopbuyitembuttonWidth, pos.y, Config::shopbuyitembuttonWidth, Config::shopitemtotalHeight };
    Rectangle mainRect = { pos.x, pos.y, mainBoxWidth, Config::shopitemtotalHeight };

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

        if (hasFunds) {
            buyClicked = DrawButton(innerBtn, ButtonType::TextGeneric, alpha, Config::colorButtonBg, Config::COLOR_GRID_LINE, mainColor, mainColor, "BUY", 18);
        } else {
            Color lockedBg = { 30, 30, 30, 255 };
            Color lockedText = { 65, 65, 65, 255 };
            DrawButton(innerBtn, ButtonType::TextGeneric, alpha, lockedBg, lockedBg, lockedText, lockedText, "BUY", 18);
            DrawText("INSUFFICIENT FUNDS", btnRect.x + (btnRect.width - MeasureText("INSUFFICIENT FUNDS", 10)) / 2, btnRect.y + 60, 10, Config::colorRedAlert);
        }
    }

    if (buyClicked && !isSlotSold) {
        Daemon stagingbuy = iteminfo;
        stagingbuy.slot = activedaemoninfo.daemons.size()+1;
        stagingbuy.updateYPosition();
        activedaemoninfo.daemons.push_back(stagingbuy);
        gamestate.balance -= iteminfo.price;
        
        isSlotSold = true; 
    }
    // if (iteminfo.iconMatrix != nullptr) {
    //     const IconGrid& grid = *iteminfo.iconMatrix;
    //     for (int y = 0; y < 16; ++y) {
    //         for (int x = 0; x < 16; ++x) {
    //             if (grid[y][x]) {
    //                 DrawRectangle(
    //                     iconPos.x + (x * pixelScale),
    //                     iconPos.y + (y * pixelScale),
    //                     pixelScale,
    //                     pixelScale,
    //                     mainColor
    //                 );
    //             }
    //         }
    //     }
    // }
    DrawRectangleRec(mainRect, Config::colorBg);
    DrawRectangleLinesEx(mainRect, 1, dimColor);
    DrawRectangle(mainRect.x, mainRect.y, 10, 3, mainColor);
    DrawRectangle(mainRect.x, mainRect.y, 3, 10, mainColor);
    DrawRectangle(mainRect.x + mainRect.width - 10, mainRect.y + mainRect.height - 3, 10, 3, mainColor);
    const float targetIconSize = 48.0f; 
    Vector2 iconPos = { mainRect.x + 14, mainRect.y + (mainRect.height - targetIconSize) / 2 };

    DrawRectangleLinesEx((Rectangle){ iconPos.x, iconPos.y, targetIconSize, targetIconSize }, 2, mainColor);
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

void drawshop() {
    DrawText("BLACK MARKET", 200, 25, 50, WHITE);

    if (!activedaemoninfo.daemons.empty() &&
        shopstate.slots[0] == -1 && shopstate.slots[1] == -1 &&
        shopstate.slots[2] == -1 && shopstate.slots[3] == -1 &&
        shopstate.slots[4] == -1) {
        GenerateShopPool();
    }

    for (int i = 0; i < 5; ++i) {
        int daemonIdx = shopstate.slots[i];
        if (daemonIdx != -1) {
            DrawShopItem(
                (Vector2){ 75, Config::shopitemsYbuffer + (80 * i) }, 
                engine.daemons[daemonIdx], 
                shopstate.sold[i]
            );
        }
    }
    //next
    if (DrawButton({1045, Config::walletY - 77, 205, 65}, ButtonType::TextGeneric, 255, Config::COLOR_GRID_LINE, Config::COLOR_UI_AMBER, Config::COLOR_UI_GREEN, WHITE, "Next", 35)) {
        RequestGameStateChange(MAP);
        for (int i = 0; i < 5; i++) shopstate.slots[i] = -1;
        return;
    }
    
    const static int rerollsprice = 100;
    bool affordable = false;
    if (100+shopstate.rerolls*rerollsprice<=gamestate.balance){
        affordable = true;
    }
    std::string rerollstring ="Reroll $" + std::to_string(100+(shopstate.rerolls*rerollsprice));
    
    //reroll
    if (DrawButton({830, Config::walletY - 77, 205, 65},
                    ButtonType::TextGeneric, 255, 
                    (affordable) ? Config::COLOR_GRID_LINE: Config::COLOR_GRID_LINE_DARKER, 
                    (affordable) ? Config::COLOR_UI_AMBER : Config::COLOR_GRID_LINE_DARKER, 
                    Config::COLOR_UI_GREEN, WHITE, 
                    rerollstring.c_str(), 35)) {
        if (affordable){
            gamestate.balance -=100+shopstate.rerolls*rerollsprice;
            shopstate.rerolls +=1;
            GenerateShopPool();
        }
    }
}
