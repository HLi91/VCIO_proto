#include "regular_slab.h"

int RegularSlab::connect(const char * filepath) {
	int fd = shm_open(filepath, O_RDWR, S_IRWXU);
	if (fd == -1) return -1;
	size_t filesize = lseek(fd, 0, SEEK_END);
	if (filesize < 0) return -1;

	return connect(fd, filesize);
}

int RegularSlab::connect(int fd, size_t filesize) {

	data_ = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	header_ = (RegularSlabHeader*)data_;
	int *bitmap_ptr = (int*)data_ + header_->bitmap_off;
	bitmap_ = new LinkBitMap(bitmap_ptr, header_->bit_map_size, header_->slab_id, header_->bitMapHeader);
	blocks_ = (DataBlock*)data_ + header_->block_off;
	return 0;

}

int RegularSlab::create(const char * filepath, size_t blocks, int slab_id) {
	int filesize = 0;
	filesize = sizeof(RegularSlabHeader) + blocks*sizeof(int) + blocks* sizeof(DataBlock);
	int fd = shm_open(filepath, O_RDWR | O_CREAT | O_EXCL, S_IRWXU);
	if (fd == -1) return -1;
	if (ftruncate(fd, filesize) != 0) return -1;
	if (lseek(fd, 0, SEEK_SET) != 0) return -1;

	data_ = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	header_ = (RegularSlabHeader*)data_;

	header_->bitmap_off = sizeof(RegularSlabHeader);
	header_->block_off = header_->bitmap_off + sizeof(int)*blocks;
	header_->in_used_block = 0;
	header_->total_block = blocks;
	header_->bit_map_size = blocks;
	header_->block_size = blocks;
	header_->slab_id = slab_id;

	memset(header_->bitMapHeader, 0, sizeof(int) * 8);
	header_->bitMapHeader[1] = -1;		//set head to -1;


	int *bitmap_ptr = (int*)((char*)data_ + header_->bitmap_off);
	bitmap_ = new LinkBitMap(bitmap_ptr, header_->bit_map_size, header_->slab_id, header_->bitMapHeader);
	blocks_ = (DataBlock*)((char*)data_ + header_->block_off);

	InitMutex();
	close(fd);

	return 0;
}

int RegularSlab::InitMutex() {
	
	pthread_mutexattr_t mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&(header_->m), &mattr);
	return 0;
}


int RegularSlab::MetadataLock() {
	pthread_mutex_lock(&(header_->m));
	return 0;
}
int RegularSlab::MetadataUnlock() {
	pthread_mutex_unlock(&(header_->m));
	return 0;
}

DataBlock * RegularSlab::operator[] (const BlockInfo_t& index) {
	if (index.slab_index != header_->slab_id)
		return NULL;
	if (index.block_index >= header_->block_size) {
		return NULL;
	}
	return &blocks_[index.block_index];
}

BitMap* RegularSlab::GetBitMap() {
	return bitmap_;
}


vector<BlockInfo_t> LinkBitMap::GetBlocks(int n){
	vector<BlockInfo_t> ret;
	for (int i = 0; i < n; i++) {
		if (header_->head != -1) {
			ret.push_back(BlockInfo_t(slab_id_, header_->head));
			header_->head = data_[header_->head];
			continue;
		}
		if (header_->pos < size_) {
			ret.push_back(BlockInfo_t(slab_id_, header_->pos));
			header_->pos++;
			continue;
		}
		return ret;
	}
	return ret;
}
int LinkBitMap::FreeBlock(BlockInfo_t & index) {
	if (index.slab_index != slab_id_)
		return -1;
	data_[index.block_index] = header_->head;
	header_->head = index.block_index;
}
int LinkBitMap::FreeBlocks(vector<BlockInfo_t> & list) {
	for (auto & it : list) {
		if (FreeBlock(it) == -1) {
			return -1;
		}
	}
	return 0;
}