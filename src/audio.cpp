#include "audio.hpp"
#include "raylib.h"
#include "variables.hpp"

void playsoundsmart(Sound sound, float volume,float pitch){
    SetSoundPitch(sound,pitch);
    SetSoundVolume(sound,volume*settings.masterVolume);
    PlaySound(sound);
}


void init_sounds(){
    TraceLog(LOG_INFO, "AUDIO: ZA BLUETOOTH DEWICE HAS BEEN CONNECTED");
    nodehitsound = LoadSound("assets/sfx/nodehit.ogg");
    hoversound = LoadSound("assets/sfx/hover.ogg");
    glitchloopsound = LoadSound("assets/sfx/glitchloop.ogg");
    transitionsound = LoadSound("assets/sfx/transition.ogg");
    sellsound = LoadSound("assets/sfx/sell.wav");

    //music
    mainmenumusic = LoadMusicStream("assets/music/mainmenumusic.wav");
    bgmusic = LoadMusicStream("assets/music/bgmusic.mp3");
}