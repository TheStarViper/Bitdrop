#include "main-menu.hpp"
#include "raylib.h"
#include "variables.hpp"
#include "button.hpp"
#include "main.hpp"
#include "transition.hpp"
#include "formatting.hpp"
#include "esc-menu.hpp"
void DrawGlowText(const std::string& text, Vector2 pos, int fontSize, Color color) {
    Font font = GetFontDefault();
    int textW = MeasureText(text.c_str(), fontSize);

    for (int i = 3; i >= 1; i--) {
        Color glow = Fade(color, 0.10f * i);
        DrawText(text.c_str(), pos.x - i, pos.y, fontSize, glow);
        DrawText(text.c_str(), pos.x + i, pos.y, fontSize, glow);
        DrawText(text.c_str(), pos.x, pos.y - i, fontSize, glow);
        DrawText(text.c_str(), pos.x, pos.y + i, fontSize, glow);
    }
    DrawText(text.c_str(), pos.x, pos.y, fontSize, color);
}

void DrawMenuPanel(Rectangle r, float cut, Color fill, Color border, float thickness) {
    Vector2 pts[6] = {
        { r.x, r.y },
        { r.x + r.width - cut, r.y },
        { r.x + r.width, r.y + cut },
        { r.x + r.width, r.y + r.height },
        { r.x + cut, r.y + r.height },
        { r.x, r.y + r.height - cut }
    };
    DrawTriangleFan(pts, 6, fill);
    for (int i = 0; i < 6; i++) {
        DrawLineEx(pts[i], pts[(i + 1) % 6], thickness, border);
    }
}
struct MenuDataMote {
    Vector2 position;
    float speed;
    float alpha;
    Color color;
};

void drawmainmenu() {
    ClearBackground(Config::COLOR_BG);

    int offsetY = ((int)(GetTime() * 6.0f)) % 40;
    for (int x = 0; x < Config::SCREEN_WIDTH; x += 40) {
        DrawLine(x, 0, x, Config::SCREEN_HEIGHT, Fade(Config::COLOR_GRID_LINE, 0.4f));
    }
    for (int y = -40; y < Config::SCREEN_HEIGHT; y += 40) {
        DrawLine(0, y + offsetY, Config::SCREEN_WIDTH, y + offsetY, Fade(Config::COLOR_GRID_LINE, 0.25f));
    }

    static std::vector<MenuDataMote> motes;
    if (motes.empty()) {
        for (int i = 0; i < 40; i++) {
            motes.push_back({
                { (float)GetRandomValue(0, Config::SCREEN_WIDTH), (float)GetRandomValue(0, Config::SCREEN_HEIGHT) },
                (float)GetRandomValue(15, 45),
                (float)GetRandomValue(20, 70) / 100.0f,
                (GetRandomValue(0, 1) == 0) ? Config::COLOR_NODE : Config::COLOR_PROBE
            });
        }
    }
    for (auto& mote : motes) {
        mote.position.y += mote.speed * GetFrameTime();
        if (mote.position.y > Config::SCREEN_HEIGHT) {
            mote.position.y = -10.0f;
            mote.position.x = (float)GetRandomValue(0, Config::SCREEN_WIDTH);
        }
        DrawCircleV(mote.position, 1.5f, Fade(mote.color, mote.alpha));
    }

    Vector2 emblemCenter = { Config::SCREEN_WIDTH / 2.0f, 220.0f };
    float emblemPulse = (sinf((float)GetTime() * 0.8f) * 0.5f + 0.5f);
    float emblemRadius = 150.0f + emblemPulse * 10.0f;

    DrawCircleLines(emblemCenter.x, emblemCenter.y, emblemRadius, Fade(Config::COLOR_UI_GREEN, 0.12f));
    DrawCircleLines(emblemCenter.x, emblemCenter.y, emblemRadius * 0.7f, Fade(Config::COLOR_UI_GREEN, 0.08f));

    float scanAngle = fmodf((float)GetTime() * 20.0f, 360.0f);
    for (int seg = 0; seg < 3; seg++) {
        float segStart = scanAngle + seg * 120.0f;
        DrawRing(emblemCenter, emblemRadius + 10.0f, emblemRadius + 13.0f, segStart, segStart + 30.0f, 12, Fade(Config::COLOR_UI_GREEN, 0.5f));
    }

    for (int t = 0; t < 4; t++) {
        float tickAngle = (45.0f + t * 90.0f) * DEG2RAD;
        Vector2 dir = { cosf(tickAngle), sinf(tickAngle) };
        Vector2 start = { emblemCenter.x + dir.x * (emblemRadius + 20.0f), emblemCenter.y + dir.y * (emblemRadius + 20.0f) };
        Vector2 end = { emblemCenter.x + dir.x * (emblemRadius + 30.0f), emblemCenter.y + dir.y * (emblemRadius + 30.0f) };
        DrawLineEx(start, end, 1.5f, Fade(Config::COLOR_UI_GREEN, 0.3f));
    }

    static float titleGlitchTimer = 0.0f;
    titleGlitchTimer -= GetFrameTime();
    float glitchOffsetX = 0.0f;
    if (titleGlitchTimer <= 0.0f && GetRandomValue(0, 300) < 2) {
        titleGlitchTimer = 0.08f;
    }
    if (titleGlitchTimer > 0.0f) {
        glitchOffsetX = (float)GetRandomValue(-4, 4);
    }

    std::string title = "BITDROP";
    int titleFontSize = 88;
    int titleW = MeasureText(title.c_str(), titleFontSize);
    Vector2 titlePos = { Config::SCREEN_WIDTH / 2.0f - titleW / 2.0f + glitchOffsetX, 175.0f };
    DrawGlowText(title, titlePos, titleFontSize, Config::COLOR_UI_GREEN);
    if (titleGlitchTimer > 0.0f) {
        DrawText(title.c_str(), titlePos.x + 3, titlePos.y, titleFontSize, Fade((Color){ 255, 50, 140, 255 }, 0.4f));
    }

    float panelW = 320.0f;
    float panelX = Config::SCREEN_WIDTH / 2.0f - panelW / 2.0f;
    float panelY = 340.0f;
    float buttonH = 48.0f;
    float buttonGap = 12.0f;
    int buttonCount = 4;
    float panelH = 30.0f + buttonCount * (buttonH + buttonGap);

    DrawMenuPanel({ panelX, panelY, panelW, panelH }, 16.0f, Fade(Color{ 10, 16, 26, 240 }, 0.9f), Config::COLOR_SHARD_BORDER, 1.0f);
    DrawLineEx({ panelX + 10, panelY }, { panelX + 30, panelY }, 2.0f, Config::COLOR_UI_GREEN);
    DrawLineEx({ panelX, panelY + 10 }, { panelX, panelY + 30 }, 2.0f, Config::COLOR_UI_GREEN);
    DrawLineEx({ panelX + panelW, panelY + panelH - 10 }, { panelX + panelW - 20, panelY + panelH }, 2.0f, Config::COLOR_UI_GREEN);

    float btnX = panelX + 20.0f;
    float btnW = panelW - 40.0f;
    float btnY = panelY + 20.0f;

    Rectangle newRunBtn = { btnX, btnY, btnW, buttonH };
    if (DrawButton(newRunBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, Config::COLOR_UI_GREEN, WHITE, "INITIATE RUN", 18)) {
        RequestGameStateChange(MAP);
    }
    btnY += buttonH + buttonGap;

    Rectangle statsBtn = { btnX, btnY, btnW, buttonH };
    if (DrawButton(statsBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, Config::COLOR_UI_AMBER, WHITE, "STATS", 16)) {
        esc_menu_state = STATS;
        esc_menu = true;
    }
    btnY += buttonH + buttonGap;

    Rectangle settingsBtn = { btnX, btnY, btnW, buttonH };
    if (DrawButton(settingsBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, Config::COLOR_UI_AMBER, WHITE, "SETTINGS", 16)) {
        esc_menu_state = SETTINGZ;
        esc_menu = true;
    }
    btnY += buttonH + buttonGap;

    Rectangle quitBtn = { btnX, btnY, btnW, buttonH };
    if (!esc_menu&&DrawButton(quitBtn, ButtonType::TextGeneric, 255, Config::colorButtonBg, Config::COLOR_GRID_LINE, (Color){ 255, 70, 70, 255 }, WHITE, "TERMINATE", 16)) {
        CloseWindow();
    }

    std::string statusLine = "CHOOSE YOUR FIGHTER // jk lmao";
    int statusW = MeasureText(statusLine.c_str(), 11);
    float statusAlpha = (sinf((float)GetTime() * 3.0f) * 0.3f + 0.7f);
    DrawText(statusLine.c_str(), Config::SCREEN_WIDTH / 2.0f - statusW / 2.0f, panelY + panelH + 20.0f, 11, Fade(Config::COLOR_UI_GREEN, statusAlpha));
}