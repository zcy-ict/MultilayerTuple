#ifndef MHASHTABLE_H
#define MHASHTABLE_H

#include "../../elementary.h"
#include "mhashnode.h"

#define MHASHTABLEMAX 0.85
#define MHASHTABLEMIN 0.2

using namespace std;


struct MHashNode;

struct MHashTable {
    uint32_t tuple_layer;

    uint32_t hash_node_num;
    uint32_t max_hash_node_num;
    uint32_t min_hash_node_num;
    uint32_t mask;  //sizeof MHashNode - 1
    MHashNode **hash_node_arr;
    int max_priority;

    int Init(int size, uint32_t _tuple_layer);
    int InsertHashNode(MHashNode *hash_node);
    MHashNode* PickHashNode(uint32_t *keys, uint32_t hash);
    int HashTableResize(uint32_t size);

    int InsertRule(Rule *rule, uint32_t *keys, uint32_t hash);
    int DeleteRule(Rule *rule, uint32_t *keys, uint32_t hash);
    uint64_t MemorySize();
    int CalculateState(ProgramState *program_state);
    int GetRules(vector<Rule*> &rules);
    int Free(bool free_self);
    int Test(void *ptr);
};

struct MTuple {
    uint32_t tuple_layer;
    uint32_t prefix_len[5];
    uint32_t prefix_len_zero[5];

    int max_priority;
    int rules_num;

    MHashTable hash_table;

    MTuple(uint32_t _tuple_layer, uint32_t *_prefix_len);
    int InsertRule(Rule *rule);
    int DeleteRule(Rule *rule);
    uint64_t MemorySize();
    int CalculateState(ProgramState *program_state);
    int GetRules(vector<Rule*> &rules);
    int Free(bool free_self);
    int Test(void *ptr);
};


#endif