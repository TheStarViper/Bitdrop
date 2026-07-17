#pragma once
#include "raylib.h"
#include "variables.hpp"

std::string FormatByteSize(long double bytes);
void RequestGameStateChange(State newState);
bool IsTransitioning(void);