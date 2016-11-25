#ifndef INODE_MANAGER_H_
#define INODE_MANAGER_H_

#include <math.h>
#include <iostream>
#include "fix_slab.h"
#include "inode.h"
#include "RegSlabManager.h"
class InodeManager {
public:
	InodeManager(FixSlab * first_slab, RegularSlabManager* reg_slab_manager);
	int CreateInode(int & new_node);
	int DeleteInode(int node_id);
	int ScheduleDeleteInode(int inode_id);
	int Open(int inode_id);
	int Close(int inode_id);
	int ReadBlock(int inode_id, int block_id, void* ptr);
	int WriteBlock(int inode_id, int block_id, int size, const void* ptr, int relative_off);
	
	int Read(int inode_id, size_t offset, size_t size, void * ptr);
	int Write(int inode_id, size_t offset, size_t size, const void * ptr);
	size_t file_size(int inode_id);
	size_t size();
	
private:
	int InodeInit(int inode_id);
	int InodeErase(int inode_id);
	int Lock(int inode_id);
	int Unlock(int inode_id);
	
	int ReadBlock_impl(int inode_id, int block_id, void* ptr, int relative_off, int size);
	int WriteBlock_impl(int inode_id, int block_id, const void* ptr, int relative_off, int size);
	int AppendBlock(int inode_id, int block_id, int num);
	int AppendBlockNums(int inode_id, int block_id, int num);

private:
	RegularSlabManager* reg_slab_manager_;
	FixSlab * first_slab_;
	struct InodeInfo* info_;
	InodeEntry* inode_;
	int inode_size_;			//max
};

#endif // !INODE_MANAGER_H_
