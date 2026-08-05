#include "nodeicons.hpp"


void BuildNodeIcon(NodeIconShape shape, IconGrid& grid) {
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            grid[y][x] = false;
        }
    }

    float cx = 7.5f, cy = 7.5f;

    switch (shape) { // this is ai generated as idk how to do the pixelart with math
        case NodeIconShape::EYE: {
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    float dx = (x - cx) / 7.0f;
                    float dy = (y - cy) / 4.0f;
                    float dist = dx * dx + dy * dy;
                    if (dist <= 1.0f && dist >= 0.75f) grid[y][x] = true;
                }
            }
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    float dx = x - cx, dy = y - cy;
                    if (dx * dx + dy * dy <= 4.0f) grid[y][x] = true;
                }
            }
            break;
        }
        case NodeIconShape::SHIELD: {
            for (int y = 0; y < 16; y++) {
                float halfWidth = 7.0f * (1.0f - (y / 15.0f) * 0.5f);
                if (y > 11) halfWidth *= (16 - y) / 4.0f;
                for (int x = 0; x < 16; x++) {
                    float dx = fabsf(x - cx);
                    bool border = (dx > halfWidth - 1.5f && dx <= halfWidth);
                    bool topOrBottom = (y <= 1) || (y >= 14);
                    if ((border || topOrBottom) && dx <= halfWidth) grid[y][x] = true;
                }
            }
            break;
        }
        case NodeIconShape::PACKET: {
            for (int y = 2; y < 14; y++) {
                for (int x = 2; x < 14; x++) {
                    bool border = (y == 2 || y == 13 || x == 2 || x == 13);
                    bool diagonal = (abs((x - 2) - (y - 2)) <= 1);
                    if (border || diagonal) grid[y][x] = true;
                }
            }
            break;
        }
        case NodeIconShape::MARKET: {
            for (int y = 1; y < 15; y++) {
                for (int x = 1; x < 15; x++) {
                    float dx = x - cx, dy = y - cy;
                    float d2 = dx * dx + dy * dy;
                    if (d2 <= 42.0f && d2 >= 30.0f) grid[y][x] = true;
                }
            }
            for (int y = 4; y < 12; y++) grid[y][(int)cx] = true;
            for (int x = 5; x < 11; x++) grid[5][x] = true;
            for (int x = 5; x < 11; x++) grid[10][x] = true;
            break;
        }
        case NodeIconShape::VENT: {
            for (int i = -7; i <= 7; i++) {
                int x1 = (int)(cx + i);
                int y1 = (int)(cy + i * 0.6f);
                int x2 = (int)(cx - i);
                int y2 = (int)(cy + i * 0.6f);
                if (x1 >= 0 && x1 < 16 && y1 >= 0 && y1 < 16) grid[y1][x1] = true;
                if (x2 >= 0 && x2 < 16 && y2 >= 0 && y2 < 16) grid[y2][x2] = true;
            }
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    float dx = x - cx, dy = y - cy;
                    float dist = sqrtf(dx * dx + dy * dy);
                    if (dist >= 6.5f && dist <= 7.5f) grid[y][x] = true;
                }
            }
            break;
        }
        case NodeIconShape::GATEWAY: {
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    float dx = x - cx, dy = y - cy;
                    float dist = sqrtf(dx * dx + dy * dy);
                    if ((dist >= 6.0f && dist <= 7.5f) || (dist >= 2.5f && dist <= 3.5f)) grid[y][x] = true;
                }
            }
            break;
        }
        case NodeIconShape::UNKNOWN: {
            for (int y = 3; y < 6; y++) {
                for (int x = 5; x < 11; x++) {
                    float dx = x - 7.5f, dy = y - 4.0f;
                    float d2 = dx * dx + dy * dy;
                    if (d2 <= 6.0f && d2 >= 3.0f) grid[y][x] = true;
                }
            }
            grid[7][8] = true; grid[8][7] = true; grid[8][8] = true;
            grid[11][7] = true; grid[12][7] = true;
            break;
        }
    }
}

const IconGrid& GetNodeIconGrid(MapNodeType type, bool revealed) {
    static IconGrid eyeIcon;
    static IconGrid shieldIcon;
    static IconGrid packetIcon;
    static IconGrid marketIcon;
    static IconGrid ventIcon;
    static IconGrid gatewayIcon;
    static IconGrid unknownIcon;
    static bool initialized = false;

    if (!initialized) {
        BuildNodeIcon(NodeIconShape::EYE, eyeIcon);
        BuildNodeIcon(NodeIconShape::SHIELD, shieldIcon);
        BuildNodeIcon(NodeIconShape::PACKET, packetIcon);
        BuildNodeIcon(NodeIconShape::MARKET, marketIcon);
        BuildNodeIcon(NodeIconShape::VENT, ventIcon);
        BuildNodeIcon(NodeIconShape::GATEWAY, gatewayIcon);
        BuildNodeIcon(NodeIconShape::UNKNOWN, unknownIcon);
        initialized = true;
    }

    switch (type) {
        case SYS_WORKSTATION:    return eyeIcon;
        case SEC_FIREWALL_v2:    return shieldIcon;
        case RAW_PACKET_STREAM:  return revealed ? packetIcon : unknownIcon;
        case BLACK_MARKET_NODE:  return marketIcon;
        case COOLING_VENT_RESET: return ventIcon;
        case MAINFRAME_GATEWAY:  return gatewayIcon;
    }
    return unknownIcon;
}