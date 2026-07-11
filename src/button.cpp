#include "button.hpp"
#include "raylib.h"



bool DrawButton(Rectangle rect, ButtonType type, unsigned char alpha, Color bgNormal, Color bgHover, Color borderCol, Color contentCol, const char* text, int fontSize) {
    Vector2 mouse = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mouse, rect);
    
    Color bg = isHovered ? bgHover : bgNormal;
    bg.a = alpha;
    DrawRectangleRec(rect, bg);
    DrawRectangleLinesEx(rect, 1.0f, borderCol);

    if (type == ButtonType::ArrowUp) {
        Vector2 v1 = { rect.x + rect.width / 2.0f, rect.y + 5.0f };
        Vector2 v2 = { rect.x + 8.0f,             rect.y + rect.height - 5.0f };
        Vector2 v3 = { rect.x + rect.width - 8.0f, rect.y + rect.height - 5.0f };
        DrawTriangle(v1, v2, v3, contentCol);
    } 
    else if (type == ButtonType::ArrowDown) {
        Vector2 v1 = { rect.x + 8.0f,             rect.y + 5.0f };
        Vector2 v2 = { rect.x + rect.width / 2.0f, rect.y + rect.height - 5.0f };
        Vector2 v3 = { rect.x + rect.width - 8.0f, rect.y + 5.0f };
        DrawTriangle(v1, v2, v3, contentCol);
    } 
    else if (type == ButtonType::TextGeneric && text != nullptr) {
        int textWidth = MeasureText(text, fontSize);
        float textX = rect.x + (rect.width - textWidth) / 2.0f;
        float textY = rect.y + (rect.height - fontSize) / 2.0f;
        
        DrawText(text, textX, textY, fontSize, contentCol);
    }

    return isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}