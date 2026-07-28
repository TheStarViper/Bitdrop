#include "debug.hpp"
#include "raylib.h"
#include <string>

void debug_panel(){

}

void debug_overlay(){
    std::string pos = "X: " + std::to_string(GetMouseX()) + " Y: " + std::to_string(GetMouseY());
    DrawText(pos.c_str(), 20, 25, 10, RED);
}