#pragma once
#include "raylib.h"

inline Sound nodehitsound;
inline Sound hoversound;
inline Sound glitchloopsound;
inline Sound transitionsound;
inline Sound sellsound;
void playsoundsmart(Sound sound, float volume=1,float pitch=1);
void init_sounds();
