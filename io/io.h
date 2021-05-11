#ifndef IO_H
#define IO_H

#include "../elementary.h"

//#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include "trie.h"

using namespace std;

vector<string> StrSplit(const string& str, const string& pattern);
int Count1(uint64_t num);
uint32_t GetIp(string str);

vector<Rule*> ReadRules(string rules_file, int rules_shuffle);
vector<Rule*> RulesPortPrefix(vector<Rule*> &rules, bool free_rules);
vector<Rule*> UniqueRules(vector<Rule*> &rules);
vector<Rule*> UniqueRulesIgnoreProtocol(vector<Rule*> &rules);
void PrintRules(vector<Rule*> &rules, string rules_file, bool print_priority);
void PrintRulesPrefix(vector<Rule*> &rules, string rules_file, bool print_priority);
void MegaFlowRules(vector<Rule*> &rules, vector<Trace*> &traces, string rules_file);
void GenerateTSEMegaflowRules();
void PrintMegaFlowPacket(string rules_file);
int FreeRules(vector<Rule*> &rules);
vector<Trace*> ReadTraces(string traces_file);
vector<Trace*> GenerateTraces(vector<Rule*> &rules);
int FreeTraces(vector<Trace*> &traces);
vector<int> GenerateAns(vector<Rule*> &rules, vector<Trace*> &traces, int force_test);
void PrintAns(string output_file, vector<int> &ans);
vector<Rule*> GenerateOneMatchRules(vector<Rule*> &rules);

uint16_t htons(uint16_t hostshort);
uint32_t htonl(uint32_t hostlong);
uint16_t ntohs(uint16_t netshort);
uint32_t ntohl(uint32_t netlong);

#endif