#pragma once

#include "variables.hpp"
#include "raylib.h"

typedef enum {
    SYS_WORKSTATION = 0,
    SEC_FIREWALL_v2,
    RAW_PACKET_STREAM,
    BLACK_MARKET_NODE,
    COOLING_VENT_RESET,
    MAINFRAME_GATEWAY
} MapNodeType;


typedef struct MapNode {
    int id;
    int column;
    int row;
    MapNodeType type;
    MapNodeType revealedType; 
    Vector2 position;
    float alertState;      
    bool isEncrypted;
    bool isRevealed;       
    bool isDecrypting;
    float decryptTimer;     
    
    int connections[Config::maxmapconnections];
    int connectionCount;

    double targetquota;
    int reward;
} MapNode;

typedef struct {
    MapNode nodes[Config::maxmapcolumns][Config::maxnodespermapcolumn];
    int columnNodeCounts[Config::maxmapcolumns];
    
    int currentColumn;
    int currentNodeId;
    bool spoofActive;
    bool showEncrypted;
    
    float traceSlider;
    MapNode* selectedNode;
    
    int portScans;
    int ddosAttacks;
    int ipSpoofs;
    bool ddosTargetMode;
    
    Camera2D camera;
    float timeRunning;
} Mapstate;

void InitMap(void);
void DrawMap(void);