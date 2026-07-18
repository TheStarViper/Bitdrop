#include "raylib.h"
#include "raymath.h"
#include "map.hpp"
#include "variables.hpp"
#include <stdio.h>   
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "main.hpp"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

static Mapstate state;

int ClampInteger(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

MapNode* FindNodeById(int id) {
    if (id < 0) return NULL;
    for (int c = 0; c < Config::totalmapcolumns; c++) {
        for (int r = 0; r < state.columnNodeCounts[c]; r++) {
            if (state.nodes[c][r].id == id) {
                return &state.nodes[c][r];
            }
        }
    }
    return NULL;
}

Color GetNodeColor(MapNodeType type, float alert) {
    Color base = BLACK;
    switch (type) {
        case SYS_WORKSTATION:    base = (Color){ 0, 220, 255, 255 }; break; 
        case SEC_FIREWALL_v2:    base = (Color){ 255, 50, 50, 255 }; break;  
        case RAW_PACKET_STREAM:  base = (Color){ 200, 50, 250, 255 }; break; 
        case BLACK_MARKET_NODE:  base = (Color){ 255, 200, 0, 255 }; break;  
        case COOLING_VENT_RESET: base = (Color){ 50, 255, 150, 255 }; break; 
        case MAINFRAME_GATEWAY:  base = (Color){ 255, 0, 150, 255 }; break;  
    }
    
    if (alert > 0.1f) {
        float factor = alert;
        base.r = (unsigned char)(base.r * (1.0f - factor) + 255 * factor);
        base.g = (unsigned char)(base.g * (1.0f - factor) + 0 * factor);
        base.b = (unsigned char)(base.b * (1.0f - factor) + 50 * factor);
    }
    return base;
}

const char* GetNodeName(MapNodeType type) {
    switch (type) {
        case SYS_WORKSTATION:    return "SYS_WORKSTATION";
        case SEC_FIREWALL_v2:    return "SEC_FIREWALL_v2";
        case RAW_PACKET_STREAM:  return "RAW_PACKET_STREAM";
        case BLACK_MARKET_NODE:  return "BLACK_MARKET_NODE";
        case COOLING_VENT_RESET: return "COOLING_VENT_RESET";
        case MAINFRAME_GATEWAY:  return "MAINFRAME_GATEWAY";
    }
    return "UNKNOWN";
}

void SimulateNodeClear(MapNode* node, float tracePercentage) {
    if (!node) return;
    
    node->alertState += tracePercentage;
    if (node->alertState > 1.0f) node->alertState = 1.0f;
    
    float leak = tracePercentage * 0.5f;
    for (int i = 0; i < node->connectionCount; i++) {
        MapNode* target = FindNodeById(node->connections[i]);
        if (target) {
            target->alertState += leak;
            if (target->alertState > 1.0f) target->alertState = 1.0f;
        }
    }
}

bool IsNodeSelectable(MapNode* target) {
    if (state.currentNodeId == -1) {
        return (target->column == 0);
    }
    
    MapNode* current = FindNodeById(state.currentNodeId);
    if (!current) return false;
    
    if (target->isEncrypted && !state.showEncrypted) return false;
    
    if (state.spoofActive) {
        return (target->column == current->column && target->id != current->id);
    }
    
    for (int i = 0; i < current->connectionCount; i++) {
        if (current->connections[i] == target->id) return true;
    }
    
    return false;
}

void GenerateTopologyMap() {
    int uniqueId = 0;

    for (int c = 0; c < Config::totalmapcolumns; c++) {
        if (c == Config::totalmapcolumns - 1) {
            state.columnNodeCounts[c] = 1;
        } else {
            state.columnNodeCounts[c] = GetRandomValue(2, Config::maxnodespermapcolumn);
        }

        float colX = 100.0f + c * 220.0f;
        float totalHeight = 550.0f;
        float stepY = totalHeight / (state.columnNodeCounts[c] + 1);

        for (int r = 0; r < state.columnNodeCounts[c]; r++) {
            MapNode* n = &state.nodes[c][r];
            n->id = uniqueId++;
            n->column = c;
            n->row = r;
            n->alertState = 0.0f;
            n->connectionCount = 0;
            n->isRevealed = false;
            static float baseScore = 204800.0f;
            float exponentVariance = GetRandomValue(70, 130) / 100.0f;
            float randomizedExponent = (float)c * exponentVariance;
            n->targetquota = (int)(baseScore * powf(1.0f + Config::exponentialmapscoregrowth, randomizedExponent));
            n->reward = std::round((500.0f + GetRandomValue(0, 100)) + (std::pow((n->column) / 15.0f, 1.5f) * (2100.0f + GetRandomValue(-200, 200))));

            float jitterX = (float)GetRandomValue(-20, 20);
            float jitterY = (float)GetRandomValue(-20, 20);
            n->position = (Vector2){ colX + jitterX, 80.0f + (r + 1) * stepY + jitterY };

            if (c == Config::totalmapcolumns - 1) {
                n->type = MAINFRAME_GATEWAY;
            } else {
                int rng = GetRandomValue(1, 100);
                if (rng <= 45) {
                    n->type = (c > 7 && GetRandomValue(1, 2) == 1) ? SEC_FIREWALL_v2 : SYS_WORKSTATION;
                } else if (rng <= 65) {
                    n->type = SEC_FIREWALL_v2;
                } else if (rng <= 80) {
                    n->type = RAW_PACKET_STREAM;
                } else if (rng <= 90) {
                    n->type = BLACK_MARKET_NODE;
                } else {
                    n->type = COOLING_VENT_RESET;
                }
            }
            n->revealedType = n->type;
            n->isEncrypted = (c > 2 && c < 14 && GetRandomValue(1, 12) == 1);
        }
    }

    int numPaths = state.columnNodeCounts[0];
    std::vector<int> pathRow(numPaths);
    for (int p = 0; p < numPaths; p++) {
        pathRow[p] = p;
    }

    for (int c = 0; c < Config::totalmapcolumns - 1; c++) {
        int currCount = state.columnNodeCounts[c];
        int nextCount = state.columnNodeCounts[c + 1];

        for (int p = 0; p < numPaths; p++) {
            int fromRow = pathRow[p];
            MapNode* fromNode = &state.nodes[c][fromRow];

            int idealNext = (currCount > 0) ? (fromRow * nextCount) / currCount : 0;
            int delta = GetRandomValue(-1, 1);
            int nextRow = ClampInteger(idealNext + delta, 0, nextCount - 1);

            int targetId = state.nodes[c + 1][nextRow].id;

            bool alreadyConnected = false;
            for (int con = 0; con < fromNode->connectionCount; con++) {
                if (fromNode->connections[con] == targetId) { alreadyConnected = true; break; }
            }

            if (!alreadyConnected && fromNode->connectionCount < Config::maxmapconnections) {
                fromNode->connections[fromNode->connectionCount++] = targetId;
            }

            pathRow[p] = nextRow;
        }
    }

    for (int c = 0; c < Config::totalmapcolumns - 1; c++) {
        int nextCol = c + 1;
        int nextCount = state.columnNodeCounts[nextCol];
        int currCount = state.columnNodeCounts[c];

        for (int nc = 0; nc < nextCount; nc++) {
            int targetId = state.nodes[nextCol][nc].id;
            bool hasParent = false;

            for (int cr = 0; cr < currCount && !hasParent; cr++) {
                for (int con = 0; con < state.nodes[c][cr].connectionCount; con++) {
                    if (state.nodes[c][cr].connections[con] == targetId) { hasParent = true; break; }
                }
            }

            if (!hasParent) {
                int idealRow = ClampInteger((currCount > 0) ? (nc * currCount) / nextCount : 0, 0, currCount - 1);
                MapNode* chosenParent = nullptr;

                for (int offset = 0; offset < currCount && !chosenParent; offset++) {
                    int candidates[2] = { idealRow - offset, idealRow + offset };
                    for (int k = 0; k < 2; k++) {
                        int row = candidates[k];
                        if (row < 0 || row >= currCount) continue;
                        MapNode* candidate = &state.nodes[c][row];
                        if (candidate->connectionCount < Config::maxmapconnections) {
                            chosenParent = candidate;
                            break;
                        }
                    }
                }

                if (chosenParent) {
                    chosenParent->connections[chosenParent->connectionCount++] = targetId;
                } else {
                    MapNode* fallbackParent = &state.nodes[c][idealRow];
                    if (fallbackParent->connectionCount > 0) {
                        fallbackParent->connections[fallbackParent->connectionCount - 1] = targetId;
                    }
                }
            }
        }

        for (int r = 0; r < currCount; r++) {
            MapNode* currNode = &state.nodes[c][r];
            if (currNode->connectionCount == 0) {
                int idealNext = ClampInteger((currCount > 0) ? (r * nextCount) / currCount : 0, 0, nextCount - 1);
                currNode->connections[currNode->connectionCount++] = state.nodes[nextCol][idealNext].id;
            }
        }
    }
}

void InitMap() {
    state.currentColumn = -1;
    state.currentNodeId = -1;
    state.spoofActive = false; 
    state.showEncrypted = true;
    state.traceSlider = 0.40f;
    state.selectedNode = NULL;
    state.timeRunning = 0.0f;
    
    state.portScans = 1;
    state.ddosAttacks = 1;
    state.ipSpoofs = 1;
    state.ddosTargetMode = false;
    
    state.camera.target = (Vector2){ 0, 0 };
    state.camera.offset = (Vector2){ 0, 0 };
    state.camera.rotation = 0.0f;
    state.camera.zoom = 1.0f;
    
    GenerateTopologyMap();
}

void DrawMap(void) {
    state.timeRunning += GetFrameTime();
    Vector2 mousePos = GetMousePosition();
    Vector2 worldMousePos = GetScreenToWorld2D(mousePos, state.camera);

    if (!IsTransitioning() && (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)||IsMouseButtonDown(MOUSE_BUTTON_LEFT))&&mousePos.x<810) {
        Vector2 delta = GetMouseDelta();
        state.camera.target.x -= delta.x;
    }

    float maxMapWidth = 300.0f + (Config::totalmapcolumns - 1) * 220.0f;
    if (state.camera.target.x < 0) state.camera.target.x = 0;
    if (state.camera.target.x > maxMapWidth - Config::SCREEN_WIDTH + 400.0f) {
        state.camera.target.x = maxMapWidth - Config::SCREEN_WIDTH + 400.0f;
    }

    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0.0f) {
        state.camera.target.x -= wheelMove * Config::mapscrollspeed;
    }

    state.selectedNode = NULL;
    for (int c = 0; c < Config::totalmapcolumns; c++) {
        for (int r = 0; r < state.columnNodeCounts[c]; r++) {
            MapNode* n = &state.nodes[c][r];
            if (n->isEncrypted && !state.showEncrypted) continue;

            if (CheckCollisionPointCircle(worldMousePos, n->position, 22.0f)) {
                state.selectedNode = n;
                if (!IsTransitioning() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (state.ddosTargetMode) {
                        if (n->type == SEC_FIREWALL_v2) {
                            n->type = SYS_WORKSTATION;
                            n->revealedType = SYS_WORKSTATION;
                            state.ddosAttacks--;
                            state.ddosTargetMode = false;
                        }
                    }
                    else if (IsNodeSelectable(n)) {
                        state.currentNodeId = n->id;
                        state.currentColumn = n->column;
                        SimulateNodeClear(n, state.traceSlider);
                        state.spoofActive = false;
                        levelstate.TARGET_QUOTA_BYTES = n->targetquota;
                        levelstate.reward = n->reward;
                        RequestGameStateChange(GAME);
                        return;
                    }
                }
            }
        }
    }

    if (!IsTransitioning() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !state.selectedNode) {
        state.ddosTargetMode = false;
    }

    //background grid
    int offsetX = (int)state.camera.target.x % 40;
    for (int i = 0; i < Config::SCREEN_WIDTH; i += 40) {
        DrawLine(i - offsetX, 0, i - offsetX, Config::SCREEN_HEIGHT, (Color){ 0, 40, 10, 255 });
    }
    for (int i = 0; i < Config::SCREEN_HEIGHT; i += 40) {
        DrawLine(0, i, Config::SCREEN_WIDTH, i, (Color){ 0, 40, 10, 255 });
    }

    BeginMode2D(state.camera);

    //hover path BFS
    std::vector<int> hoverPath;
    std::unordered_map<int,int> pathIndex;

    if (state.selectedNode && state.currentNodeId != -1 && state.selectedNode->id != state.currentNodeId) {
        std::unordered_map<int,int> cameFrom;
        std::unordered_set<int> visited;
        std::queue<int> q;

        q.push(state.currentNodeId);
        visited.insert(state.currentNodeId);
        bool found = false;

        while (!q.empty() && !found) {
            int curId = q.front(); q.pop();
            MapNode* curNode = FindNodeById(curId);
            if (!curNode) continue;

            for (int i = 0; i < curNode->connectionCount; i++) {
                int nextId = curNode->connections[i];
                MapNode* nextNode = FindNodeById(nextId);
                if (!nextNode) continue;
                if (nextNode->isEncrypted && !state.showEncrypted) continue;
                if (visited.count(nextId)) continue;

                visited.insert(nextId);
                cameFrom[nextId] = curId;

                if (nextId == state.selectedNode->id) { found = true; break; }
                q.push(nextId);
            }
        }

        if (found) {
            int cur = state.selectedNode->id;
            hoverPath.push_back(cur);
            while (cur != state.currentNodeId) {
                cur = cameFrom[cur];
                hoverPath.push_back(cur);
            }
            std::reverse(hoverPath.begin(), hoverPath.end());
            for (size_t i = 0; i < hoverPath.size(); i++) pathIndex[hoverPath[i]] = (int)i;
        }
    }

    //connection lines (single pass, not duplicated)
    for (int c = 0; c < Config::totalmapcolumns; c++) {
        for (int r = 0; r < state.columnNodeCounts[c]; r++) {
            MapNode* n = &state.nodes[c][r];

            for (int i = 0; i < n->connectionCount; i++) {
                MapNode* target = FindNodeById(n->connections[i]);
                if (!target) continue;
                if (target->isEncrypted && !state.showEncrypted) continue;

                Color lineCol = (Color){ 0, 80, 20, 150 };
                float thickness = 1.5f;

                bool nodeIsCurrent = (state.currentNodeId == n->id);
                bool canSelectTarget = IsNodeSelectable(target);

                auto itN = pathIndex.find(n->id);
                auto itT = pathIndex.find(target->id);
                bool isOnHoverPath = (itN != pathIndex.end() && itT != pathIndex.end()
                                    && itT->second == itN->second + 1);

                if (isOnHoverPath) {
                    float alphaPulse = (sinf(state.timeRunning * 9.0f) * 0.3f) + 0.7f;
                    lineCol = (Color){ 255, 230, 0, (unsigned char)(255 * alphaPulse) };
                    thickness = 3.0f;
                } else if (nodeIsCurrent && canSelectTarget) {
                    float alphaPulse = (sinf(state.timeRunning * 7.0f) * 0.3f) + 0.7f;
                    lineCol = (Color){ 0, 255, 100, (unsigned char)(255 * alphaPulse) };
                    thickness = 3.0f;
                } else if (n->column < state.currentColumn || (n->column == state.currentColumn && n->id != state.currentNodeId)) {
                    lineCol = (Color){ 0, 60, 20, 100 };
                }

                DrawLineEx(n->position, target->position, thickness, lineCol);
            }
        }
    }

    // column highlight bands (single pass, not nested per-node)
    for (int c = 0; c < Config::totalmapcolumns; c++) {
        bool columnHasSelectable = false;
        for (int r = 0; r < state.columnNodeCounts[c]; r++) {
            MapNode* n = &state.nodes[c][r];
            if (n->isEncrypted && !state.showEncrypted) continue;
            if (IsNodeSelectable(n)) { columnHasSelectable = true; break; }
        }

        if (columnHasSelectable) {
            float colX = 100.0f + c * 220.0f;
            float cameraTopY = state.camera.target.y - (state.camera.offset.y / state.camera.zoom);
            float cameraBottomY = cameraTopY + (Config::SCREEN_HEIGHT / state.camera.zoom);
            float visibleHeight = cameraBottomY - cameraTopY;

            DrawRectangle(colX - 50, cameraTopY, 100, visibleHeight, (Color){51, 249, 47, 50});
        }
    }

    //nodes
    for (int c = 0; c < Config::totalmapcolumns; c++) {
        for (int r = 0; r < state.columnNodeCounts[c]; r++) {
            MapNode* n = &state.nodes[c][r];
            if (n->isEncrypted && !state.showEncrypted) continue;

            float radius = (n->type == MAINFRAME_GATEWAY) ? 28.0f : 16.0f;
            float pulse = 1.0f;

            if (n->alertState > 0.05f) {
                pulse += sinf(state.timeRunning * 12.0f) * (n->alertState * 0.25f);
            } else if (n->type == MAINFRAME_GATEWAY) {
                pulse += sinf(state.timeRunning * 3.0f) * 0.1f;
            }

            Color coreColor = GetNodeColor(n->type, n->alertState);

            if (n->type == RAW_PACKET_STREAM && !n->isRevealed) {
                coreColor = (Color){ 130, 130, 130, 255 };
            }

            if (state.currentNodeId == n->id) {
                DrawCircleV(n->position, radius * pulse + 5.0f, WHITE);
            }

            DrawCircleV(n->position, radius * pulse, coreColor);
            DrawCircleLinesV(n->position, radius * pulse, (Color){ 255, 255, 255, 180 });

            const char* symb = "N";
            if (n->type == MAINFRAME_GATEWAY) symb = "BOSS";
            else if (n->type == SYS_WORKSTATION) symb = "SYS";
            else if (n->type == SEC_FIREWALL_v2) symb = "FW";
            else if (n->type == BLACK_MARKET_NODE) symb = "MKT";
            else if (n->type == COOLING_VENT_RESET) symb = "VENT";
            else if (n->type == RAW_PACKET_STREAM) symb = n->isRevealed ? "PKT" : "?";

            int fontSize = (n->type == MAINFRAME_GATEWAY) ? 12 : 10;
            int textW = MeasureText(symb, fontSize);

            DrawText(symb, (int)n->position.x - textW / 2, (int)n->position.y - fontSize / 2, fontSize, BLACK);

            if (n->isEncrypted) {
                DrawText("[ENC]", (int)n->position.x, (int)n->position.y - (int)radius - 14, 9, MAGENTA);
            }
        }
    }

    EndMode2D();

    //UI chrome
    DrawRectangle(0, 0, Config::SCREEN_WIDTH, 50, (Color){ 5, 20, 10, 230 });
    DrawLine(0, 50, Config::SCREEN_WIDTH, 50, (Color){ 0, 255, 80, 255 });
    DrawText("HACKING ITINUARY", 20, 15, 18, (Color){ 0, 255, 100, 255 });

    char statusBuf[128];
    if (state.currentNodeId == -1) {
        strcpy(statusBuf, "STATUS: SELECT ENTRY NODE");
    } else {
        snprintf(statusBuf, sizeof(statusBuf), "NODE LOCATION: ID %02d | COLUMN: %d/%d", state.currentNodeId, state.currentColumn, Config::totalmapcolumns - 1);
    }
    DrawText(statusBuf, 550, 18, 14, (Color){ 0, 200, 255, 255 });

    int panX = 800;
    int panY = 50;
    int panW = 480;
    int panH = Config::SCREEN_HEIGHT - 50;

    DrawRectangle(panX, panY, panW, panH, (Color){ 10, 15, 12, 245 });
    DrawLine(panX, panY, panX, Config::SCREEN_HEIGHT, (Color){ 0, 255, 80, 255 });

    if (state.selectedNode && GetWorldToScreen2D(state.selectedNode->position, state.camera).x < 800) {
        MapNode* sn = state.selectedNode;
        Vector2 screenPos = GetWorldToScreen2D(sn->position, state.camera);

        char name_string[64];
        snprintf(name_string, sizeof(name_string), "%s", GetNodeName(sn->type));
        std::string reward_string = "REWARD: $" + std::to_string(sn->reward);
        std::string targetquota_string = "QUOTA: " + FormatByteSize(sn->targetquota);

        int nameoffsetx = MeasureText(name_string, 11);
        int rewardoffsetx = MeasureText(reward_string.c_str(), 11);
        int quotaoffsetx = MeasureText(targetquota_string.c_str(), 11);

        int max_width = nameoffsetx;
        if (quotaoffsetx > max_width) max_width = quotaoffsetx;
        if (rewardoffsetx > max_width) max_width = rewardoffsetx;

        int padding = 10;
        int bg_width = max_width + padding * 2;
        int bg_height = 56;

        int bg_x = screenPos.x - bg_width / 2;
        int bg_y = screenPos.y - 86;

        DrawRectangle(bg_x, bg_y, bg_width, bg_height, Fade(BLACK, 0.7f));
        DrawRectangleLines(bg_x, bg_y, bg_width, bg_height, Fade(GREEN, 0.5f));

        DrawText(name_string, screenPos.x - nameoffsetx / 2, bg_y + 6, 11, GetNodeColor(sn->type, 0.0f));
        DrawText(targetquota_string.c_str(), screenPos.x - quotaoffsetx / 2, bg_y + 22, 11, GREEN);
        DrawText(reward_string.c_str(), screenPos.x - rewardoffsetx / 2, bg_y + 38, 11, GREEN);

        float minoffset = (sn->type == MAINFRAME_GATEWAY) ? 28.0f : 16.0f;
        float maxoffset = (sn->type == MAINFRAME_GATEWAY) ? 30.0f : 18.0f;
        float offset = GetPulseOffset(minoffset, maxoffset, 10.0f, Easings::EaseInOutQuad);
        Color pulseColor = Fade(GREEN, 255);

        float bracketSize = 6.0f;
        float thickness = 2.0f;
        Vector2 center = screenPos;

        DrawLineEx({center.x - offset, center.y - offset}, {center.x - offset + bracketSize, center.y - offset}, thickness, pulseColor);
        DrawLineEx({center.x - offset, center.y - offset}, {center.x - offset, center.y - offset + bracketSize}, thickness, pulseColor);
        DrawLineEx({center.x + offset, center.y - offset}, {center.x + offset - bracketSize, center.y - offset}, thickness, pulseColor);
        DrawLineEx({center.x + offset, center.y - offset}, {center.x + offset, center.y - offset + bracketSize}, thickness, pulseColor);
        DrawLineEx({center.x - offset, center.y + offset}, {center.x - offset + bracketSize, center.y + offset}, thickness, pulseColor);
        DrawLineEx({center.x - offset, center.y + offset}, {center.x - offset, center.y + offset - bracketSize}, thickness, pulseColor);
        DrawLineEx({center.x + offset, center.y + offset}, {center.x + offset - bracketSize, center.y + offset}, thickness, pulseColor);
        DrawLineEx({center.x + offset, center.y + offset}, {center.x + offset, center.y + offset - bracketSize}, thickness, pulseColor);
    }

    DrawRectangle(0, Config::SCREEN_HEIGHT - 35, Config::SCREEN_WIDTH - panW, 35, (Color){ 2, 10, 5, 240 });
    DrawText("CONTROLS: LEFT-CLICK to select/hack | RIGHT-MOUSE DRAG to scroll graph map", 15, Config::SCREEN_HEIGHT - 24, 12, (Color){ 0, 180, 70, 255 });
}