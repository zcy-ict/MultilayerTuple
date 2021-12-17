#include "io.h"

using namespace std;

vector<string> StrSplit(const string& str, const string& pattern) {
    vector<string> ret;
    if(pattern.empty())
        return ret;
    int start = 0, index = str.find_first_of(pattern, 0);
    while(index != str.npos) {
        if(start != index)
            ret.push_back(str.substr(start, index-start));
        start = index + 1;
        index = str.find_first_of(pattern, start);
    }
    if(!str.substr(start).empty())
        ret.push_back(str.substr(start));
    return ret;
}

int Count1(uint64_t num) {
	int ans = 0;
	while (num) {
		ans += num & 1;
		num /= 2;
	}
	return ans;
}

uint32_t GetIp(string str) {
	uint32_t num1, num2, num3, num4;
	sscanf(str.c_str(),"%u.%u.%u.%u", &num1, &num2, &num3, &num4);
	return (num1 << 24) | (num2 << 16) | (num3 << 8) | num4;
}

string GetIpStr(uint32_t ip) {
    char str[100];
    sprintf(str, "%d.%d.%d.%d", ip >> 24, ip >> 16 & 255, ip >> 8 & 255, ip & 255);
    return str;
}

void PrintIp(uint32_t ip) {
	printf("%d.%d.%d.%d", ip >> 24, ip >> 16 & 255, ip >> 8 & 255, ip & 255);
}

void PrintRule(Rule *rule) {
	PrintIp(rule->range[0][0]);
	printf("/%d ", rule->prefix_len[0]);
	PrintIp(rule->range[1][0]);
	printf("/%d ", rule->prefix_len[1]);
    for (int i = 2; i < 5; ++i)
        printf("%d:%d ", rule->range[i][0], rule->range[i][1]);
	printf("priority %d\n", rule->priority);
}

vector<Rule*> ReadRules(string rules_file, int rules_shuffle) {
    vector<Rule*> rules;
	int rules_num = 0;
    char buf[1025];

	FILE *fp = fopen(rules_file.c_str(), "rb");
	if (!fp)
		printf("Cannot open the file %s\n", rules_file.c_str());
    int priority = 0;
    while (fgets(buf,1000,fp)!=NULL)
        ++priority;
    
    fp = fopen(rules_file.c_str(), "rb");
	while (fgets(buf,1000,fp)!=NULL) { 
        string str = buf + 1;
        vector<string> vc = StrSplit(str, "\t /:");

        Rule *rule = (Rule*)malloc(sizeof(Rule));
        memset(rule, 0, sizeof(Rule));
        // src_ip dst_ip
        for (int i = 0; i < 2; ++i) {
            rule->range[i][0] = GetIp(vc[i * 2]);
            rule->prefix_len[i] = atoi(vc[i * 2 + 1].c_str());
            uint32_t mask = (1LL << (32 - rule->prefix_len[i])) - 1;
            rule->range[i][0] -= rule->range[i][0] & mask;
			rule->range[i][1] = rule->range[i][0] | mask;
        }
        // src_port dst_port
        rule->range[2][0] = atoi(vc[4].c_str());
        rule->range[2][1] = atoi(vc[5].c_str());
        rule->range[3][0] = atoi(vc[6].c_str());
        rule->range[3][1] = atoi(vc[7].c_str());
        // protocol
        rule->range[4][0] = strtoul(vc[8].c_str(), NULL, 0);
        uint32_t mask = 255 - strtoul(vc[9].c_str(), NULL, 0);
		rule->range[4][0] -= rule->range[4][0] & mask;
		rule->range[4][1] = rule->range[4][0] | mask;
		rule->prefix_len[4] = Count1(255 - mask);
        // priority
        rule->priority = priority--;

        rules.push_back(rule);
        ++rules_num;
        //printf("%s", str.c_str());
        //for (int i = 0; i < vc.size(); ++i) printf("%d : %s\n", i, vc[i].c_str());
        //PrintRule(rule);
        // if (rules_num >= 3) break;
    }
    if (rules_shuffle > 0) {
        random_shuffle(rules.begin(),rules.end());
    }
    //rules.resize(10);
	//printf("rules_num = %ld\n", rules.size());
    return rules;
}

int port_bit_mask[17][2] ={ { 0, 0xffff },
    { 0x1, 0xfffe }, { 0x3, 0xfffc }, { 0x7, 0xfff8 }, { 0xf, 0xfff0 },
    { 0x1f, 0xffe0 }, { 0x3f, 0xffc0 }, { 0x7f, 0xff80 },
    { 0xff, 0xff00 }, { 0x1ff, 0xfe00 }, { 0x3ff, 0xfc00 },
    { 0x7ff, 0xf800 }, { 0xfff, 0xf000 }, { 0x1fff, 0xe000 },
    { 0x3fff, 0xc000 }, { 0x7fff, 0x8000 }, { 0xffff, 0 }
};

struct PrefixRange {
    uint32_t low;
    uint32_t high;
    uint32_t prefix_len;
};

vector<PrefixRange> GetPortMask(int port_start, int port_end) {
    vector<PrefixRange> prefix;
    PrefixRange prefix_range;
    for (int pos = port_start; pos <= port_end;) {
        int i;
        for (i = 1; i < 17; ++i) {
            int pos2 = pos + port_bit_mask[i][0];
            if (pos2 > port_end || (pos & port_bit_mask[i][1]) != (pos2 & port_bit_mask[i][1]))
                break;
        }
        --i;
        prefix_range.low = pos;
        prefix_range.high = pos + port_bit_mask[i][0];
        prefix_range.prefix_len = 16 - i;
        prefix.push_back(prefix_range);
        pos += (port_bit_mask[i][0] + 1);
    }
    return prefix;
}

vector<Rule*> RulesPortPrefix(vector<Rule*> &rules, bool free_rules) {
    vector<Rule*> prefix_rules;
    int rules_num = rules.size();
    for (int i = 0; i < rules_num; ++i) {
        vector<PrefixRange> src_port_prefix = GetPortMask(rules[i]->range[2][0], rules[i]->range[2][1]);
        vector<PrefixRange> dst_port_prefix = GetPortMask(rules[i]->range[3][0], rules[i]->range[3][1]);
        int src_port_prefix_num = src_port_prefix.size();
        int dst_port_prefix_num = dst_port_prefix.size();
        for (int j = 0; j < src_port_prefix_num; ++j)
            for (int k = 0; k < dst_port_prefix_num; ++k) {
                Rule *rule = (Rule*)malloc(sizeof(Rule));
                *rule = *rules[i];

                rule->range[2][0] = src_port_prefix[j].low;
                rule->range[2][1] = src_port_prefix[j].high;
                rule->prefix_len[2] = src_port_prefix[j].prefix_len;

                rule->range[3][0] = dst_port_prefix[k].low;
                rule->range[3][1] = dst_port_prefix[k].high;
                rule->prefix_len[3] = dst_port_prefix[k].prefix_len;

                prefix_rules.push_back(rule);
            }
    }
    if (free_rules)
        FreeRules(rules);
    return prefix_rules;
}

vector<Rule*> UniqueRules(vector<Rule*> &rules) {
    vector<Rule*> unique_rules;
    map<Rule, int> match;
    int rules_num = rules.size();
    Rule rule;
    for (int i = 0; i < rules_num; ++i) {
        rule = *rules[i];
        rule.priority = 0;
        if (match.find(rule) == match.end()) {
            match[rule] = 1;
            unique_rules.push_back(rules[i]);
        }
    }
    // printf("UniqueRules %ld to %ld\n", rules.size(), unique_rules.size());
    match.clear();
    return unique_rules;
}

vector<Rule*> UniqueRulesIgnoreProtocol(vector<Rule*> &rules) {
    vector<Rule*> unique_rules;
    map<Rule, int> match;
    int rules_num = rules.size();
    Rule rule;
    for (int i = 0; i < rules_num; ++i) {
        rule = *rules[i];
        rule.range[4][0] = 0;
        rule.range[4][1] = 0;
        rule.prefix_len[4] = 0;
        rule.priority = 0;
        if (match.find(rule) == match.end()) {
            match[rule] = 1;
            unique_rules.push_back(rules[i]);
        }
    }
    // printf("UniqueRulesIgnoreProtocol %ld to %ld\n", rules.size(), unique_rules.size());
    match.clear();
    return unique_rules;
}

void PrintRules(vector<Rule*> &rules, string rules_file, bool print_priority) {
    int rules_num = rules.size();
    FILE *fp = fopen(rules_file.c_str(), "w");
    for (int i = 0; i < rules_num; ++i) {
        fprintf(fp, "@");
        for (int j = 0; j < 2; ++j) {
            fprintf(fp, "%s/%d\t", GetIpStr(rules[i]->range[j][0]).c_str(), rules[i]->prefix_len[j]);
        }
        for (int j = 2; j < 4; ++j)
            fprintf(fp, "%d : %d\t", rules[i]->range[j][0], rules[i]->range[j][1]);
        if (rules[i]->range[4][0] == rules[i]->range[4][1])
            fprintf(fp, "0x%02x/0xFF\t", rules[i]->range[4][0]);
        else
            fprintf(fp, "0x%02x/0x00\t", rules[i]->range[4][0]);
        fprintf(fp, "0x0000/0x0000\t");
        if (print_priority)
            fprintf(fp, "%d\t", rules[i]->priority);
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void PrintRulesPrefix(vector<Rule*> &rules, string rules_file, bool print_priority) {
    int rules_num = rules.size();
    FILE *fp = fopen(rules_file.c_str(), "w");
    for (int i = 0; i < rules_num; ++i) {
        fprintf(fp, "@");
        for (int j = 0; j < 2; ++j) {
            fprintf(fp, "%s/%d\t", GetIpStr(rules[i]->range[j][0]).c_str(), rules[i]->prefix_len[j]);
        }
        for (int j = 2; j < 4; ++j)
            fprintf(fp, "0x%04x/%d\t", rules[i]->range[j][0], rules[i]->prefix_len[j]);
        fprintf(fp, "0x%02x/%d\t", rules[i]->range[4][0], rules[i]->prefix_len[4]);
        // fprintf(fp, "0x0000/0x0000\t");
        if (print_priority)
            fprintf(fp, "%d\t", rules[i]->priority);
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void PrintRulesMegaFlow(vector<Rule> &rules, string rules_file, bool print_priority) {
    int rules_num = rules.size();
    FILE *fp = fopen(rules_file.c_str(), "w");
    for (int i = 0; i < rules_num; ++i) {
        fprintf(fp, "@");
        for (int j = 0; j < 2; ++j) {
            fprintf(fp, "%s/%d\t", GetIpStr(rules[i].range[j][0]).c_str(), rules[i].prefix_len[j]);
        }
        for (int j = 2; j < 4; ++j)
            fprintf(fp, "0x%04x/%d\t", rules[i].range[j][0], rules[i].prefix_len[j]);
        
        if (print_priority)
            fprintf(fp, "%d\t", rules[i].priority);
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void CheckMegaFlowRules(vector<Rule> &rules) {
    int rules_num = rules.size();
    for (int i = 0; i < rules_num; ++i) {
        for (int j = i + 1; j < rules_num; ++j) {
            bool contain = true;
            for (int k = 0; k < 4; ++k) {
                int min_prefix_len = min(rules[i].prefix_len[k], rules[j].prefix_len[k]);
                if (k == 2 || k == 3)
                    min_prefix_len += 16;
                if (rules[i].range[k][0] >> (32 - min_prefix_len) != 
                    rules[j].range[k][0] >> (32 - min_prefix_len))
                    contain = false;
            }
            if (contain)
                printf("contain %d %d\n", i, j);
        }
    }
}

Rule rule_empty;
void MegaFlowRules(vector<Rule*> &rules, vector<Trace*> &traces, string rules_file) {
    Trie src_ip_trie;
    Trie dst_ip_trie;
    Trie src_port_trie;
    Trie dst_port_trie;
    src_ip_trie.Init();
    dst_ip_trie.Init();
    src_port_trie.Init();
    dst_port_trie.Init();
    int rules_num = rules.size();
    int traces_num = traces.size();
    for (int i = 0; i < rules_num; ++i) {
        src_ip_trie.InsertRule(rules[i]->range[0][0], rules[i]->prefix_len[0], rules[i]->priority);
        dst_ip_trie.InsertRule(rules[i]->range[1][0], rules[i]->prefix_len[1], rules[i]->priority);
        src_port_trie.InsertRule(rules[i]->range[2][0], rules[i]->prefix_len[2] + 16, rules[i]->priority);
        dst_port_trie.InsertRule(rules[i]->range[3][0], rules[i]->prefix_len[3] + 16, rules[i]->priority);
        // printf("%s\n", GetIpStr(rules[i]->range[0][0]).c_str());
    }
    vector<Rule> megaflow_rules;
    map<Rule, int> match;
    // for (int i = 0; i < rules_num; ++i) {
    //     uint32_t src_ip = rules[i]->range[0][0];
    //     uint32_t dst_ip = rules[i]->range[1][0];
    //     uint32_t dst_port = rules[i]->range[3][0];
    for (int i = 0; i < traces_num; ++i) {
        // printf("%d\n", i);
        uint32_t src_ip = traces[i]->key[0];
        uint32_t dst_ip = traces[i]->key[1];
        uint32_t src_port = traces[i]->key[2];
        uint32_t dst_port = traces[i]->key[3];


        int src_ip_len = src_ip_trie.Lookup(src_ip, 32);
        int dst_ip_len = dst_ip_trie.Lookup(dst_ip, 32);
        int src_port_len = src_port_trie.Lookup(src_port, 32);
        int dst_port_len = dst_port_trie.Lookup(dst_port, 32);
        // printf("%d %d %d\n", src_ip_len, dst_ip_len, dst_port_len - 16);
        // printf("%d %d\n", i + 1, prefix_len);

        Rule rule = rule_empty;
        rule.range[0][0] = (src_ip >> (32 - src_ip_len)) << (32 - src_ip_len);
        rule.prefix_len[0] = src_ip_len;
        rule.range[1][0] = (dst_ip >> (32 - dst_ip_len)) << (32 - dst_ip_len);
        rule.prefix_len[1] = dst_ip_len;
        rule.range[2][0] = (src_port >> (32 - src_port_len)) << (32 - src_port_len);
        rule.prefix_len[2] = src_port_len - 16;
        rule.range[3][0] = (dst_port >> (32 - dst_port_len)) << (32 - dst_port_len);
        rule.prefix_len[3] = dst_port_len - 16;

        if (match.find(rule) == match.end()) {
            match[rule] = 1;
            // rule.priority = rules[i]->priority;
            megaflow_rules.push_back(rule);
        }
    }
    // printf("CheckMegaFlowRules\n");
    CheckMegaFlowRules(megaflow_rules);
    // printf("PrintRulesMegaFlow\n");
    PrintRulesMegaFlow(megaflow_rules, rules_file, false);
    // exit(1);
}

void GenerateTSEMegaflowRules() {
    // printf("GenerateTSEMegaflowRules\n");

    vector<Rule> megaflow_rules;
    for (int src_ip_len = 32; src_ip_len >= 1; --src_ip_len)
        for (int dst_ip_len = 32; dst_ip_len >= 1; --dst_ip_len)
            for (int src_port_len = 16; src_port_len >= 1; --src_port_len)
                for (int dst_port_len = 16; dst_port_len >= 1; --dst_port_len) {
                    Rule rule = rule_empty;
                    rule.range[0][0] = 1ULL << (32 - src_ip_len);
                    rule.prefix_len[0] = src_ip_len;
                    rule.range[1][0] = 1ULL << (32 - dst_ip_len);
                    rule.prefix_len[1] = dst_ip_len;
                    rule.range[2][0] = 1ULL << (16 - src_port_len);
                    rule.prefix_len[2] = src_port_len;
                    rule.range[3][0] = 1ULL << (16 - dst_port_len);
                    rule.prefix_len[3] = dst_port_len;
                    megaflow_rules.push_back(rule);
                }
    // printf("PrintRulesMegaFlow\n");

    PrintRulesMegaFlow(megaflow_rules, "data/tse_megaflow_rules", false);
    random_shuffle(megaflow_rules.begin(), megaflow_rules.end());
    PrintRulesMegaFlow(megaflow_rules, "data/tse_megaflow_rules_shuffle", false);
}

void PrintMegaFlowPacket(string rules_file) {
    FILE *fp = fopen(rules_file.c_str(), "w");
    for (uint32_t src_ip = 0xFFFFFFFFU; src_ip > 0; src_ip >>= 1)
        for (uint32_t dst_ip = 0xFFFFFFFFU; dst_ip > 0; dst_ip >>= 1)
            for (uint16_t src_port = 0xFFFF; src_port > 0; src_port >>= 1)
                for (uint16_t dst_port = 0xFFFF; dst_port > 0; dst_port >>= 1)
                    fprintf(fp, "0x%08x 0x%08x 0x%04x 0x%04x\n", src_ip, dst_ip, src_port, dst_port);
    fclose(fp);
}

int FreeRules(vector<Rule*> &rules) {
    int rules_num = rules.size();
    for (int i = 0; i < rules_num; ++i)
        free(rules[i]);
    rules.clear();
    return 0;
}

vector<Trace*> ReadTraces(string traces_file) {
    vector<Trace*> traces;
	int traces_num = 0;
    char buf[1025];

	FILE *fp = fopen(traces_file.c_str(), "rb");
	if (!fp)
		printf("Cannot open the file %s\n", traces_file.c_str());
	while (fgets(buf,1000,fp)!=NULL) { 
        string str = buf;
        vector<string> vc = StrSplit(str, "\t");
        Trace *trace = (Trace*)malloc(sizeof(Trace));
        for (int i = 0; i < 5; ++i)
            trace->key[i] = atoi(vc[i].c_str());

        traces.push_back(trace);
        ++traces_num;
        //for (int i = 0; i < 5; ++i) printf("%u ", trace.key[i]); printf("\n");
        //if (traces_num >= 3) break;
    }
	//printf("traces_num = %ld\n", traces.size());
    return traces;
}

vector<Trace*> GenerateTraces(vector<Rule*> &rules) {
    int rules_num = rules.size();
    vector<Trace*> traces;
    for (int i = 0; i < rules_num; ++i) {
        Trace *trace = (Trace*)malloc(sizeof(Trace));
        for (int j = 0; j < 5; ++j)
            trace->key[j] = rules[i]->range[j][0];
        traces.push_back(trace);
    }
    return traces;
}

int FreeTraces(vector<Trace*> &traces) {
    int traces_num = traces.size();
    for (int i = 0; i < traces_num; ++i)
        free(traces[i]);
    traces.clear();
    return 0;
}

bool SameTrace(Trace *trace1, Trace *trace2) {
	for (int i = 0; i < 5; i++)
		if (trace1->key[i] != trace2->key[i])
			return false;
	return true;
}

vector<int> GenerateAns(vector<Rule*> &rules, vector<Trace*> &traces, int force_test) {
    vector<int> ans;
    if (force_test == 0)
        return ans;
    int rules_num = rules.size();
    int traces_num = traces.size();
	for (int i = 0; i < traces_num; ++i) {
		if (i > 0 && SameTrace(traces[i - 1], traces[i])) {
            ans.push_back(ans[i - 1]);
			continue;
		}
        int priority = 0;
		for (int j = 0; j < rules_num; ++j) {
			if (force_test == 2 && j % 4 == 0) continue;
			if (rules[j]->priority > priority && MatchRuleTrace(rules[j], traces[i])) {
				priority = rules[j]->priority;
			}
		}
        ans.push_back(priority);
	    //printf("%d %d\n", i, priority);
	}
	//printf("ans_num = %ld\n", ans.size());
    return ans;
}

void PrintAns(string output_file, vector<int> &ans) {
    int ans_num = ans.size();
	FILE *fp = fopen(output_file.c_str(), "wb");
	if (!fp)
		printf("Cannot open the file %s\n", output_file.c_str());
    for (int i = 0; i < ans_num; ++i)
        fprintf(fp, "%d\n", ans[i]);
}

vector<Rule*> GenerateOneMatchRules(vector<Rule*> &rules) {
    int num = rules.size();
    vector<Trace*> traces = GenerateTraces(rules);
    vector<int> ans = GenerateAns(rules, traces, 1);
    
    vector<Rule*> new_rules;
    for (int i = 0; i < num; ++i)
        if (ans[i] == rules[i]->priority)
            new_rules.push_back(rules[i]);
    return new_rules;
}

uint16_t htons(uint16_t hostshort) {
    return ((hostshort & 255) << 8) | (hostshort >> 8);
}

uint32_t htonl(uint32_t hostlong) {
    return (htons(hostlong & 65535) << 16) | htons(hostlong >> 16);
}

uint16_t ntohs(uint16_t netshort) {
    return ((netshort & 255) << 8) | (netshort >> 8);
}

uint32_t ntohl(uint32_t netlong) {
    return (ntohs(netlong & 65535) << 16) | ntohs(netlong >> 16);
}