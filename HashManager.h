#ifndef HASH_MANAGER_H_
#define HASH_MANAGER_H_

#include <string>
#include "fix_slab.h"


using namespace std;

/*this is a dummy hash table
 */
class HashManager {
public:
	HashManager(FixSlab* first_slab);
	int Insert(string key, int val);
	int Insert(int hashed_key, int val);
	int HashFunction(string key);
	int Erase(string key);
	int find(string key);
	int size();
	int& at(string key);
	int& operator[](string key);


private:
	FixSlab* first_slab_;
	HashEntry * hash_;
	int hash_size_;			//max
};
#endif // !HASH_MANAGER_H_
