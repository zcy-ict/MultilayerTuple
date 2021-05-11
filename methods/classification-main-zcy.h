#ifndef  CLASSIFICATIONMAINZCY_H
#define  CLASSIFICATIONMAINZCY_H

#include "../elementary.h"
#include "../io/io.h"
#include "../methods/multilayertuple/multilayertuple.h"

using namespace std;

int ClassificationMainZcy(CommandStruct command, ProgramState *program_state, vector<Rule*> &rules, 
                          vector<Trace*> &traces, vector<int> &ans);

#endif