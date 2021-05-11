#ifndef  TUPLE_H
#define  TUPLE_H

#define POINTER_SIZE_BYTES 4

#include "../Simulation.h"
#include "cmap.h"
#include <unordered_map>
#include <algorithm>
#include <fstream>

using namespace std;

struct Tuple {
public:
	Tuple(const std::vector<int>& dims, const std::vector<unsigned int>& lengths, const PSRule& r) : dims(dims), lengths(lengths) {
		for (int w : lengths) {
			tuple.push_back(w);
		}
		cmap_init(&map_in_tuple);
		Insertion(r);
	}
	//~Tuple() { Destroy(); }
	void Destroy() {
		cmap_destroy(&map_in_tuple);
	}

	bool IsEmpty() { return CountNumRules() == 0; }

	int FindMatchPacket(const PSPacket& p);
	int FindMatchPacketAccess(const PSPacket& p, ProgramState *program_state);
	void Insertion(const PSRule& r);
	void Deletion(const PSRule& r);
	int WorstAccesses() const;
	int CountNumRules() const  {
		return cmap_count(&map_in_tuple);
	//	return  table.size();
	}
	Memory MemSizeBytes(Memory ruleSizeBytes) const {
		uint64_t memory_size = sizeof(Tuple);
		memory_size += CmapMemorySize(&map_in_tuple);
		return memory_size;
		//return table.size() * ruleSizeBytes + table.bucket_count() * POINTER_SIZE_BYTES;
	}

	void printsipdip() {
		//printf("sipdip: %d %d\n", sip_length, dip_length);
	}
protected:
	bool inline IsPacketMatchToRule(const PSPacket& p, const PSRule& r);
	unsigned int inline HashRule(const PSRule& r) const;
	unsigned int inline HashPacket(const PSPacket& p) const;
	cmap map_in_tuple;
	//std::unordered_map<uint32_t, std::vector<PSRule>> table;

	std::vector<int> dims;
	std::vector<unsigned int> lengths;
	std::vector<int> tuple;
};

struct PriorityTuple : public Tuple {
public:
	PriorityTuple(const std::vector<int>& dims, const std::vector<unsigned int>& lengths, const PSRule& r) :Tuple(dims, lengths, r){
		maxPriority = r.priority;
		priority_container.insert(maxPriority);
	}
	void Insertion(const PSRule& r, bool& priority_change);
	void Deletion(const PSRule& r, bool& priority_change);

	int maxPriority = -1;
	std::multiset<int> priority_container;
};

class TupleSpaceSearch : public PacketClassifier {
	
public:
	virtual ~TupleSpaceSearch() {
		for (auto p : all_tuples) {
			p.second.Destroy();
		}
	}

	void ConstructClassifier(const std::vector<PSRule>& r);
	int ClassifyAPacket(const PSPacket& one_packet);
	int ClassifyAPacketAccess(const PSPacket& one_packet, ProgramState *program_state);
	void DeleteRule(const PSRule& one_rule);
	void InsertRule(const PSRule& one_rule);
	int CalculateState(ProgramState *program_state);

	int MemoryAccess() const {
		return WorstAccesses();
	}
	virtual int WorstAccesses() const;
	Memory MemSizeBytes() const {
		int ruleSizeBytes = 19; // TODO variables sizes
		int sizeBytes = 0;
		for (auto& pair : all_tuples) {
			sizeBytes += pair.second.MemSizeBytes(ruleSizeBytes);
		}
		return sizeBytes;
	}
	void PlotTupleDistribution() {

		std::vector<std::pair<unsigned int, Tuple> > v;
		copy(all_tuples.begin(), all_tuples.end(), back_inserter(v));
		
		auto cmp = [](const std::pair<unsigned int, Tuple>& lhs, const std::pair<unsigned int, Tuple>& rhs) {
			return lhs.second.CountNumRules() > rhs.second.CountNumRules();
		};
		sort(v.begin(), v.end(), cmp);

		std::ofstream log("logfile.txt", std::ios_base::app | std::ios_base::out);
		int sum = 0;
		for (auto x : v) {
			sum += x.second.CountNumRules();
		}
		log << v.size() << " " << sum << " ";
		int left = 5;
		for (auto x : v) {
			log << x.second.CountNumRules() << " ";
			left--;
			if (left <= 0) break;
		}
		log << std::endl;
	}
	virtual int GetNumberOfTuples() const {
		return all_tuples.size();
	}
	size_t NumTables() const { return GetNumberOfTuples(); }
	size_t RulesInTable(size_t index) const { return 0; } // TODO : assign some order
protected:
	uint64_t inline KeyRulePrefix(const PSRule& r) {
		int key = 0;
		for (int d : dims) {
			key <<= 6;
			key += r.prefix_length[d];
		}
		return key;
	}
	std::unordered_map<uint64_t, Tuple> all_tuples;
	//maintain rules for monitoring purpose
	std::vector<PSRule> rules;
	std::vector<int> dims;
};

class PriorityTupleSpaceSearch : public TupleSpaceSearch {

public:
	int ClassifyAPacket(const PSPacket& one_packet);
	int ClassifyAPacketAccess(const PSPacket& one_packet, ProgramState *program_state);
	void DeleteRule(const PSRule& one_rule);
	void InsertRule(const PSRule& one_rule);
	int CalculateState(ProgramState *program_state);
	int WorstAccesses() const;
	Memory MemSizeBytes() const {
		uint64_t memory_size = sizeof(PriorityTupleSpaceSearch);
		int ruleSizeBytes = 19; // TODO variables sizes
		int sizeBytes = 0;
		memory_size += (64 + sizeof(PriorityTuple*))* priority_tuples_vector.size();
		memory_size += 64 * all_priority_tuples.size();
		for (auto& tuple : priority_tuples_vector) {
			memory_size += tuple->MemSizeBytes(ruleSizeBytes);
		}
		return memory_size;
	}

	int GetNumberOfTuples() const {
		return all_priority_tuples.size();
	}
	void PlotPriorityTupleDistribution() {

		RetainInvaraintOfPriorityVector();
		std::ofstream log("logfile.txt", std::ios_base::app | std::ios_base::out);
		int sum = 0;
		for (auto x : priority_tuples_vector) {
			sum += x->CountNumRules();
		}
		log << priority_tuples_vector.size() << " " << sum << " ";
		int left = 5;
		for (auto x : priority_tuples_vector) {
			log << x->CountNumRules() << " ";
			left--;
			if (left <= 0) break;
		}
		log << std::endl;

	}

	size_t NumTables() const { return priority_tuples_vector.size(); }
	size_t RulesInTable(size_t index) const { return priority_tuples_vector[index]->CountNumRules(); }
private:
	void RetainInvaraintOfPriorityVector() {
		std::sort(begin(priority_tuples_vector), end(priority_tuples_vector), []( PriorityTuple * lhs,  PriorityTuple * rhs) { return lhs->maxPriority > rhs->maxPriority; });
	}
	std::unordered_map<uint64_t, PriorityTuple *> all_priority_tuples;
	std::vector<PriorityTuple *> priority_tuples_vector;
};


#endif