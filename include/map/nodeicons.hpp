#pragma once
#include "shop.hpp"
#include "map.hpp"

enum class NodeIconShape { EYE, SHIELD, PACKET, MARKET, VENT, GATEWAY, UNKNOWN };

void BuildNodeIcon(NodeIconShape shape, IconGrid& grid);
const IconGrid& GetNodeIconGrid(MapNodeType type, bool revealed);