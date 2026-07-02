#pragma once
#include "raylib.h"

namespace Config {
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;

    constexpr int TILE_SIZE = 75;                // Size of chessboard squares (75 * 8 = 600px)
    constexpr int BOARD_OFFSET_X = 210;          // Left buffer
    constexpr int BOARD_OFFSET_Y = 60;          // Top buffer

    constexpr int PANEL_X = BOARD_OFFSET_X+TILE_SIZE*8+50;                // Sidebar alignment x-coordinate
    constexpr int PANEL_Y = 60;                 // Sidebar alignment y-coordinate
    constexpr int PANEL_WIDTH = 380;            // Sidebar width
    constexpr int PANEL_HEIGHT = TILE_SIZE*8;           // Panel matches board footprint
    constexpr int ROW_HEIGHT = 35;              // Row spacing for historical lists

    constexpr float ANIMATION_DURATION = 0.12f;

    inline const int SIDEBAR_MAX_WIDTH = 160;
    inline const int SIDEBAR_MIN_WIDTH = 60;

    //settings
    inline bool showMoveHighlights = true;
    inline bool showBoardCoordinates = true;
    inline bool fiftymovecounter = false;
    inline bool threefoldcounter = false;
    inline bool highcontrast = false;
    inline bool boardmarkings = true;
}

namespace Gamestates{

}

namespace Resources{
    inline Sound hoversound;
    inline Sound winsound;
    inline Sound movesound;
}

struct smartbool { //from my other project
    enum State {
        False,
        NewTrue,
        True
    };

    State state = False;

    void operator=(bool value) {this->set(value);}
    operator bool() const {return state != False; }
    bool is_new_true() const { return state == NewTrue; }
    
    void set(bool value) {
        if (value) {
            if (state == False) state = NewTrue;
        } else {
            state = False;
        }
    }
    void update() {
        if (state == NewTrue) {
            state = True;
        }
    }
};