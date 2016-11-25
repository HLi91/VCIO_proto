#ifndef CIO_REG_SLAB_H
#define CIO_REG_SLAB_H

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <vector>
#include <stdio.h>
#include <string.h>
#include "islab.h"

using namespace std;

class RegularSlabHeader {
public:
	pthread_mutex_t m;
	int total_block;
	int in_used_block;
	size_t bitmap_off;
	size_t block_off;

	int slab_id;
	int bit_map_size;
	int block_size;

	int bitMapHeader[8];

};

class BitMap {
public:
	BitMap(int* bit_map, int size, int slab_id, int* header) : data_(bit_map), size_(size), slab_id_(slab_id) {}
	virtual vector<BlockInfo_t> GetBlocks(int n) = 0;
	virtual int FreeBlock(BlockInfo_t & index) = 0;
	virtual int FreeBlocks(vector<BlockInfo_t> & list) = 0;
protected:
	int * data_;
	int size_;
	int slab_id_;
};

class LinkedBitmapHeader {
public:
	int pos;
	int head;
};
class LinkBitMap : public BitMap {
public:
	LinkBitMap(int* bit_map, int size, int slab_id, int* header) : BitMap(bit_map, size, slab_id, header) { header_ = (LinkedBitmapHeader*)header; }
	vector<BlockInfo_t> GetBlocks(int n);
	int FreeBlock(BlockInfo_t & index);
	int FreeBlocks(vector<BlockInfo_t> & list);

private:
	LinkedBitmapHeader* header_;
};


class RegularSlab: public iSlab {
public: 
	int connect(const char * filepath);
	int connect(int fd, size_t filesize);
	int create(const char * filepath, size_t blocks, int slab_id);

	//I don't think this is necessary
	int MetadataLock();
	int MetadataUnlock();

	DataBlock * operator[] (const BlockInfo_t& index);

	BitMap* GetBitMap();
	RegularSlabHeader * header_;


	
private:
	int InitMutex();
private:
	
	//i am using integer map instead, just for simplicity
	BitMap* bitmap_;
	DataBlock* blocks_;

	
};

#endif