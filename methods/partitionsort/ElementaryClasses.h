#ifndef  ELEM_H
#define  ELEM_H
#include <vector>
#include <queue>
#include <list>
#include <set>
#include <iostream>
#include <algorithm>
#include <random>
#include <numeric>
#include <memory>
#include <chrono> 
#include <array>
#include <map>
#include <unordered_map>

#include "../../elementary.h"

#define FieldSA 0
#define FieldDA 1
#define FieldSP 2
#define FieldDP 3
#define FieldProto 4

#define LowDim 0
#define HighDim 1

#define POINT_SIZE_BITS 32

using namespace std;

typedef uint32_t Point;
typedef vector<Point> PSPacket;

struct PSRule
{
	//PSRule(){};
	PSRule(int dim = 5) : dim(dim), range(dim, { { 0, 0 } }), prefix_length(dim, 0){ markedDelete = 0; }
 
	int dim;
	int	priority;

	int id;
	int tag;
	bool markedDelete = 0;

	vector<unsigned> prefix_length;

	vector<array<Point,2>> range;

	bool inline MatchesPacket(const PSPacket& p) const {
		for (int i = 0; i < dim; i++) {
			if (p[i] < range[i][LowDim] || p[i] > range[i][HighDim]) return false;
		}
		return true;
	}

	void Print() const {
		for (int i = 0; i < dim; i++) {
			printf("%u:%u ", range[i][LowDim], range[i][HighDim]);
		}
		printf("\n");
	}

	bool operator<(const PSRule& rule)const{
		for (int i = 0; i < 5; ++i)
			for (int j = 0; j < 2; ++j)
				if (range[i][j] != rule.range[i][j])
					return range[i][j] < rule.range[i][j];
		for (int i = 0; i < 5; ++i)
			if (prefix_length[i] != rule.prefix_length[i])
				return prefix_length[i] < rule.prefix_length[i];
		return priority < rule.priority;
    }

    bool operator==(const PSRule& rule)const {
		for (int i = 0; i < 5; ++i)
			for (int j = 0; j < 2; ++j)
				if (range[i][j] != rule.range[i][j])
					return false;
		for (int i = 0; i < 5; ++i)
			if (prefix_length[i] != rule.prefix_length[i])
				return false;
		return priority == rule.priority;
    }
};



class Interval {
public:
	Interval() {}
	virtual Point GetLowPoint() const = 0;
	virtual Point GetHighPoint() const = 0;
	virtual void Print() const=0;
};

class interval : public Interval {
public: 
	interval(unsigned int a, unsigned int b, int id) : a(a), b(b), id(id) {}
	Point GetLowPoint() const { return a; }
	Point GetHighPoint() const { return b; }
	void Print()const {};

	Point a, b;
	bool operator < (const interval& rhs) const {
		if (a != rhs.a) {
			return a < rhs.a;
		} else return b < rhs.b;
	}
	bool operator == (const interval& rhs) const {
		return a == rhs.a && b == rhs.b;
	}
	int id;
	int weight;

};
struct EndPoint {
	EndPoint(double val, bool isRightEnd, int id) : val(val), isRightEnd(isRightEnd), id(id){}
	bool operator < (const EndPoint & rhs) const {
		return val < rhs.val;
	}
	double val;
	bool isRightEnd;
	int id;
};
class Random {
public:
	// random number generator from Stroustrup: 
	// http://www.stroustrup.com/C++11FAQ.html#std-random
	// static: there is only one initialization (and therefore seed).
	// static int random_int(int low, int high)
	// {
	// 	//static mt19937  generator;
	// 	using Dist = uniform_int_distribution < int >;
	// 	static Dist uid{};
	// 	return uid(generator, Dist::param_type{ low, high });
	// }

	// random number generator from Stroustrup: 
	// http://www.stroustrup.com/C++11FAQ.html#std-random
	// static: there is only one initialization (and therefore seed).
	// static int random_unsigned_int()
	// {
	// 	//static mt19937  generator;
	// 	using Dist = uniform_int_distribution < unsigned int >;
	// 	static Dist uid{};
	// 	return uid(generator, Dist::param_type{ 0, 4294967295 });
	// }
	static int random_unsigned_int()
	{
		//static mt19937  generator;
		return (uint32_t)rand() << 16 | rand();
		// using Dist = uniform_int_distribution < unsigned int >;
		// static Dist uid{};
		// return uid(generator, Dist::param_type{ 0, 4294967295 });
	}
// 	static double random_real_btw_0_1()
// 	{
// 		//static mt19937  generator;
// 		using Dist = uniform_real_distribution < double >;
// 		static Dist uid{};
// 		return uid(generator, Dist::param_type{ 0,1 });
// 	}

	template <class T>
	static vector<T> shuffle_vector(vector<T> vi) {
		// static mt19937  generator;
		// shuffle(begin(vi), end(vi), generator);
		random_shuffle(vi.begin(), vi.end());
		return vi;
	}
// private:
// 	static mt19937 generator;
};


enum TestMode {
	ModeClassification,
	ModeUpdate,
	ModeValidation
};



enum ClassifierTests {
	TestNone = 0,
	TestPartitionSort = 0x0001,
	TestPriorityTuple = 0x0002,
	TestHyperSplit = 0x0004,
	TestHyperCuts = 0x0008,
	TestAll = 0xFFFFFFFF
};

enum PSMode {
	NoCompression,
	PathCompression,
	PriorityNode,
	NoIntermediateTree
};


inline ClassifierTests operator|(ClassifierTests a, ClassifierTests b) {
	return static_cast<ClassifierTests>(static_cast<int>(a) | static_cast<int>(b));
}

inline void SortRules(vector<PSRule>& rules) {
	sort(rules.begin(), rules.end(), [](const PSRule& rx, const PSRule& ry) { return rx.priority >= ry.priority; });
}

inline void SortRules(vector<PSRule*>& rules) {
	sort(rules.begin(), rules.end(), [](const PSRule* rx, const PSRule* ry) { return rx->priority >= ry->priority; });
}



typedef uint32_t Memory;


template<class K, class V>
const V& PSGetOrElse(const std::unordered_map<K, V> &m, const K& key, const V& def) {
	if (m.find(key) == m.end()) return def;
	else return m.at(key);
}


class PacketClassifier {
public:
	virtual void ConstructClassifier(const vector<PSRule>& rules) = 0;
	virtual int ClassifyAPacket(const PSPacket& packet) = 0;
	virtual int ClassifyAPacketAccess(const PSPacket& packet, ProgramState *program_state) = 0;
	virtual void DeleteRule(const PSRule& rule) = 0;
	virtual void InsertRule(const PSRule& rule) = 0;
	virtual Memory MemSizeBytes() const = 0;
	virtual int MemoryAccess() const = 0;
	virtual size_t NumTables() const = 0;
	virtual size_t RulesInTable(size_t tableIndex) const = 0;
    virtual int CalculateState(ProgramState *program_state) = 0;

	int TablesQueried() const {	return queryCount; }
	int NumPacketsQueriedNTables(int n) const { return PSGetOrElse<int, int>(packetHistogram, n, 0); };

protected:
	void QueryUpdate(int query) { 
		packetHistogram[query]++;
		queryCount += query;
	}

private:
	int queryCount = 0;
	std::unordered_map<int, int> packetHistogram;
};

class ClassifierTable {
public:
	virtual int ClassifyAPacket(const PSPacket& packet) const = 0;
	virtual void Insertion(const PSRule& rule, bool& priorityChange) = 0;
	virtual void Deletion(const PSRule& rule, bool& priorityChange) = 0;
	virtual bool CanInsertRule(const PSRule& rule) const = 0;

	virtual size_t NumRules() const = 0;
	virtual int MaxPriority() const = 0;

	virtual Memory MemSizeBytes(Memory ruleSize) const = 0;
	virtual std::vector<PSRule> GetRules() const = 0;
};

#endif
