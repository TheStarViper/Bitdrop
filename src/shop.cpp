#include "shop.hpp"
#include "raylib.h"
#include <string>
#include <vector>
#include "variables.hpp"

void DrawShopItem(Vector2 pos, Daemon iteminfo) {
    const float totalWidth = 640.0f;
    const float totalHeight = 80.0f;
    const float buttonWidth = 140.0f;
    const float gap = 8.0f;
    const float mainBoxWidth = totalWidth - buttonWidth - gap;
    Color mainColor   = iteminfo.GetColor();
    Color dimColor    = (Color){ static_cast<unsigned char>(mainColor.r * 0.4f), static_cast<unsigned char>(mainColor.g * 0.4f), static_cast<unsigned char>(mainColor.b * 0.4f), 255 };
    Color textDim     = (Color){ static_cast<unsigned char>(mainColor.r * 0.7f), static_cast<unsigned char>(mainColor.g * 0.7f), static_cast<unsigned char>(mainColor.b * 0.7f), 255 };
    


    Rectangle btnRect = { pos.x, pos.y, buttonWidth, totalHeight };
    Rectangle mainRect = { pos.x + buttonWidth + gap, pos.y, mainBoxWidth, totalHeight };
    DrawRectangleRec(btnRect, Config::colorBg);
    DrawRectangleLinesEx(btnRect, 1, dimColor);
    
    Rectangle innerBtn = { btnRect.x + 8, btnRect.y + 14, btnRect.width - 16, btnRect.height - 36 };
    DrawRectangleRec(innerBtn, Config::colorButtonBg);
    
    if (gamestate.balance>=iteminfo.price) {
        DrawText("BUY", innerBtn.x + (innerBtn.width - MeasureText("BUY", 18)) / 2, innerBtn.y + 12, 18, mainColor);
    } else {
        DrawText("BUY", innerBtn.x + (innerBtn.width - MeasureText("BUY", 18)) / 2, innerBtn.y + 12, 18, (Color){ 65, 65, 65, 255 });
        DrawText("INSUFFICIENT FUNDS", btnRect.x + (btnRect.width - MeasureText("INSUFFICIENT FUNDS", 10)) / 2, btnRect.y + 60, 10, Config::colorRedAlert);
    }

    DrawRectangleRec(mainRect, Config::colorBg);
    DrawRectangleLinesEx(mainRect, 1, dimColor);
    DrawRectangle(mainRect.x, mainRect.y, 10, 3, mainColor);
    DrawRectangle(mainRect.x, mainRect.y, 3, 10, mainColor);
    DrawRectangle(mainRect.x + mainRect.width - 10, mainRect.y + mainRect.height - 3, 10, 3, mainColor);
    const float targetIconSize = 48.0f; 
    Vector2 iconPos = { mainRect.x + 14, mainRect.y + (mainRect.height - targetIconSize) / 2 };

    DrawRectangleLinesEx((Rectangle){ iconPos.x, iconPos.y, targetIconSize, targetIconSize }, 2, mainColor);

    const float pixelScale = 3.0f; 
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
    float textStartX = iconPos.x + targetIconSize + 16;

    DrawText(iteminfo.GetName().c_str(), textStartX, mainRect.y + 12, 20, mainColor);
    DrawText(iteminfo.GetDesc().c_str(), textStartX, mainRect.y + 40, 11, textDim);
    
    int lvlWidth = MeasureText(std::to_string(iteminfo.GetLevel()).c_str(), 13);
    DrawText(std::to_string(iteminfo.GetLevel()).c_str(), mainRect.x + mainRect.width - lvlWidth - 14, mainRect.y + 12, 13, textDim);

    char costTxt[16];
    sprintf(costTxt, "$%d", iteminfo.price);
    int costWidth = MeasureText(costTxt, 22);
    DrawText(costTxt, mainRect.x + mainRect.width - costWidth - 14, mainRect.y + 44, 22, mainColor);
}

void drawshop(){
    DrawShopItem((Vector2){ 20, 50 }, engine.daemons[2]);
}