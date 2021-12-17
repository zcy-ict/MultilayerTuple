#include "mhashnode.h"

using namespace std;

extern int max_layers_num;
extern int prefix_dims_num;
extern uint32_t create_next_layer_rules_num;
extern uint32_t delete_next_layer_rules_num;

MHashNode::MHashNode(uint32_t *_keys, uint32_t _hash) {
	for (int i = 0; i < prefix_dims_num; ++i)
		keys[i] = _keys[i];
    hash = _hash;
    rules_num = 0;
    max_priority = 0;
    rule_node = NULL;
    next = NULL;

    has_next_multilayertuple = false;
    next_multilayertuple = NULL;
}

bool MHashNode::SameKey(uint32_t *_keys) {
	for (int i = 0; i < prefix_dims_num; ++i)
		if (keys[i] != _keys[i])
			return false;
	return true;
}

int MHashNode::InsertRule(Rule *rule, uint32_t tuple_layer) {

    if (has_next_multilayertuple) {
        if (next_multilayertuple->InsertRule(rule) > 0) {
            printf("next_multilayertuple->Insert fail\n");
            exit(1);
        }
        ++rules_num;
        if (rule->priority > max_priority)
            max_priority = rule->priority;
        return 0;
    }
    // insert rule_node list
    MRuleNode *insert_rule_node = new MRuleNode(rule);
    MRuleNode *pre_rule_node = NULL;
    MRuleNode *next_rule_node = rule_node;
    while (true) {
        if (!next_rule_node || rule->priority > next_rule_node->rule->priority) {
            if (!pre_rule_node) {
                insert_rule_node->next = rule_node;
                rule_node = insert_rule_node;
            } else {
                pre_rule_node->next = insert_rule_node;
                insert_rule_node->next = next_rule_node;
            }
            ++rules_num;
            // printf("insert hashnode \n");
            break;
        }
        pre_rule_node = next_rule_node;
        next_rule_node = next_rule_node->next;
    }
    if (rule->priority > max_priority)
    	max_priority = rule->priority;

    if (rules_num >= create_next_layer_rules_num && !has_next_multilayertuple && tuple_layer < max_layers_num) {
        // printf("Create next_multilayertuple\n");
        vector<Rule*> rules;
        GetRules(rules);
        int get_rules_num = rules.size();
        Free(false);

        has_next_multilayertuple = true;
        next_multilayertuple = new MultilayerTuple();
        next_multilayertuple->Init(tuple_layer + 1, false);
        next_multilayertuple->Create(rules, false);
        for (int i = 0; i < get_rules_num; ++i)
            InsertRule(rules[i], tuple_layer);
    }

	return 0;
}

int MHashNode::DeleteRule(Rule *rule, uint32_t tuple_layer) {
    bool delete_success = false;
    if (has_next_multilayertuple) {
        if (next_multilayertuple->DeleteRule(rule) > 0) {
            printf("next_multilayertuple->Insert fail\n");
            exit(1);
        }
        --rules_num;
        max_priority = next_multilayertuple->max_priority;
        delete_success = true;
    }

    if (!delete_success) {
        // delete in rule_node list
        MRuleNode *pre_rule_node = NULL;
        MRuleNode *next_rule_node = rule_node;
        while (next_rule_node) {
            if (SameRule(next_rule_node->rule, rule)) {
                --rules_num;
                if (!pre_rule_node) {
                    rule_node = next_rule_node->next;
                    if (rules_num == 0)
                        max_priority = 0;
                    else
                        max_priority = rule_node->priority;
                } else {
                    pre_rule_node->next = next_rule_node->next;
                }
                next_rule_node->Free(true);
                delete_success = true;
                break;
            }
            pre_rule_node = next_rule_node;
            next_rule_node = next_rule_node->next;
        }
    }
    
    if (has_next_multilayertuple && rules_num <= delete_next_layer_rules_num) {
        // printf("delete next_multilayertuple\n");
        vector<Rule*> rules;
        GetRules(rules);
        int get_rules_num = rules.size();
        Free(false);

        for (int i = 0; i < get_rules_num; ++i)
            InsertRule(rules[i], tuple_layer);
        // printf("rules_num %d\n", rules_num);
    }

	return !delete_success;
}

uint64_t MHashNode::MemorySize() {
    uint64_t memory_size = sizeof(MHashNode);
    MRuleNode *rule_node2 = rule_node;
    while (rule_node2) {
        memory_size += rule_node2->MemorySize();
        rule_node2 = rule_node2->next;
    }
    if (has_next_multilayertuple)
        memory_size += next_multilayertuple->MemorySize();
    return memory_size;
}

int MHashNode::CalculateState(ProgramState *program_state) {
    if (has_next_multilayertuple) {
        ++program_state->next_layer_num;
        next_multilayertuple->CalculateState(program_state);
    }
	return 0;
}

int MHashNode::GetRules(vector<Rule*> &rules) {
    MRuleNode *current_rule_node = rule_node, *next_rule_node = NULL;
    while (current_rule_node) {
        next_rule_node = current_rule_node->next;
        rules.push_back(current_rule_node->rule);
        current_rule_node = next_rule_node;
    }

    if (has_next_multilayertuple)
        next_multilayertuple->GetRules(rules);
	return 0;
}

int MHashNode::Free(bool free_self) {
    MRuleNode *current_rule_node = rule_node, *next_rule_node = NULL;
    while (current_rule_node) {
        next_rule_node = current_rule_node->next;
        current_rule_node->Free(true);
        current_rule_node = next_rule_node;
    }
    rule_node = NULL;
    rules_num = 0;
    max_priority = 0;

    if (has_next_multilayertuple) {
        has_next_multilayertuple = false;
        next_multilayertuple->Free(true);
        next_multilayertuple = NULL;
    }
    if (free_self)
        free(this);
	return 0;
}

int MHashNode::Test(void *ptr) {
	return 0;
}