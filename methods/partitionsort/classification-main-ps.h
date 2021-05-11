#ifndef  CLASSIFICATIONMAINPS_H
#define  CLASSIFICATIONMAINPS_H

#include "../../elementary.h"
#include "../../io/io.h"
#include "ps-io.h"
#include "OVS/TupleSpaceSearch.h"
#include "PartitionSort/PartitionSort.h"
#include "TupleMerge/TupleMergeOnline.h"

using namespace std;

int ClassificationMainPS(CommandStruct command, ProgramState *program_state, vector<Rule*> &rules, 
                          vector<Trace*> &traces, vector<int> &ans);

#endif