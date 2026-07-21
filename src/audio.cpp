#include "audio.hpp"
#include "raylib.h"

void playsoundsmart(Sound sound, float volume,float pitch){
    SetSoundPitch(sound,pitch);
    SetSoundVolume(sound,volume);
    PlaySound(sound);
}


void init_sounds(){
    TraceLog(LOG_INFO, "AUDIO: ZA BLUETOOTH DEWICE HAS BEEN CONNECTED");
    nodehitsound = LoadSound("assets/sfx/nodehit.ogg");
    hoversound = LoadSound("assets/sfx/hover.ogg");
    glitchloopsound = LoadSound("assets/sfx/glitchloop.ogg");
    transitionsound = LoadSound("assets/sfx/transition.ogg");
}