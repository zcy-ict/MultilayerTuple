#ifndef  UTIL_H
#define  UTIL_H
#include "../ElementaryClasses.h"


class WeightedInterval;
struct LightWeightedInterval;
class Utilities {
	static const int LOW = 0;
	static const int HIGH = 1;
public:
	static bool  IsIdentical(const PSRule& r1, const PSRule& r2);
	static int GetMaxOverlap(const std::multiset<unsigned int>& lo, const std::multiset< unsigned int>& hi);

	static std::pair<std::vector<int>, int> MWISIntervals(const std::vector<WeightedInterval>&I);
	static std::pair<std::vector<int>, int> FastMWISIntervals(const std::vector<LightWeightedInterval>&I);
	static std::pair<std::vector<int>, int> MWISIntervals(const std::vector<PSRule>& I, int x);

	static std::vector<WeightedInterval> CreateUniqueInterval(const std::vector<PSRule>& rules, int field);
	static std::vector<LightWeightedInterval> FastCreateUniqueInterval(const std::vector<interval>& rules);
	static std::vector<std::vector<WeightedInterval>> CreateUniqueIntervalsForEachField(const std::vector<PSRule>& rules);
 
	static std::vector<PSRule> RedundancyRemoval(const std::vector<PSRule>& rules);

};
 
struct LightWeightedInterval {
	static const int LOW = 0;
	static const int HIGH = 1;
	LightWeightedInterval(unsigned int a, unsigned int b, int w) : a(a), b(b), w(w) {}
	unsigned int a;
	unsigned int b;
	int w;
	unsigned int GetLow() const { return a; }
	unsigned int GetHigh() const { return b; }
	int GetWeight() const { return w; }
	void Push(int x) {
		rule_indices.push_back(x);
	}
	std::vector<int> GetRuleIndices() const {
		return rule_indices;
	}
	std::vector<int> rule_indices;
};


class WeightedInterval{
	static const int LOW = 0;
	static const int HIGH = 1;
public:

	WeightedInterval(const std::vector<PSRule>& rules, unsigned int a, unsigned int b) : rules(rules) {
		if (rules.size() == 0) {
			std::cout << "ERROR: EMPTY RULE AND CONSTRUCT INTERVAL?" << std::endl;
			exit(0);
		}
		ival = std::make_pair(a, b);
		SetWeightBySizePlusOne();
		field = 0;
	}
	WeightedInterval(const std::vector<PSRule>& rules, int field) : rules(rules), field(field) {
		if (rules.size() == 0) {
			std::cout << "ERROR: EMPTY RULE AND CONSTRUCT INTERVAL?" << std::endl;
			exit(0);
		}
		ival = std::make_pair(rules[0].range[field][LOW], rules[0].range[field][HIGH]);
		SetWeightBySizePlusOne();
	}
	int CountPOM(int second_field) {
		std::multiset<unsigned int> Astart;
		std::multiset<unsigned int> Aend;
		for (int i = 0; i < (int)rules.size();
			 i++) {
			Astart.insert(rules[i].range[second_field][LOW]);
			Aend.insert(rules[i].range[second_field][HIGH]);
		}
		return Utilities::GetMaxOverlap(Astart, Aend);
	}
	void SetWeightByPenaltyPOM(int second_field) {
		weight = std::max((int)rules.size() - 100 * CountPOM(second_field), 1);
	}
	void SetWeightBySizePlusOne() {
		weight = rules.size() +1;
	}
	void SetWeightBySize() {
		weight = rules.size();
	}
	unsigned int GetLow() const{ return ival.first; }
	unsigned int GetHigh() const { return ival.second; }
	std::vector<PSRule> GetRules() const{ return rules; }
	int GetField() const { return field; }
	int GetWeight() const { return weight; }
protected:
	std::pair<unsigned int, unsigned int> ival;
	int weight = 100000;
	std::vector<PSRule> rules;
	int  field;
};


#endif
