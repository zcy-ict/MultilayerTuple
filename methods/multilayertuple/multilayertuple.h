#ifndef  MULTILAYERTUPLE_H
#define  MULTILAYERTUPLE_H

#include "../../elementary.h"
#include "mtuple.h"

using namespace std;

struct MTuple;

class MultilayerTuple : public Classifier {
public:
    
    int Create(vector<Rule*> &rules, bool insert);

    int InsertRule(Rule *rule);
    int DeleteRule(Rule *rule);
    int Lookup(Trace *trace, int priority);
    int LookupAccess(Trace *trace, int priority, Rule *ans_rule, ProgramState *program_state);

    int Reconstruct();
    uint64_t MemorySize();
    int CalculateState(ProgramState *program_state);
    int GetRules(vector<Rule*> &rules);
    int Free(bool free_self);
    int Test(void *ptr);

    int Init(uint32_t _tuple_layer, bool _start_tuple_layer);
    void InsertTuple(MTuple *tuple);
    void SortTuples();
    uint32_t GetReducedPrefix(uint32_t *prefix_len, Rule *rule);

    bool start_tuple_layer;
    uint32_t tuple_layer;

    MTuple **tuples_arr;
    map<uint32_t, MTuple*> tuples_map;
    int tuples_num;
    int max_tuples_num;
    int rules_num;
    int max_priority;
};

#endif