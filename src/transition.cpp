#include "variables.hpp"
#include "audio.hpp"
#include "transition.hpp"

void RequestGameStateChange(State newState) {
    StopSound(glitchloopsound);
    if (transition.phase != TRANS_NONE) return;
    transition.phase = TRANS_GLITCH_OUT;
    transition.timer = 0.0f;
    transition.pendingState = newState;
    float pitch = GetRandomValue(80,120)/100;
    playsoundsmart(transitionsound,.8,pitch);
}

float GetTransitionProgress() {
    if (transition.phase == TRANS_GLITCH_OUT) {
        return Clamp(transition.timer / transition.duration, 0.0f, 1.0f);
    }
    if (transition.phase == TRANS_GLITCH_IN) {
        return 1.0f - Clamp(transition.timer / transition.duration, 0.0f, 1.0f);
    }
    return 0.0f;
}

void UpdateTransition() {
    if (transition.phase == TRANS_NONE) return;

    transition.timer += GetFrameTime();

    if (transition.phase == TRANS_GLITCH_OUT) {
        if (transition.timer >= transition.duration) {
            gamestate.gamestate = transition.pendingState;
            transition.phase = TRANS_GLITCH_IN;
            transition.timer = 0.0f;
        }
    } else if (transition.phase == TRANS_GLITCH_IN) {
        if (transition.timer >= transition.duration) {
            transition.phase = TRANS_NONE;
            transition.timer = 0.0f;
        }
    }
}

void DrawGlitchedScene(RenderTexture2D target) {
    float intensity = GetTransitionProgress();

    if (intensity <= 0.0f) {
        DrawTextureRec(target.texture,
            (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height },
            (Vector2){ 0, 0 }, WHITE);
        return;
    }

    DrawTextureRec(target.texture,
        (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height },
        (Vector2){ 0, 0 }, Fade(WHITE, 1.0f - intensity * 0.3f));

    int aberration = (int)(intensity * 10.0f);
    if (aberration > 0) {
        DrawTextureRec(target.texture,
            (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height },
            (Vector2){ (float)aberration, 0 }, Fade(RED, 0.25f * intensity));
        DrawTextureRec(target.texture,
            (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height },
            (Vector2){ (float)-aberration, 0 }, Fade(SKYBLUE, 0.25f * intensity));
    }

    int sliceCount = (int)(intensity * 18.0f);
    for (int i = 0; i < sliceCount; i++) {
        int sliceY = GetRandomValue(0, target.texture.height - 8);
        int sliceHeight = GetRandomValue(3, 14);
        int xShift = GetRandomValue(-30, 30) * (int)(1.0f + intensity * 3.0f);

        Rectangle src = { 0, (float)sliceY, (float)target.texture.width, -(float)sliceHeight };
        Rectangle dst = { (float)xShift, (float)sliceY, (float)target.texture.width, (float)sliceHeight };
        DrawTexturePro(target.texture, src, dst, (Vector2){0,0}, 0.0f, WHITE);
    }

    int blockCount = (int)(intensity * 5.0f);
    for (int i = 0; i < blockCount; i++) {
        int bx = GetRandomValue(0, Config::SCREEN_WIDTH - 80);
        int by = GetRandomValue(0, Config::SCREEN_HEIGHT - 30);
        int bw = GetRandomValue(20, 120);
        int bh = GetRandomValue(4, 24);
        Color noiseCol = (GetRandomValue(0,1) == 0)
            ? (Color){ 0, 255, 120, 200 }
            : (Color){ 10, 10, 10, 220 };
        DrawRectangle(bx, by, bw, bh, Fade(noiseCol, intensity));
    }

    for (int y = 0; y < Config::SCREEN_HEIGHT; y += 3) {
        if (GetRandomValue(0, 100) < 20) {
            DrawRectangle(0, y, Config::SCREEN_WIDTH, 1, Fade(BLACK, 0.3f * intensity));
        }
    }
}

bool IsTransitioning() {
    return transition.phase != TRANS_NONE;
}