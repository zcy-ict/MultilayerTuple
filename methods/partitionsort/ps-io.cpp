#include "ps-io.h"

using namespace std;

vector<PSRule> GeneratePSRules(vector<Rule*> &rules) {
	vector<PSRule> ps_rules;
	int rules_num = rules.size();
	for (int i = 0; i < rules_num; ++i) {
		PSRule rule;
		rule.priority = rules[i]->priority;
		rule.markedDelete = 0;
		for (int j = 0; j < 5; ++j) {
			rule.range[j][0] = rules[i]->range[j][0];
			rule.range[j][1] = rules[i]->range[j][1];
			if (j == 0 || j == 1)
				rule.prefix_length[j] = rules[i]->prefix_len[j];
			else if (j == 2 || j == 3)
				rule.prefix_length[j] = rule.range[j][0] == rule.range[j][1] ? 32 : 16;
			else
				rule.prefix_length[j] = rule.range[j][0] == rule.range[j][1] ? 32 : 24;
		}
		ps_rules.push_back(rule);
	}
	return ps_rules;
}

// void PrintByteCutsRules(vector<BCRule> &rules) {

//     FILE *fp = fopen("rules2", "w");
// 	for (int i = 0; i < rules.size();++i) {
// 		fprintf(fp, "%d %d ", rules[i].priority - 1, rules[i].markedDelete);
// 		for (int j = 0; j < 5; ++j)
// 			fprintf(fp, "%u %u %u ", rules[i].range[j].low, rules[i].range[j].high, rules[i].prefix_length[j]);
// 		fprintf(fp, "\n");
// 	}
// 	fclose(fp);
// }

vector<PSPacket> GeneratePSPackets(vector<Trace*> &traces) {
	vector<PSPacket> packets;
	int traces_num = traces.size();
	for (int i = 0; i < traces_num; ++i) {
		PSPacket packet;
		for (int j = 0; j < 5; ++j)
			packet.push_back(traces[i]->key[j]);
		packets.push_back(packet);
	}
	return packets;
}