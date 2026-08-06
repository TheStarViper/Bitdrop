#pragma once
#include "raylib.h"
#include <stdbool.h>

typedef struct Timer {
    float duration;
    float elapsed;
    bool active;
    bool finished;
    bool started;
} Timer;

void TimerStart(Timer *timer, float duration);
void TimerUpdate(Timer *timer);
bool TimerJustFinished(Timer *timer);
float TimerProgress(Timer *timer);
void TimerStartOnce(Timer *timer, float duration);