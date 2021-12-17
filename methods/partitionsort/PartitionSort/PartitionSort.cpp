#include "PartitionSort.h"

void PartitionSort::InsertRule(const PSRule& one_rule) {

 
	for (auto mitree : mitrees)
	{
		bool prioritychange = false;
		
		bool success = mitree->TryInsertion(one_rule, prioritychange);
		if (success) {
			
			if (prioritychange) {
				InsertionSortMITrees();
			}
			mitree->ReconstructIfNumRulesLessThanOrEqualTo(10);
			rule_tree_map[&one_rule] = mitree;
			// printf("insert %llu\n", &one_rule);
			// rules.push_back(std::make_pair(one_rule, mitree));
			return;
		}
	}
	bool priority_change = false;
	 
	auto tree_ptr = new OptimizedMITree(one_rule);
	tree_ptr->TryInsertion(one_rule, priority_change);
	rule_tree_map[&one_rule] = tree_ptr;
	// printf("insert %llu\n", &one_rule);
	// rules.push_back(std::make_pair(one_rule, tree_ptr));
	mitrees.push_back(tree_ptr);  
	InsertionSortMITrees();
}


void PartitionSort::DeleteRule(const PSRule& one_rule){
	// if (i < 0 || i >= rules.size()) {
	// 	printf("Warning index delete rule out of bound: do nothing here\n");
	// 	printf("%lu vs. size: %lu", i, rules.size());
	// 	return;
	// }
	// printf("delete %llu\n", &one_rule);
	if (rule_tree_map.find(&one_rule) == rule_tree_map.end()) {
		printf("Warning index delete rule out of bound: do nothing here\n");
		return;
	}
	bool prioritychange = false;

	OptimizedMITree * mitree = rule_tree_map[&one_rule]; 
	mitree->Deletion(one_rule, prioritychange); 
 
	if (prioritychange) {
		InsertionSortMITrees();
	}


	if (mitree->Empty()) {
		mitrees.pop_back();
		delete mitree;
	}


	// if (i != rules.size() - 1) {
	// 	rules[i] = std::move(rules[rules.size() - 1]);
	// }
	// rules.pop_back();


}

int PartitionSort::CalculateState(ProgramState *program_state) {
	program_state->tuples_num = mitrees.size();
	program_state->tuples_sum = mitrees.size();
	return 0;
}