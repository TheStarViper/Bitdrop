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

inline TransitionState transition;
void RequestGameStateChange(State newState);
float GetTransitionProgress();
void UpdateTransition();
void DrawGlitchedScene(RenderTexture2D target);
bool IsTransitioning();