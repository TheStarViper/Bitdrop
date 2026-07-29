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

void DrawGlitchedScene(RenderTexture2D target, Vector2 shake) {
    float intensity = GetTransitionProgress();

    if (intensity <= 0.0f) {
        DrawTextureRec(target.texture,
            (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height },
            (Vector2){ shake.x, shake.y }, WHITE);
        return;
    }

    DrawTextureRec(target.texture,
        (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height },
        (Vector2){ shake.x, shake.y }, Fade(WHITE, 1.0f - intensity * 0.3f));

    int aberration = (int)(intensity * 10.0f);
    if (aberration > 0) {
        DrawTextureRec(target.texture,
            (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height },
            (Vector2){ shake.x + (float)aberration, shake.y }, Fade(RED, 0.25f * intensity));
        DrawTextureRec(target.texture,
            (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height },
            (Vector2){ shake.x + (float)-aberration, shake.y }, Fade(SKYBLUE, 0.25f * intensity));
    }

    int sliceCount = (int)(intensity * 18.0f);
    for (int i = 0; i < sliceCount; i++) {
        int sliceY = GetRandomValue(0, target.texture.height - 8);
        int sliceHeight = GetRandomValue(3, 14);
        int xShift = GetRandomValue(-30, 30) * (int)(1.0f + intensity * 3.0f);

        Rectangle src = { 0, (float)sliceY, (float)target.texture.width, -(float)sliceHeight };
        Rectangle dst = { shake.x + (float)xShift, shake.y + (float)sliceY, (float)target.texture.width, (float)sliceHeight };
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
        DrawRectangle(bx + (int)shake.x, by + (int)shake.y, bw, bh, Fade(noiseCol, intensity));
    }

    for (int y = 0; y < Config::SCREEN_HEIGHT; y += 3) {
        if (GetRandomValue(0, 100) < 20) {
            DrawRectangle((int)shake.x, y + (int)shake.y, Config::SCREEN_WIDTH, 1, Fade(BLACK, 0.3f * intensity));
        }
    }
}

bool IsTransitioning() {
    return transition.phase != TRANS_NONE;
}

ContainedGlitch localGlitch = { 0 };

void TriggerGlitchAt(Rectangle area, float duration) {
    localGlitch.bounds = area;
    localGlitch.duration = (duration > 0.0f) ? duration : 0.1f;
    localGlitch.timer = 0.0f;
    localGlitch.active = true;
}

void UpdateLocalGlitch() {
    if (!localGlitch.active) return;

    localGlitch.timer += GetFrameTime();
    if (localGlitch.timer >= localGlitch.duration) {
        localGlitch.active = false;
        localGlitch.timer = 0.0f;
    }
}

void DrawGlitchArea(RenderTexture2D target, Vector2 shake) {
    if (!localGlitch.active) return;

    float progress = localGlitch.timer / localGlitch.duration;
    float intensity = 1.0f - Clamp(progress, 0.0f, 1.0f);

    Rectangle rect = localGlitch.bounds;
    float texH = (float)target.texture.height;

    BeginScissorMode((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height);

    Rectangle srcRect = { rect.x, texH - rect.y - rect.height, rect.width, -rect.height };

    DrawTextureRec(target.texture, srcRect,
        (Vector2){ rect.x + shake.x, rect.y + shake.y }, 
        Fade(WHITE, 1.0f - intensity * 0.3f));

    int aberration = (int)(intensity * 10.0f);
    if (aberration > 0) {
        DrawTextureRec(target.texture, srcRect,
            (Vector2){ rect.x + shake.x + (float)aberration, rect.y + shake.y }, 
            Fade(RED, 0.25f * intensity));

        DrawTextureRec(target.texture, srcRect,
            (Vector2){ rect.x + shake.x - (float)aberration, rect.y + shake.y }, 
            Fade(SKYBLUE, 0.25f * intensity));
    }

    int sliceCount = (int)(intensity * 12.0f);
    for (int i = 0; i < sliceCount; i++) {
        int sliceY = GetRandomValue((int)rect.y, (int)(rect.y + rect.height - 4));
        int sliceHeight = GetRandomValue(2, 8);
        int xShift = GetRandomValue(-15, 15) * (int)(1.0f + intensity * 2.0f);

        Rectangle sliceSrc = { 
            rect.x, 
            texH - (float)sliceY - (float)sliceHeight, 
            rect.width, 
            -(float)sliceHeight 
        };
        Rectangle sliceDst = { 
            rect.x + shake.x + (float)xShift, 
            (float)sliceY + shake.y, 
            rect.width, 
            (float)sliceHeight 
        };

        DrawTexturePro(target.texture, sliceSrc, sliceDst, (Vector2){0,0}, 0.0f, WHITE);
    }

    int blockCount = (int)(intensity * 4.0f);
    for (int i = 0; i < blockCount; i++) {
        int bw = GetRandomValue(10, (int)rect.width / 2);
        int bh = GetRandomValue(4, 12);
        int bx = GetRandomValue((int)rect.x, (int)(rect.x + rect.width - bw));
        int by = GetRandomValue((int)rect.y, (int)(rect.y + rect.height - bh));

        Color noiseCol = (GetRandomValue(0, 1) == 0)
            ? (Color){ 0, 255, 120, 200 }
            : (Color){ 10, 10, 10, 220 };

        DrawRectangle(bx + (int)shake.x, by + (int)shake.y, bw, bh, Fade(noiseCol, intensity));
    }

    for (int y = (int)rect.y; y < (int)(rect.y + rect.height); y += 3) {
        if (GetRandomValue(0, 100) < 20) {
            DrawRectangle((int)rect.x + (int)shake.x, y + (int)shake.y, (int)rect.width, 1, Fade(BLACK, 0.3f * intensity));
        }
    }

    EndScissorMode();
}