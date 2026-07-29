#pragma once
#include "variables.hpp"

enum TransitionPhase {
    TRANS_NONE,
    TRANS_GLITCH_OUT,
    TRANS_GLITCH_IN
};

struct TransitionState {
    TransitionPhase phase = TRANS_NONE;
    float timer = 0.0f;
    float duration = 0.16f;
    State pendingState;
};

typedef struct ContainedGlitch {
    bool active;
    Rectangle bounds;
    float timer;
    float duration;
} ContainedGlitch;

inline TransitionState transition;
void RequestGameStateChange(State newState);
float GetTransitionProgress();
void UpdateTransition();
void DrawGlitchedScene(RenderTexture2D target, Vector2 shake);
bool IsTransitioning();
void TriggerGlitchAt(Rectangle area, float duration);
void UpdateLocalGlitch();
void DrawGlitchArea(RenderTexture2D target, Vector2 shake);