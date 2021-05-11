#ifndef  PSORT_H
#define  PSORT_H

#include "OptimizedMITree.h"
#include "../Simulation.h"
#include "SortableRulesetPartitioner.h"
class PartitionSort : public PacketClassifier {

public:
	~PartitionSort() {
		for (auto x : mitrees) {
			free(x);
		}
	}
	void ConstructClassifier(const std::vector<PSRule>& rules)  {
		
		this->rules.reserve(rules.size());
		for (const auto& r : rules) {
			InsertRule(r);
		}
	}
	int ClassifyAPacket(const PSPacket& packet) {
		int result = 0;
		int query = 0;
		for (const auto& t : mitrees) {
		
			if (result > t->MaxPriority()){
				break;
			}
			query++;
			result = std::max(t->ClassifyAPacket(packet), result);
		}
		QueryUpdate(query);
		return result;
		
	}  
	int ClassifyAPacketAccess(const PSPacket& packet, ProgramState *program_state) {
		program_state->access_tuples.ClearNum();
	    program_state->access_nodes.ClearNum();
	    program_state->access_rules.ClearNum();
		int result = 0;
		int query = 0;
		for (const auto& t : mitrees) {
		
			if (result > t->MaxPriority()){
				break;
			}
			program_state->access_tuples.AddNum();
			query++;
			result = std::max(t->ClassifyAPacketAccess(packet, program_state), result);
		}
		QueryUpdate(query);
	    program_state->access_tuples.Update();
	    program_state->access_nodes.Update();
	    program_state->access_rules.Update();
		return result;
		
	}
	void DeleteRule(const PSRule& one_rule);
	void InsertRule(const PSRule& one_rule);

	int CalculateState(ProgramState *program_state);
	
	Memory MemSizeBytes() const {
		int size_total_bytes = 0;
		size_total_bytes += sizeof(PartitionSort);
		for (const auto& t : mitrees) {
			size_total_bytes += t->MemoryConsumption();
		}
		size_total_bytes += sizeof(OptimizedMITree *) * mitrees.size();
		size_total_bytes += sizeof(pair<PSRule,OptimizedMITree *>) * rule_tree_map.size();

		return size_total_bytes;
	}
	int MemoryAccess() const {
		return 0;
	}
	size_t NumTables() const {
		return mitrees.size();
	}
	size_t RulesInTable(size_t index) const { return mitrees[index]->NumRules(); }

protected:
	std::vector<OptimizedMITree *> mitrees;
	std::vector<std::pair<PSRule,OptimizedMITree *>> rules;
	map<const PSRule*, OptimizedMITree *> rule_tree_map;

	 
	void InsertionSortMITrees() {
		int i, j, numLength = mitrees.size();
		OptimizedMITree * key;
		for (j = 1; j < numLength; j++)
		{
			key = mitrees[j];
			for (i = j - 1; (i >= 0) && (mitrees[i]-> MaxPriority() < key-> MaxPriority()); i--)
			{
				mitrees[i + 1] = mitrees[i];
			}
			mitrees[i + 1] = key;
		}
	}

};
#endif
