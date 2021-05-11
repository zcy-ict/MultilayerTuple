#ifndef  PSIO_H
#define  PSIO_H

#include "../../elementary.h"
#include "ElementaryClasses.h"

using namespace std;

vector<PSRule> GeneratePSRules(vector<Rule*> &rules);
void PrintPSRules(vector<PSRule> &rules);
vector<PSPacket> GeneratePSPackets(vector<Trace*> &traces);
#endif