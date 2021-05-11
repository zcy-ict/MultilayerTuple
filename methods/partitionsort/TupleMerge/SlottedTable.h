/*
 * MIT License
 *
 * Copyright (c) 2017 by J. Daly at Michigan State University
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef SLOTTEDTABLE
#define SLOTTEDTABLE

#include "../Simulation.h"

#include "../OVS/TupleSpaceSearch.h"
#include "../OVS/cmap.h"

#include <unordered_set>

using namespace std;

namespace TupleMergeUtils {
	typedef std::vector<int> TMTuple;

	int Log2(unsigned int x);
	int Sum(const std::vector<int>& v);

	uint32_t Mask(int bits);
	bool CompatibilityCheck(const TMTuple& ruleTuple, const TMTuple& tableTuple);
	bool AreSame(const TMTuple& t1, const TMTuple& t2);
	void PreferedTuple(const PSRule& r, TMTuple& tuple);
	void BestTuple(const std::vector<PSRule>& rules, TMTuple& tuple);
	uint32_t Hash(const PSRule& r, const TMTuple& tuple);
	uint32_t Hash(const PSPacket& r, const TMTuple& tuple);
	
	bool IsHashable(const std::vector<PSRule>& rules, size_t collisionLimit);
	void PrintTuple(const TMTuple& tuple);

	struct TupleHasher {
		std::size_t operator()(const TMTuple& v) const {
			int hash = 0;
			for (int x : v) {
				hash *= 17;
				hash += x;
			}
			return hash;
		}
	};

	struct TupleEquals {
		bool operator()(const TMTuple& x, const TMTuple& y) const {
			if (x.size() != y.size()) return false;
			for (size_t i = 0; i < x.size(); i++) {
				if (x[i] != y[i]) return false;
			}
			return true;
		}
	};
	
	typedef std::unordered_set<TMTuple, TupleHasher, TupleEquals> TupleSet;
	
	template<class T>
	using TupleMap = std::unordered_map<TMTuple, T, TupleHasher, TupleEquals>;
}

struct SlottedTable {
public:
	SlottedTable(const std::vector<int>& dims, const std::vector<unsigned int>& lengths) 
			: dims(dims), lengths(lengths), maxPriority(-1) {
		cmap_init(&map_in_tuple);
	}
	SlottedTable(const TupleMergeUtils::TMTuple& tuple);
	~SlottedTable() { cmap_destroy(&map_in_tuple); }

	bool IsEmpty() { return NumRules() == 0; }

	int ClassifyAPacket(const PSPacket& p) const;
	int ClassifyAPacketAccess(const PSPacket& p, ProgramState *program_state) const;
	void Insertion(const PSRule& r, bool& priority_change);
	bool Deletion(const PSRule& r, bool& priority_change);
	
	bool CanInsert(const TupleMergeUtils::TMTuple& tuple) const {
		for (size_t i = 0; i < dims.size(); i++) {
			if (lengths[i] > tuple[dims[i]]) return false;
		}
		return true;
	}
	bool IsThatTuple(const TupleMergeUtils::TMTuple& tuple) const;
	bool CanTakeRulesFrom(const SlottedTable* table) const;
	bool HaveSameTuple(const SlottedTable* table) const;
	
	size_t NumCollisions(const PSRule& r) const;
	std::vector<PSRule> Collisions(const PSRule& r) const;
	std::vector<PSRule> GetRules() const;
	
	int WorstAccesses() const;
	int NumRules() const  {
		return cmap_count(&map_in_tuple);
	}
	Memory MemSizeBytes(Memory ruleSizeBytes) const {
		uint64_t memory_size = sizeof(SlottedTable);
		memory_size += CmapMemorySize(&map_in_tuple);
		return memory_size;
		// return 	cmap_count(&map_in_tuple)* ruleSizeBytes + cmap_array_size(&map_in_tuple) * 8;
	}

	int MaxPriority() const { return maxPriority; };
	
protected:
	uint32_t inline HashRule(const PSRule& r) const;
	uint32_t inline HashPacket(const PSPacket& p) const;
	
	cmap map_in_tuple;

	std::vector<int> dims;
	std::vector<unsigned int> lengths;
	
	int maxPriority = -1;
	std::multiset<int> priority_container;
};

#endif