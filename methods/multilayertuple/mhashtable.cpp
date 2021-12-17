#include "mhashtable.h"

using namespace std;

extern int prefix_dims_num;

int MHashTable::Init(int size, uint32_t _tuple_layer) {
	tuple_layer = _tuple_layer;

    hash_node_num = 0;
    max_hash_node_num = size * MHASHTABLEMAX;
	min_hash_node_num = size * MHASHTABLEMIN;
	mask = size - 1;
    hash_node_arr = (MHashNode**)malloc(sizeof(MHashNode*) * (mask + 1));
    for (int i = 0; i < size; ++i)
        hash_node_arr[i] = NULL;
    max_priority = 0;
	return 0;
}

int MHashTable::InsertHashNode(MHashNode *hash_node) {
    if (hash_node_num == max_hash_node_num)
        HashTableResize((mask + 1) << 1);
    ++hash_node_num;
    uint32_t index = hash_node->hash & mask;
    MHashNode *pre = hash_node_arr[index];

    if (!pre || hash_node->max_priority > pre->max_priority) {
        hash_node_arr[index] = hash_node;
		hash_node->next = pre;
        return 0;
    }
	MHashNode *next = pre->next;
    while (true) {
		if (!next || hash_node->max_priority > next->max_priority) {
			pre->next = hash_node;
			hash_node->next = next;
			return 0;
		}
		pre = next;
		next = pre->next;
	}
	return 1;
}

MHashNode* MHashTable::PickHashNode(uint32_t *keys, uint32_t hash) {
    uint32_t index = hash & mask;
    MHashNode *hash_node = hash_node_arr[index];
    MHashNode *pre_hash_node = NULL;
    while (hash_node) {
        if (hash_node->SameKey(keys)) {
            if (pre_hash_node == NULL)
				hash_node_arr[index] = hash_node->next;
			else
				pre_hash_node->next = hash_node->next;
			--hash_node_num;
			hash_node->next = NULL;
			return hash_node;
        }
        pre_hash_node = hash_node;
        hash_node = hash_node->next;
    }
    return NULL;
}

int MHashTable::HashTableResize(uint32_t size) {
	// printf("HashTableResize\n");
    uint32_t origin_size = mask + 1;
    MHashNode **origin_hash_node_arr = hash_node_arr;

    hash_node_num = 0;
    max_hash_node_num = size * MHASHTABLEMAX;
	min_hash_node_num = size * MHASHTABLEMIN;
	mask = size - 1;
    hash_node_arr = (MHashNode**)malloc(sizeof(MHashNode*) * (mask + 1));
    for (int i = 0; i < size; ++i)
        hash_node_arr[i] = NULL;

    MHashNode *hash_node, *next;
    for (int i = 0; i < origin_size; ++i) {
        hash_node = origin_hash_node_arr[i];
        while (hash_node) {
            next = hash_node->next;
            hash_node->next = NULL;
            InsertHashNode(hash_node);
            hash_node = next;
        }
    }
    free(origin_hash_node_arr);
    return 0;
}

int MHashTable::InsertRule(Rule *rule, uint32_t *keys, uint32_t hash) {
	MHashNode *hash_node = PickHashNode(keys, hash);
    if (!hash_node)
        hash_node = new MHashNode(keys, hash);
    // printf("hash_node %016lx\n", (uint64_t)hash_node);
    hash_node->InsertRule(rule, tuple_layer);
    InsertHashNode(hash_node);
    if (rule->priority > max_priority)
        max_priority = rule->priority;
	return 0;
}

int MHashTable::DeleteRule(Rule *rule, uint32_t *keys, uint32_t hash) {
    MHashNode *hash_node = PickHashNode(keys, hash);
    if (!hash_node) {
        printf("Wrong: No such hash_node\n");
        return 1;
    }
    if (hash_node->DeleteRule(rule, tuple_layer) > 0) {
        printf("Wrong: HashNode DeleteRule\n");
        return 1;
    }
    if (hash_node->rules_num == 0){
        hash_node->Free(true);
        if (hash_node_num < min_hash_node_num && mask + 1 > 32)
            HashTableResize((mask + 1)/ 2);
    }
    else{
        InsertHashNode(hash_node);
    }
    if (rule->priority == max_priority) {
        max_priority = 0;
        for (int i = 0; i <= mask; ++i)
            if (hash_node_arr[i])
                max_priority = max(max_priority, hash_node_arr[i]->max_priority);
    }
    return 0;
}

uint64_t MHashTable::MemorySize() {
    uint64_t memory_size = sizeof(MHashTable);
    memory_size += sizeof(MHashNode*) * (mask + 1);
    MHashNode *hash_node = NULL;
    for (int i = 0; i <= mask; ++i) {
        hash_node = hash_node_arr[i];
        while (hash_node) {
            memory_size += hash_node->MemorySize();
            hash_node = hash_node->next;
        }
    }
    return memory_size;
}

int MHashTable::CalculateState(ProgramState *program_state) {
    program_state->hash_node_num += hash_node_num;
    program_state->bucket_sum += mask + 1;
    MHashNode *hash_node = NULL;
    for (int i = 0; i <= mask; ++i) {
        hash_node = hash_node_arr[i];
        if (hash_node != NULL)
            ++program_state->bucket_use;
        while (hash_node) {
            hash_node->CalculateState(program_state);
            hash_node = hash_node->next;
        }
    }
	return 0;
}

int MHashTable::GetRules(vector<Rule*> &rules) {
    MHashNode *hash_node = NULL, *next_hash_node = NULL;
    for (int i = 0; i <= mask; ++i) {
        hash_node = hash_node_arr[i];
        while (hash_node) {
            next_hash_node = hash_node->next;
            hash_node->GetRules(rules);
            hash_node = next_hash_node;
        }
    }
	return 0;
}

int MHashTable::Free(bool free_self) {
    MHashNode *hash_node = NULL, *next_hash_node = NULL;
    for (int i = 0; i <= mask; ++i) {
        hash_node = hash_node_arr[i];
        while (hash_node) {
            next_hash_node = hash_node->next;
            hash_node->Free(true);
            hash_node = next_hash_node;
        }
    }
    free(hash_node_arr);
    if (free_self)
        free(this);
	return 0;
}

int MHashTable::Test(void *ptr) {
	return 0;
}