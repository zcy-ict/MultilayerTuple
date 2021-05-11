#ifndef  SIMULATION_H
#define  SIMULATION_H

#include "../../elementary.h"
#include "ElementaryClasses.h"
// #include "Utilities/MapExtensions.h"

#include <map>
#include <unordered_map>

using namespace std;


vector<int> PerformOnlyPacketClassification(PacketClassifier& classifier, vector<PSRule> &rules, vector<PSPacket> &packets);

#endif