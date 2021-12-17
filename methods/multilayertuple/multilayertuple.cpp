#include "multilayertuple.h"

using namespace std;

// To implement MultilayerTuple in other scene, please revise these parameters.
int max_prefix_len[5] = {32, 32, 16, 16, 8};
int prefix_dims_num;  // to be 2 or 5
int max_layers_num = 5;  // the max_prelix_len 32 = 2^5
uint32_t create_next_layer_rules_num = 20;
uint32_t delete_next_layer_rules_num = create_next_layer_rules_num / 2;


int MultilayerTuple::Init(uint32_t _tuple_layer, bool _start_tuple_layer) {
    tuple_layer = _tuple_layer;
    start_tuple_layer = _start_tuple_layer;
	return 0;
}

int MultilayerTuple::Create(vector<Rule*> &rules, bool insert) {

    pthread_mutex_init(&lookup_mutex, NULL);
    pthread_mutex_init(&update_mutex, NULL);

    if (tuple_layer == 0 || prefix_dims_num == 0) {
    	printf("Wrong : MultilayerTuple tuple_layer %d prefix_dims_num %d\n", 
                tuple_layer, prefix_dims_num);
    	exit(1);
    }

    tuples_num = 0;
    max_tuples_num = 16;
    tuples_arr = (MTuple**)malloc(sizeof(MTuple*) * max_tuples_num);
    for (int i = 0; i < max_tuples_num; ++i)
        tuples_arr[i] = NULL;
    tuples_map.clear();

    rules_num = 0;
    max_priority = 0;

    // printf("Create end. tuple_layer %d prefix_dims_num %d\n", tuple_layer, prefix_dims_num);

    if (insert) {
        int rules_num = rules.size();
        for (int i = 0; i < rules_num; ++i)
            InsertRule(rules[i]);
    }

	return 0;
}

void MultilayerTuple::InsertTuple(MTuple *tuple) {
    if (tuples_num == max_tuples_num) {
		MTuple **new_tuples_arr = (MTuple**)malloc(sizeof(MTuple*) * max_tuples_num * 2);
		for (int i = 0; i < max_tuples_num; ++i)
			new_tuples_arr[i] = tuples_arr[i];
        for (int i = max_tuples_num; i < max_tuples_num * 2; ++i)
            new_tuples_arr[i] = NULL;
        max_tuples_num *= 2;
        free(tuples_arr);
        tuples_arr = new_tuples_arr;
    }
	tuples_arr[tuples_num++] = tuple;
}

bool CmpMTuple(MTuple *tuple1, MTuple *tuple2) {
	return tuple1->max_priority > tuple2->max_priority;
}

void MultilayerTuple::SortTuples() {
    sort(tuples_arr, tuples_arr + tuples_num, CmpMTuple);
}

uint32_t MultilayerTuple::GetReducedPrefix(uint32_t *prefix_len, Rule *rule) {
    uint32_t prefix_pair = 0;
    for (int i = 0; i < prefix_dims_num; ++i) {
        int step = max(1, max_prefix_len[i] >> tuple_layer);
        prefix_len[i] = rule->prefix_len[i] - rule->prefix_len[i] % step;
        prefix_pair = prefix_pair << 6 | prefix_len[i];
    }
    return prefix_pair;
}

int MultilayerTuple::InsertRule(Rule *rule) {
	uint32_t prefix_len[5];
	uint32_t prefix_pair = GetReducedPrefix(prefix_len, rule);
	map<uint32_t, MTuple*>::iterator iter = tuples_map.find(prefix_pair);
    MTuple *tuple = NULL;
    if (iter != tuples_map.end()) {
        tuple = iter->second;
    } else {
        tuple = new MTuple(tuple_layer, prefix_len);
        InsertTuple(tuple);
        tuples_map[prefix_pair] = tuple;
    }
    if (tuple->InsertRule(rule) > 0)
        return 1;
    
    ++rules_num;
    if (rule->priority == tuple->max_priority)
        SortTuples();
    max_priority = max(max_priority, rule->priority);
    return 0;
}

int MultilayerTuple::DeleteRule(Rule *rule) {
    uint32_t prefix_len[5];
    uint32_t prefix_pair = GetReducedPrefix(prefix_len, rule);
    map<uint32_t, MTuple*>::iterator iter = tuples_map.find(prefix_pair);
    MTuple *tuple = NULL;
    if (iter != tuples_map.end()) {
        tuple = iter->second;
    } else {
        printf("Wrong: DeleteRule has no tuple\n");
        return 1;
    }

    if (tuple->DeleteRule(rule) > 0)
        return 1;

    if (tuple->rules_num == 0) {
        //printf("delete tuple\n");
        tuple->max_priority = 0;
        SortTuples();
        tuples_arr[--tuples_num] = NULL;
        tuples_map.erase(prefix_pair);
        tuple->Free(true);
    } else if (rule->priority >= tuple->max_priority) {
        SortTuples();
    }

    --rules_num;
    if (rules_num == 0)
        max_priority = 0;
    else
        max_priority = tuples_arr[0]->max_priority;

	return 0;
}

int MultilayerTuple::Lookup(Trace *trace, int priority) {
    uint32_t keys[5];
    for (int i = 0; i < tuples_num; ++i) {
        MTuple *tuple = tuples_arr[i];
        if (priority >= tuple->max_priority)
            break;
        for (int i = 0; i < prefix_dims_num; ++i)
            keys[i] = (uint64_t)trace->key[i] >> tuple->prefix_len_zero[i];
        uint32_t hash = HashKeys(keys, prefix_dims_num);

        MHashNode *hash_node = tuple->hash_table.hash_node_arr[hash & tuple->hash_table.mask];
        while (hash_node) {
            if (priority >= hash_node->max_priority)
                break;
            if (hash_node->SameKey(keys)) {
                if (hash_node->has_next_multilayertuple) {
                    priority = hash_node->next_multilayertuple->Lookup(trace, priority);
                } else {
                    MRuleNode *rule_node = hash_node->rule_node;
                    while (rule_node) {
                        if (priority >= rule_node->priority)
                            break;
                        if (rule_node->src_ip_begin <= trace->key[0] && trace->key[0] <= rule_node->src_ip_end &&
                            rule_node->dst_ip_begin <= trace->key[1] && trace->key[1] <= rule_node->dst_ip_end &&
                            rule_node->src_port_begin <= trace->key[2] && trace->key[2] <= rule_node->src_port_end &&
                            rule_node->dst_port_begin <= trace->key[3] && trace->key[3] <= rule_node->dst_port_end &&
                            rule_node->protocol_begin <= trace->key[4] && trace->key[4] <= rule_node->protocol_end) {
                            priority = rule_node->priority;
                            break;
                        }
                        rule_node = rule_node->next;  
                    };
                }
                break;
            }
            hash_node = hash_node->next;
        }
    }
    // printf("lookup layer %d priority %d\n", tuple_layer, priority);
	return priority;
}

int MultilayerTuple::LookupAccess(Trace *trace, int priority, Rule *ans_rule, ProgramState *program_state) { 
    if (start_tuple_layer) {
        program_state->access_tuples.ClearNum();
        program_state->access_tables.ClearNum();
        program_state->access_nodes.ClearNum();
        program_state->access_rules.ClearNum();
    }
    uint32_t keys[5];
    for (int i = 0; i < tuples_num; ++i) {
        MTuple *tuple = tuples_arr[i];
        if (priority >= tuple->max_priority)
            break;
        program_state->access_tuples.AddNum();
        program_state->access_tables.AddNum();
        for (int i = 0; i < prefix_dims_num; ++i)
            keys[i] = (uint64_t)trace->key[i] >> tuple->prefix_len_zero[i];
        uint32_t hash = HashKeys(keys, prefix_dims_num);

        MHashNode *hash_node = tuple->hash_table.hash_node_arr[hash & tuple->hash_table.mask];
        while (hash_node) {
            if (priority >= hash_node->max_priority)
                break;
            program_state->access_nodes.AddNum();
            if (hash_node->SameKey(keys)) {
                if (hash_node->has_next_multilayertuple) {
                    priority = hash_node->next_multilayertuple->LookupAccess(trace, priority, ans_rule, program_state);
                } else {
                    MRuleNode *rule_node = hash_node->rule_node;
                    while (rule_node) {
                        if (priority >= rule_node->priority)
                            break;
                        program_state->access_rules.AddNum();
                        if (rule_node->src_ip_begin <= trace->key[0] && trace->key[0] <= rule_node->src_ip_end &&
                            rule_node->dst_ip_begin <= trace->key[1] && trace->key[1] <= rule_node->dst_ip_end &&
                            rule_node->src_port_begin <= trace->key[2] && trace->key[2] <= rule_node->src_port_end &&
                            rule_node->dst_port_begin <= trace->key[3] && trace->key[3] <= rule_node->dst_port_end &&
                            rule_node->protocol_begin <= trace->key[4] && trace->key[4] <= rule_node->protocol_end) {
                            priority = rule_node->priority;
                            break;
                        }
                        rule_node = rule_node->next;  
                    };
                }
                break;
            }
            hash_node = hash_node->next;
        }
    }
    // printf("lookup layer %d priority %d\n", tuple_layer, priority);
    if (start_tuple_layer) {
        program_state->access_tuples.Update();
        program_state->access_tables.Update();
        program_state->access_nodes.Update();
        program_state->access_rules.Update();
    }
    return priority;
}

int MultilayerTuple::Reconstruct() {
	return 0;
}

uint64_t MultilayerTuple::MemorySize() {
    uint64_t memory_size = sizeof(MultilayerTuple);
    memory_size += sizeof(MTuple*) * max_tuples_num;
    memory_size += 64 * tuples_num; // map
    for (int i = 0; i < tuples_num; ++i) {
        memory_size += tuples_arr[i]->MemorySize();
        //printf("%d\n", tuples_arr[i]->MemorySize());
        //return memory_size;

    }
    return memory_size;
	return 0;
}

int MultilayerTuple::CalculateState(ProgramState *program_state) {
    if (start_tuple_layer)
        program_state->tuples_num = tuples_num;
    program_state->tuples_sum += tuples_num;
    for (int i = 0; i < tuples_num; ++i)
        tuples_arr[i]->CalculateState(program_state);

    // printf("tuples_num %d\n", tuples_num);
	return 0;
}

int MultilayerTuple::GetRules(vector<Rule*> &rules) {
    for (int i = 0; i < tuples_num; ++i)
        tuples_arr[i]->GetRules(rules);
	return 0;
}

int MultilayerTuple::Free(bool free_self) {
    for (int i = 0; i < tuples_num; ++i)
        tuples_arr[i]->Free(true);
    free(tuples_arr);
    tuples_map.clear();
    if (free_self)
        free(this);
	return 0;
}

int MultilayerTuple::Test(void *ptr) {
	return 0;
}