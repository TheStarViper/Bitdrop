#include "raylib.h"
#include "raymath.h"
#include "map.hpp"
#include "variables.hpp"
#include <stdio.h>   
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>



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

void GenerateTopologyMap(void) {
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
            n->position = (Vector2){ colX, 80.0f + (r + 1) * stepY };
            
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
    
    for (int c = 0; c < Config::totalmapcolumns - 1; c++) {
        int nextCol = c + 1;
        int nextCount = state.columnNodeCounts[nextCol];
        int currCount = state.columnNodeCounts[c];
        
        for (int r = 0; r < currCount; r++) {
            MapNode* currNode = &state.nodes[c][r];
            int baseTarget = (r * nextCount) / currCount;
            
            int rangeStart = ClampInteger(baseTarget - 1, 0, nextCount - 1);
            int rangeEnd = ClampInteger(baseTarget + 1, 0, nextCount - 1);
            
            for (int t = rangeStart; t <= rangeEnd; t++) {
                if (currNode->connectionCount < Config::maxmapconnections) {
                    currNode->connections[currNode->connectionCount++] = state.nodes[nextCol][t].id;
                }
            }
        }
        
        for (int nc = 0; nc < nextCount; nc++) {
            int targetId = state.nodes[nextCol][nc].id;
            bool hasParent = false;
            
            for (int cr = 0; cr < currCount; cr++) {
                for (int con = 0; con < state.nodes[c][cr].connectionCount; con++) {
                    if (state.nodes[c][cr].connections[con] == targetId) {
                        hasParent = true;
                        break;
                    }
                }
                if (hasParent) break;
            }
            
            if (!hasParent) {
                int randomParentRow = GetRandomValue(0, currCount - 1);
                MapNode* parentNode = &state.nodes[c][randomParentRow];
                if (parentNode->connectionCount < Config::maxmapconnections) {
                    parentNode->connections[parentNode->connectionCount++] = targetId;
                }
            }
        }
    }
}

void InitGame(void) {
    state.currentColumn = -1;
    state.currentNodeId = -1;
    state.spoofActive = false;
    state.showEncrypted = false;
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
    
    float scrollSpeed = 650.0f * GetFrameTime();
    if (IsKeyDown(KEY_RIGHT)) state.camera.target.x += scrollSpeed;
    if (IsKeyDown(KEY_LEFT))  state.camera.target.x -= scrollSpeed;
    
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();
        state.camera.target.x -= delta.x;
    }
    
    float maxMapWidth = 100.0f + (Config::totalmapcolumns - 1) * 220.0f;
    if (state.camera.target.x < 0) state.camera.target.x = 0;
    if (state.camera.target.x > maxMapWidth - Config::SCREEN_WIDTH + 200.0f) {
        state.camera.target.x = maxMapWidth - Config::SCREEN_WIDTH + 200.0f;
    }
    
    state.selectedNode = NULL;
    for (int c = 0; c < Config::totalmapcolumns; c++) {
        for (int r = 0; r < state.columnNodeCounts[c]; r++) {
            MapNode* n = &state.nodes[c][r];
            if (n->isEncrypted && !state.showEncrypted) continue;
            
            if (CheckCollisionPointCircle(worldMousePos, n->position, 22.0f)) {
                state.selectedNode = n;
                
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
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
                    }
                }
            }
        }
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !state.selectedNode) {
        state.ddosTargetMode = false;
    }
    
    BeginDrawing();
    ClearBackground(BLACK);
    
    int offsetX = (int)state.camera.target.x % 40;
    for (int i = 0; i < Config::SCREEN_WIDTH; i += 40) {
        DrawLine(i - offsetX, 0, i - offsetX, Config::SCREEN_HEIGHT, (Color){ 0, 40, 10, 255 });
    }
    for (int i = 0; i < Config::SCREEN_HEIGHT; i += 40) {
        DrawLine(0, i, Config::SCREEN_WIDTH, i, (Color){ 0, 40, 10, 255 });
    }
    
    BeginMode2D(state.camera);
    
    for (int c = 0; c < Config::totalmapcolumns; c++) {
        for (int r = 0; r < state.columnNodeCounts[c]; r++) {
            MapNode* n = &state.nodes[c][r];
            if (n->isEncrypted && !state.showEncrypted) continue;
            
            for (int i = 0; i < n->connectionCount; i++) {
                MapNode* target = FindNodeById(n->connections[i]);
                if (!target) continue;
                if (target->isEncrypted && !state.showEncrypted) continue;
                
                Color lineCol = (Color){ 0, 80, 20, 150 }; 
                float thickness = 1.5f;
                
                bool nodeIsCurrent = (state.currentNodeId == n->id);
                bool canSelectTarget = IsNodeSelectable(target);
                
                if (nodeIsCurrent && canSelectTarget) {
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
            bool selectState = IsNodeSelectable(n);
            
            if (n->type == RAW_PACKET_STREAM && !n->isRevealed) {
                coreColor = (Color){ 130, 130, 130, 255 }; 
            }
            
            if (selectState) {
                float selectGlow = (sinf(state.timeRunning * 8.0f) * 0.4f) + 0.6f;
                DrawCircleV(n->position, radius * pulse + 8.0f, (Color){ 0, 255, 120, (unsigned char)(100 * selectGlow) });
            }
            
if (state.currentNodeId == n->id) {
                DrawCircleV(n->position, radius * pulse + 5.0f, WHITE); 
            }
            
            DrawCircleV(n->position, radius * pulse, coreColor);
            
            // FIXED AUDIT: Correct signature is (Vector2 center, float radius, Color color)
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
                DrawText("[ENC]", (int)n->position.x - 15, (int)n->position.y - (int)radius - 14, 9, MAGENTA);
            }
        }
    }
    
    EndMode2D();
    
    DrawRectangle(0, 0, Config::SCREEN_WIDTH, 50, (Color){ 5, 20, 10, 230 });
    DrawLine(0, 50, Config::SCREEN_WIDTH, 50, (Color){ 0, 255, 80, 255 });
    
    DrawText("TRACEROUTE TOPOLOGY NETWORK SCHEMATIC v4.92", 20, 15, 18, (Color){ 0, 255, 100, 255 });
    
    char statusBuf[128];
    if (state.currentNodeId == -1) {
        strcpy(statusBuf, "STATUS: PENDING INITIAL INTRUSION (SELECT ENTRY COLUMN NODE)");
    } else {
        snprintf(statusBuf, sizeof(statusBuf), "NODE LOCATION: ID %02d | COLUMN: %d/%d", state.currentNodeId, state.currentColumn, Config::totalmapcolumns - 1);
    }
    DrawText(statusBuf, 750, 18, 14, (Color){ 0, 200, 255, 255 });
    
    int panX = Config::SCREEN_WIDTH - 300;
    int panY = 50;
    int panW = 300;
    int panH = Config::SCREEN_HEIGHT - 50;
    
    DrawRectangle(panX, panY, panW, panH, (Color){ 10, 15, 12, 245 });
    DrawLine(panX, panY, panX, Config::SCREEN_HEIGHT, (Color){ 0, 255, 80, 255 });
    
    int uiY = panY + 20;
    
    DrawText("SYSTEM DATA DECRYPTION", panX + 20, uiY, 14, WHITE);
    uiY += 25;
    
    Rectangle btnDecrypt = (Rectangle){ (float)panX + 20.0f, (float)uiY, 260.0f, 30.0f };
    bool hoverDecrypt = CheckCollisionPointRec(mousePos, btnDecrypt);
    DrawRectangleRec(btnDecrypt, hoverDecrypt ? (Color){ 40, 40, 50, 255 } : (Color){ 20, 22, 25, 255 });
    
    DrawRectangleLinesEx(btnDecrypt, 1.0f, state.showEncrypted ? MAGENTA : (Color){ 100, 100, 100, 255 });
    
    DrawText(state.showEncrypted ? "DISCONNECT NET_DECRYPT" : "RUN DECRYPT_NETWORK.exe", panX + 40, uiY + 8, 12, state.showEncrypted ? MAGENTA : LIGHTGRAY);
    
    if (hoverDecrypt && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        state.showEncrypted = !state.showEncrypted;
    }
    uiY += 45;
    
    DrawText("SIMULATED NODE CLEARANCE PRESETS", panX + 20, uiY, 13, WHITE);
    uiY += 22;
    
    Rectangle sliderBar = (Rectangle){ (float)panX + 20.0f, (float)uiY + 5.0f, 260.0f, 8.0f };
    DrawRectangleRec(sliderBar, (Color){ 50, 50, 50, 255 });
    int handleX = (int)(sliderBar.x + (state.traceSlider * sliderBar.width));
    DrawRectangle(handleX - 6, uiY, 12, 18, (Color){ 0, 255, 100, 255 });
    
    char sliderStr[32];
    snprintf(sliderStr, sizeof(sliderStr), "On-Clear Trace Spike: %d%%", (int)(state.traceSlider * 100));
    DrawText(sliderStr, panX + 20, uiY + 25, 11, (Color){ 180, 180, 180, 255 });
    
    if (CheckCollisionPointRec(mousePos, (Rectangle){ sliderBar.x, sliderBar.y - 10, sliderBar.width, 28 })) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            state.traceSlider = (mousePos.x - sliderBar.x) / sliderBar.width;
            if (state.traceSlider < 0.0f) state.traceSlider = 0.0f;
            if (state.traceSlider > 1.0f) state.traceSlider = 1.0f;
        }
    }
    uiY += 55;
    
    DrawText("NET_RUNNER PING EXECUTABLES", panX + 20, uiY, 13, WHITE);
    uiY += 25;
    
    // Tool 1
    Rectangle rTool1 = (Rectangle){ (float)panX + 20.0f, (float)uiY, 260.0f, 35.0f };
    bool hTool1 = CheckCollisionPointRec(mousePos, rTool1);
    DrawRectangleRec(rTool1, hTool1 ? (Color){ 30, 50, 40, 255 } : (Color){ 15, 25, 20, 255 });
    DrawRectangleLinesEx(rTool1, 1.0f, (state.portScans > 0) ? (Color){ 0, 200, 80, 255 } : RED);
    char bufT1[64]; snprintf(bufT1, sizeof(bufT1), "Port_Scan.sh  [Qty: %d]", state.portScans);
    DrawText(bufT1, panX + 32, uiY + 6, 12, WHITE);
    DrawText("Expose exact layout identities of all '?' nodes", panX + 32, uiY + 20, 9, DARKGRAY);
    if (hTool1 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && state.portScans > 0) {
        state.portScans--;
        for (int c = 0; c < Config::totalmapcolumns; c++) {
            for (int r = 0; r < state.columnNodeCounts[c]; r++) {
                state.nodes[c][r].isRevealed = true;
            }
        }
    }
    uiY += 48;
    
    // Tool 2
    Rectangle rTool2 = (Rectangle){ (float)panX + 20.0f, (float)uiY, 260.0f, 35.0f };
    bool hTool2 = CheckCollisionPointRec(mousePos, rTool2);
    Color ddosBorder = state.ddosTargetMode ? RED : ((state.ddosAttacks > 0) ? (Color){ 0, 200, 80, 255 } : RED);
    DrawRectangleRec(rTool2, hTool2 ? (Color){ 30, 50, 40, 255 } : (Color){ 15, 25, 20, 255 });
    DrawRectangleLinesEx(rTool2, 1.0f, ddosBorder);
    char bufT2[64]; snprintf(bufT2, sizeof(bufT2), "DDoS_Attack.bat [Qty: %d]", state.ddosAttacks);
    DrawText(state.ddosTargetMode ? "CLICK TARGET FIREWALL..." : bufT2, panX + 32, uiY + 6, 12, state.ddosTargetMode ? RED : WHITE);
    DrawText("Downgrade an Elite Firewall into a Workstation", panX + 32, uiY + 20, 9, DARKGRAY);
    if (hTool2 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && state.ddosAttacks > 0) {
        state.ddosTargetMode = !state.ddosTargetMode;
    }
    uiY += 48;
    
    // Tool 3
    Rectangle rTool3 = (Rectangle){ (float)panX + 20.0f, (float)uiY, 260.0f, 35.0f };
    bool hTool3 = CheckCollisionPointRec(mousePos, rTool3);
    DrawRectangleRec(rTool3, hTool3 ? (Color){ 30, 50, 40, 255 } : (Color){ 15, 25, 20, 255 });
    DrawRectangleLinesEx(rTool3, 1.0f, state.spoofActive ? GOLD : ((state.ipSpoofs > 0) ? (Color){ 0, 200, 80, 255 } : RED));
    char bufT3[64]; snprintf(bufT3, sizeof(bufT3), "IP_Spoof.cfg   [Qty: %d]", state.ipSpoofs);
    DrawText(state.spoofActive ? "SPOOF LAYER ACTIVE" : bufT3, panX + 32, uiY + 6, 12, state.spoofActive ? GOLD : WHITE);
    DrawText("Bypass limits: jump laterally within same column", panX + 32, uiY + 20, 9, DARKGRAY);
    if (hTool3 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && state.ipSpoofs > 0 && state.currentNodeId != -1) {
        state.spoofActive = !state.spoofActive;
        if (state.spoofActive) state.ipSpoofs--;
        else state.ipSpoofs++; 
    }
    uiY += 60;
    
    DrawText("NODE INSPECTION LOGS", panX + 20, uiY, 13, WHITE);
    uiY += 20;
    Rectangle inspectionBox = (Rectangle){ (float)panX + 15.0f, (float)uiY, 270.0f, 130.0f };
    DrawRectangleRec(inspectionBox, (Color){ 5, 8, 6, 255 });
    DrawRectangleLinesEx(inspectionBox, 1.0f, (Color){ 0, 120, 40, 255 });
    
    if (state.selectedNode) {
        MapNode* sn = state.selectedNode;
        char nameLine[64]; snprintf(nameLine, sizeof(nameLine), "NAME: %s", GetNodeName(sn->type));
        char colLine[64];  snprintf(colLine, sizeof(colLine),   "GRID: Col %d, Row %d", sn->column, sn->row);
        char alertLine[64];snprintf(alertLine, sizeof(alertLine), "TRACE METRIC: %.1f%%", sn->alertState * 100.0f);
        char encLine[64];  snprintf(encLine, sizeof(encLine),   "SECURITY ENCRYPT: %s", sn->isEncrypted ? "TRUE" : "FALSE");
        
        DrawText(nameLine, panX + 25, uiY + 15, 12, GetNodeColor(sn->type, 0.0f));
        DrawText(colLine, panX + 25, uiY + 40, 11, LIGHTGRAY);
        DrawText(alertLine, panX + 25, uiY + 65, 11, (sn->alertState > 0.5f) ? RED : ORANGE);
        DrawText(encLine, panX + 25, uiY + 90, 11, sn->isEncrypted ? MAGENTA : GREEN);
    } else {
        DrawText("NO ACTIVE TARGET HOVERED\n\nSYSTEM IDLE...", panX + 25, uiY + 45, 12, (Color){ 0, 100, 30, 255 });
    }
    
    DrawRectangle(0, Config::SCREEN_HEIGHT - 35, Config::SCREEN_WIDTH - panW, 35, (Color){ 2, 10, 5, 240 });
    DrawText("CONTROLS: LEFT-CLICK to select/hack | RIGHT-MOUSE DRAG or ARROW KEYS to scroll graph map", 15, Config::SCREEN_HEIGHT - 24, 12, (Color){ 0, 180, 70, 255 });
    
    EndDrawing();
}
