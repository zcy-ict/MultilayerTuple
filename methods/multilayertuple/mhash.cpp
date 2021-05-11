#include "mhash.h"

using namespace std;

uint32_t HashKeys(uint32_t *keys, uint32_t keys_num) {
	uint32_t hash = keys[0];
	for (int i = 1; i < keys_num; ++i)
		hash = hash * 1000000007 + keys[i];
	return hash;
}