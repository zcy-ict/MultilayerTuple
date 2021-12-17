#include "mtuple.h"

using namespace std;

extern int max_prefix_len[5];
extern int prefix_dims_num;

MTuple::MTuple(uint32_t _tuple_layer, uint32_t *_prefix_len) {
	tuple_layer = _tuple_layer;
	for (int i = 0; i < prefix_dims_num; ++i) {
		prefix_len[i] = _prefix_len[i];
		prefix_len_zero[i] = max_prefix_len[i] - prefix_len[i];
	}
	hash_table.Init(32, tuple_layer);
	max_priority = 0;
	rules_num = 0;

	// printf("new MTuple: dims %d , ", prefix_dims_num);
	// for (int i = 0; i < prefix_dims_num; ++i)
	// 	printf("%d ", prefix_len[i]);
	// printf("\n");
}

int MTuple::InsertRule(Rule *rule) {
	uint32_t keys[5];
	for (int i = 0; i < prefix_dims_num; ++i)
		keys[i] = (uint64_t)rule->range[i][0] >> prefix_len_zero[i];
	uint32_t hash = HashKeys(keys, prefix_dims_num);
	// printf("hash %08x\n", hash);

	if (hash_table.InsertRule(rule, keys, hash) > 0)
        return 1;
    ++rules_num;
    max_priority = hash_table.max_priority;

	return 0;
}

int MTuple::DeleteRule(Rule *rule) {
	uint32_t keys[5];
	for (int i = 0; i < prefix_dims_num; ++i)
		keys[i] = (uint64_t)rule->range[i][0] >> prefix_len_zero[i];
	uint32_t hash = HashKeys(keys, prefix_dims_num);

	if (hash_table.DeleteRule(rule, keys, hash) > 0)
        return 1;
    --rules_num;
    max_priority = hash_table.max_priority;
	return 0;
}

uint64_t MTuple::MemorySize() {
    uint64_t memory_size = sizeof(MTuple);
    memory_size += hash_table.MemorySize() - sizeof(MHashTable);
    return memory_size;
}

int MTuple::CalculateState(ProgramState *program_state) {
	hash_table.CalculateState(program_state);
	return 0;
}

int MTuple::GetRules(vector<Rule*> &rules) {
    hash_table.GetRules(rules);
	return 0;
}

int MTuple::Free(bool free_self) {
    hash_table.Free(false);
    if (free_self)
        free(this);
	return 0;
}

int MTuple::Test(void *ptr) {
	return 0;
}