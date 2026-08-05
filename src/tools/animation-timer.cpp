#include "raylib.h"
#include <stdbool.h>
#include "animation-timer.hpp"


void TimerStart(Timer *timer, float duration) {
    timer->duration = duration;
    timer->elapsed = 0.0f;
    timer->active = true;
    timer->finished = false;
}

void TimerUpdate(Timer *timer) {
    if (!timer->active) return;

    timer->elapsed += GetFrameTime();
    if (timer->elapsed >= timer->duration) {
        timer->elapsed = timer->duration;
        timer->active = false;
        timer->finished = true;
    }
}

bool TimerJustFinished(Timer *timer) {
    if (timer->finished) {
        timer->finished = false;
        return true;
    }
    return false;
}

float TimerProgress(Timer *timer) {
    if (timer->duration <= 0) return 1.0f;
    return timer->elapsed / timer->duration;
}