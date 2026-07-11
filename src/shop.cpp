#include "shop.hpp"
#include "raylib.h"
#include <string>
#include <vector>
#include "variables.hpp"
#include "button.hpp"

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

void drawshop(){
    static bool slotSoldOut[5] = { false, false, false, false, false };

    DrawText("BLACK MARKET", 200,25,50,WHITE);
    DrawShopItem((Vector2){ 75, Config::shopitemsYbuffer }, engine.daemons[2], slotSoldOut[0]);
    DrawShopItem((Vector2){ 75, Config::shopitemsYbuffer+80 }, engine.daemons[4], slotSoldOut[1]);
    DrawShopItem((Vector2){ 75, Config::shopitemsYbuffer+80*2 }, engine.daemons[1], slotSoldOut[2]);
    DrawShopItem((Vector2){ 75, Config::shopitemsYbuffer+80*3 }, engine.daemons[3], slotSoldOut[3]);
    DrawShopItem((Vector2){ 75, Config::shopitemsYbuffer+80*4 }, engine.daemons[0], slotSoldOut[4]);

    if (DrawButton({1045,Config::walletY-77,205,65},
                    ButtonType::TextGeneric,
                    255,Config::COLOR_GRID_LINE,
                    Config::COLOR_UI_AMBER,
                    Config::COLOR_UI_GREEN,
                    WHITE,"Next",40)){
        for(int i = 0; i < 5; i++) slotSoldOut[i] = false;
        gamestate.gamestate=GAME;
    }

    if (DrawButton({830,Config::walletY-77,205,65},
                    ButtonType::TextGeneric,
                    255,Config::COLOR_GRID_LINE,
                    Config::COLOR_UI_AMBER,
                    Config::COLOR_UI_GREEN,
                    WHITE,"Reroll",40)){
        TraceLog(LOG_INFO,"LOG Reroll");
        for(int i = 0; i < 5; i++) slotSoldOut[i] = false; 
    }
}