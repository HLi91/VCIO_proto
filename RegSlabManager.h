#ifndef REG_SLAB_MANAGER_H_
#define REG_SLAB_MANAGER_H_

#include <vector>
#include <string>
#include "regular_slab.h"
#include "fix_slab.h"

using namespace std;

#define SLAB_SIZE 4*1024


class RegularSlabManager {
public:
	RegularSlabManager(FixSlab * first_slab);
	DataBlock* GetBlock(BlockInfo_t &position);
	vector<BlockInfo_t> RequestEmptyBlock(int num, int node_id);
private:
	int UpdateSlabListLocked();
	int AddSlabLocked(int blocks);
private:
	FixSlab* first_slab_;
	vector<RegularSlab> slab_list_;
};

#endif // !REG_SLAB_MANAGER_H_
