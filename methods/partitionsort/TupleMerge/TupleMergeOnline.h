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
#ifndef TUPLEMERGE
#define TUPLEMERGE

#include "../ElementaryClasses.h"
#include "../Simulation.h"

#include "SlottedTable.h"
#include <map>

using namespace std;

namespace ForgeUtils {
	void Crazify(TupleMergeUtils::TMTuple& tuple);
}

class TupleMergeOnline : public PacketClassifier {
public:
	TupleMergeOnline(const std::unordered_map<std::string, std::string>& args);
	~TupleMergeOnline();
	
	virtual void ConstructClassifier(const std::vector<PSRule>& rules);
	virtual int ClassifyAPacket(const PSPacket& p);
	virtual int ClassifyAPacketAccess(const PSPacket& one_packet, ProgramState *program_state);
	virtual void DeleteRule(const PSRule& rule);
	virtual void InsertRule(const PSRule& r);
	virtual int CalculateState(ProgramState *program_state);
	virtual Memory MemSizeBytes() const {
		int ruleSizeBytes = 19; // TODO variables sizes
		int sizeBytes = 0;
		sizeBytes += sizeof(TupleMergeOnline);
		for (const auto table : tables) {
			sizeBytes += table->MemSizeBytes(ruleSizeBytes);
		}
		int assignmentsSizeBytes = rules.size() * 8;
		int arraySize = tables.size() * 8;
		return sizeBytes + assignmentsSizeBytes + arraySize;
	}
	virtual int MemoryAccess() const {
		int cost = 0;
		/*
		for (const auto t : tables) {
			cost += t->MemoryAccess() + 1;
		}*/
		return cost;
	}
	virtual size_t NumTables() const { return tables.size(); }
	virtual size_t RulesInTable(size_t index) const { return tables[index]->NumRules(); }
	virtual size_t PriorityOfTable(size_t index) const {
		return tables[index]->MaxPriority();
	}

protected:
	void Resort() {
		sort(tables.begin(), tables.end(), [](auto& tx, auto& ty) { return tx->MaxPriority() > ty->MaxPriority(); });
	}
	SlottedTable* FindOrMake(const TupleMergeUtils::TMTuple& t);
	
	std::vector<SlottedTable*> tables;
	std::unordered_map<int, SlottedTable*> assignments; // Priority -> Table
	map<const PSRule, SlottedTable*> assignments2; // Priority -> Table

	std::vector<PSRule> rules;

	int collideLimit;
};



#endif