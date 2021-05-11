#include "trie.h"

using namespace std;

TrieNode* TrieNode::Create(int _priority, bool _solid_node) {
    TrieNode* trie_node = (TrieNode*)malloc(sizeof(TrieNode));
    trie_node->child[0] = NULL;
    trie_node->child[1] = NULL;
    trie_node->priority = _priority;
    trie_node->solid_node = _solid_node;
    return trie_node;
}

int TrieNode::CountNum() {
    if (this == NULL)
        return 0;
    return 1 + child[0]->CountNum() + child[1]->CountNum();
}

void TrieNode::Free() {
    if (this != NULL)
        free(this);
}

void TrieNode::FreeAll() {
    if (this == NULL)
        return;
    child[0]->FreeAll();
    child[1]->FreeAll();
    free(this);
}

int Trie::Init() {
    root = root->Create(0, true);
    solid_node_num = 1;
    return 0;
}

int Trie::Update(uint32_t ip, uint8_t prefix_len, int priority, int operation) {
    if (prefix_len == 0) {
        return 0;
        printf("prefix can not be 0\n");
        exit(1);
    }
    TrieNode *pre_node = NULL;
    TrieNode *node = root;
    pre_nodes[0] = node;
    pre_nodes_num = 1;
    for (int i = 0; i < prefix_len; ++i) {
        if (node->solid_node)
            pre_node = node;
        int bit = (ip >> (31 - i)) & 1;
        if (node->child[bit] == NULL)
            node->child[bit] = node->child[bit]->Create(0, false);
        node = node->child[bit];
        
        pre_nodes[pre_nodes_num] = node;
        ++pre_nodes_num;
    }

    if (operation == INSERT) {
        if (node->solid_node) {
            node->priority = max(node->priority, priority);
        } else
            node->priority = priority;
        node->solid_node = true;
    } else if (operation == DELETE) {
    }
    return 0;
}
     
int Trie::InsertRule(uint32_t ip, uint8_t prefix_len, int priority) {
    return Update(ip, prefix_len, priority, INSERT);
}

int Trie::DeleteRule(uint32_t ip, uint8_t prefix_len) {
    return Update(ip, prefix_len, 0, DELETE);
}

int Trie::Lookup(uint32_t ip, int max_prefix_len) {

    TrieNode *node = root;
    int index = 0;
    for (; index < max_prefix_len; ++index) {
        if (!node)
            return index;
        int bit = (ip >> (31 - index)) & 1;
        node = node->child[bit];
    }
    return index;
}

uint64_t Trie::Memory() {
    uint64_t size = sizeof(Trie);
    size += root->CountNum() * sizeof(TrieNode);
    return size;
}

int Trie::Free() {
    root->FreeAll();
    return 0;
}

int Trie::Test(void *ptr) {
    return 0;
}