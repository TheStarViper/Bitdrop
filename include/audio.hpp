#pragma once
#include "raylib.h"

inline Sound nodehitsound;
inline Sound hoversound;
inline Sound glitchloopsound;
inline Sound transitionsound;
inline Sound sellsound;


inline Music mainmenumusic;
inline Music bgmusic;
void playsoundsmart(Sound sound, float volume=1,float pitch=1);
void init_sounds();
