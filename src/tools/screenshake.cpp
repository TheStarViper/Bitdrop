#include "screenshake.hpp"
#include <cmath>
#include "variables.hpp"

namespace {
    float shakeDuration = 0.0f;
    float shakeTimer = 0.0f;
    float shakeIntensity = 0.0f;
    float shakeSeed = 0.0f;
}

void screenshake(float intensity, float duration) {
    if (settings.screenShakeEnabled){
        if (shakeTimer <= 0.0f || intensity >= shakeIntensity) {
            shakeIntensity = intensity;
            shakeDuration = duration;
            shakeTimer = duration;
        }
    }
}

void UpdateScreenShake(float dt) {
    if (shakeTimer > 0.0f) {
        shakeTimer -= dt;
        if (shakeTimer < 0.0f) shakeTimer = 0.0f;
    }
    shakeSeed += dt * 60.0f;
}

Vector2 GetScreenShakeOffset() {
    if (shakeTimer <= 0.0f || shakeDuration <= 0.0f) {
        return { 0.0f, 0.0f };
    }
    float progress = shakeTimer / shakeDuration;
    float falloff = progress * progress;
    float offsetX = sinf(shakeSeed * 1.7f) * shakeIntensity * falloff;
    float offsetY = sinf(shakeSeed * 2.3f + 1.5f) * shakeIntensity * falloff;
    return { offsetX, offsetY };
}
