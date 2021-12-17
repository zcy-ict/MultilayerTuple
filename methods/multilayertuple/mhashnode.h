#ifndef MHASHNODE_H
#define MHASHNODE_H

#include "../../elementary.h"
#include "mhashnode.h"
#include "multilayertuple.h"

using namespace std;

struct MRuleNode;
struct MHashNode;
struct MHashTable;
struct MTuple;
class MultilayerTuple;

struct MRuleNode {
	Rule *rule;
	uint32_t src_ip_begin, src_ip_end;
	uint32_t dst_ip_begin, dst_ip_end;
	uint16_t src_port_begin, src_port_end;
	uint16_t dst_port_begin, dst_port_end;
	uint8_t protocol_begin, protocol_end;
	int priority;
	MRuleNode *next;
	MRuleNode(Rule *_rule) {
		rule = _rule;
		src_ip_begin   = rule->range[0][0];
		src_ip_end     = rule->range[0][1];
		dst_ip_begin   = rule->range[1][0];
		dst_ip_end     = rule->range[1][1];
		src_port_begin = rule->range[2][0];
		src_port_end   = rule->range[2][1];
		dst_port_begin = rule->range[3][0];
		dst_port_end   = rule->range[3][1];
		protocol_begin = rule->range[4][0];
		protocol_end   = rule->range[4][1];
		priority       = rule->priority;
		next = NULL;
	}
    uint64_t MemorySize() {
		return sizeof(MRuleNode);
	}
	
	int Free(bool free_self) {
		if (free_self)
			free(this);
		return 0;
	}
};

struct MHashNode {
    MRuleNode *rule_node;
    MHashNode *next;

    uint32_t keys[5];
    uint32_t hash;
    uint32_t rules_num;
    int max_priority;

    bool has_next_multilayertuple;
    class MultilayerTuple *next_multilayertuple;

    MHashNode(uint32_t *_keys, uint32_t _hash);
    bool SameKey(uint32_t *_keys);
    int InsertRule(Rule *rule, uint32_t tuple_layer);
    int DeleteRule(Rule *rule, uint32_t tuple_layer);
    uint64_t MemorySize();
    int CalculateState(ProgramState *program_state);
    int GetRules(vector<Rule*> &rules);
    int Free(bool free_self);
    int Test(void *ptr);
};

#endif