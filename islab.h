#ifndef ISLAB_H_
#define ISLAB_H_
#include <iostream>

#define BLOCKSIZE (256*1024)

class BlockInfo_t {
public:
	BlockInfo_t();
	BlockInfo_t(int slab, int block) : slab_index(slab), block_index(block) {}
	int slab_index;
	int block_index;
	
	
};

class DataBlock {
public:
	char data[BLOCKSIZE];
	int last_bit;
	//following field should only be accessed by RegSlabManager
	int inode_id;
};

class iSlab {
public:
	virtual int connect(const char * filepath) = 0;
protected:
	void * data_;
};

#endif