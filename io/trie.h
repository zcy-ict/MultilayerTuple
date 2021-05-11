#ifndef  TRIE_H
#define  TRIE_H

#include "../elementary.h"

#define INSERT 1
#define DELETE 2


using namespace std;

struct TrieNode {
    TrieNode *child[2];
    int priority;
    bool solid_node;

    TrieNode* Create(int _priority, bool _solid_node);
    int CountNum();
    void Free();
    void FreeAll();
};

class Trie{
public:
    int Init();
    
    int InsertRule(uint32_t ip, uint8_t prefix_len, int priority);
    int DeleteRule(uint32_t ip, uint8_t prefix_len);

    int Lookup(uint32_t ip, int max_prefix_len);

    uint64_t Memory();
    int Free();
    int Test(void *ptr);
// private:

    int Update(uint32_t ip, uint8_t prefix_len, int priority, int operation);

    TrieNode* root;
    int solid_node_num;

    TrieNode* pre_nodes[129];
    int pre_nodes_num;
};


#endif