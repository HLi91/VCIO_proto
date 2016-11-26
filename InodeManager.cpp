#include "InodeManager.h"

InodeManager::InodeManager(FixSlab * first_slab, RegularSlabManager* reg_slab_manager) {
	first_slab_ = first_slab;
	info_ = first_slab_->GetInodeInfo();
	void * inode;
	inode_size_ = first_slab_->GetInodeRegion(inode);
	inode_ = (InodeEntry*)inode;
	reg_slab_manager_ = reg_slab_manager;
}
int InodeManager::CreateInode(int & new_node) {

	first_slab_->Lock();
	if (info_->Inode_free_list != NULL) {
		new_node = (InodeEntry*)info_->Inode_free_list - inode_;
		info_->Inode_free_list = ((InodeEntry*)(info_->Inode_free_list))->next_;
		inode_[new_node].next_ = NULL;
	}
	else if (inode_size_ > info_->cur_inode_index) {
		new_node = info_->cur_inode_index;
		info_->cur_inode_index++;
	}
	else {
		first_slab_->Unlock();
		return -1;
	}

	first_slab_->Unlock();
	InodeInit(new_node);
	return 0;
}
int InodeManager::DeleteInode(int inode_id) {
	first_slab_->Lock();
	if (info_->cur_inode_index < inode_id) {
		first_slab_->Unlock();
		return -1;
	}
	InodeErase(inode_id);
	inode_[inode_id].next_ = (InodeEntry*)info_->Inode_free_list;
	info_->Inode_free_list = &inode_[inode_id];
	inode_[inode_id].status_ = FILEUNINIT;

	first_slab_->Unlock();
}

int InodeManager::ScheduleDeleteInode(int inode_id) {
	first_slab_->Lock();
	if (info_->cur_inode_index < inode_id) {
		first_slab_->Unlock();
		return -1;
	}
	if (inode_[inode_id].status_ != FILENORMAL) {
		std::cout << "ERROR: cannot schedule delete, other operation already scheduled" << std::endl;
		first_slab_->Unlock();
		return -1;

	}
	inode_[inode_id].status_ = FILEDELETE;


	first_slab_->Unlock();
	return 0;
}


int InodeManager::Open(int inode_id) {
	Lock(inode_id);
	inode_[inode_id].reference_count_++;
	Unlock(inode_id);
}
int InodeManager::Close(int inode_id) {
	Lock(inode_id);
	inode_[inode_id].reference_count_--;
	if (inode_[inode_id].reference_count_ == 0) {
		switch (inode_[inode_id].status_)
		{
		case FILEDELETE:
			DeleteInode(inode_id);
			//lock already destroyed
			return 0;
			break;
		case FILEMOVE:
			break;

		default:
			break;
		}
	}
	Unlock(inode_id);
}
int InodeManager::ReadBlock(int inode_id, int block_id, void* ptr) {
	Lock(inode_id);
	int ret = ReadBlock_impl(inode_id, block_id, ptr, 0, BLOCKSIZE);
	Unlock(inode_id);
	return ret;
}
int InodeManager::WriteBlock(int inode_id, int block_id, int size, const void* ptr, int relative_off) {
	Lock(inode_id);
	int ret = WriteBlock_impl(inode_id, block_id, ptr, relative_off, size);
	Unlock(inode_id);
	return ret;
}
int InodeManager::Read(int inode_id, size_t offset, size_t size, void * ptr) {
	Lock(inode_id);
	int block_id = offset / BLOCKSIZE;
	int relative_off = offset % BLOCKSIZE;
	int size_to_read = size;
	void * ptr_to_read = ptr;
	while (size_to_read != 0) {
		int ret = ReadBlock_impl(inode_id, block_id, ptr_to_read, relative_off, size_to_read);
		if (ret == -1) {
			Unlock(inode_id);
			return size - size_to_read;
		}
		relative_off = 0;
		size_to_read -= ret;
		ptr_to_read = (char*)ptr_to_read + ret;
		block_id++;
	}
	Unlock(inode_id);
	return size;
}
int InodeManager::Write(int inode_id, size_t offset, size_t size, const void * ptr){
	Lock(inode_id);
	int block_id = offset / BLOCKSIZE;
	int relative_off = offset % BLOCKSIZE;
	int size_to_write = size;
	const void * ptr_to_write = ptr;
	int request_blocks = size / BLOCKSIZE;
	AppendBlock(inode_id, inode_[inode_id].block_count_, request_blocks);
	while (size_to_write != 0) {
		int ret = WriteBlock_impl(inode_id, block_id, ptr_to_write, relative_off, size_to_write);
		if (ret == -1) {
			Unlock(inode_id);
			return size - size_to_write;
		}
		relative_off = 0;
		size_to_write -= ret;
		ptr_to_write = (char*)ptr_to_write + ret;
		block_id++;
	}
	Unlock(inode_id);
	return size;

}

int InodeManager::ReadBlock_impl(int inode_id, int block_id, void* ptr, int relative_off, int size) {

	if (block_id >= inode_[inode_id].block_count_) {
		
		return -1;
	}
	if (block_id < 16) {
		DataBlock* src = reg_slab_manager_->GetBlock(inode_[inode_id].direct[block_id]);
		//size = size < src->last_bit ? size : src->last_bit;			//todo: not right
		if (size + relative_off > src->last_bit) {
			//cout << "size" << size << endl;
			//cout << relative_off << endl << src->last_bit << endl;
			size = src->last_bit - relative_off;
			//cout << size << endl;
			
		}
		memcpy(ptr, src->data + relative_off, size);

		return size;
	}
	block_id -= 16;
	if (block_id < BLOCKSIZE / sizeof(BlockInfo_t)) {
		DataBlock* second = reg_slab_manager_->GetBlock(inode_[inode_id].second);
		BlockInfo_t* list = (BlockInfo_t*)(second->data);
		DataBlock* src = reg_slab_manager_->GetBlock(list[block_id]);
		if (size + relative_off > src->last_bit) {
			//cout << "size" << size << endl;
			//cout << size << endl;
			//cout << relative_off << endl << src->last_bit << endl;
			size = src->last_bit - relative_off;
			//cout << size << endl;
			
		}
		memcpy(ptr, src->data, size);

		return size;
	}
	block_id -= BLOCKSIZE / sizeof(BlockInfo_t);
	DataBlock* third = reg_slab_manager_->GetBlock(inode_[inode_id].third);
	BlockInfo_t* third_list = (BlockInfo_t*)(third->data);
	DataBlock* second = reg_slab_manager_->GetBlock(third_list[block_id / (BLOCKSIZE / sizeof(BlockInfo_t))]);
	BlockInfo_t* second_list = (BlockInfo_t*)(second->data);
	DataBlock* first = reg_slab_manager_->GetBlock(second_list[block_id % (BLOCKSIZE / sizeof(BlockInfo_t))]);
	if (size + relative_off > first->last_bit) {
		//cout << "size" << size << endl;
		//cout << size << endl;
		//cout << relative_off << endl << first->last_bit << endl;
		size = first->last_bit - relative_off;
		//cout << size << endl;
		
	}
	memcpy(ptr, first->data, size);

	return size;
}
int InodeManager::WriteBlock_impl(int inode_id, int block_id, const void* ptr, int relative_off, int size) {
	
	if (block_id >= inode_[inode_id].block_count_) {
		
		if (AppendBlock(inode_id, block_id, block_id + 1 - inode_[inode_id].block_count_) == -1) {
			cout << "ERROR: append block fail" << endl;
			return -1;
		}
	}
	if (block_id < 16) {
		DataBlock* src = reg_slab_manager_->GetBlock(inode_[inode_id].direct[block_id]);

		size = relative_off + size < BLOCKSIZE ? size : BLOCKSIZE - relative_off;
		memcpy(src->data + relative_off, ptr, size);
		
		src->last_bit = size + relative_off;
		inode_[inode_id].filesize_ += size;
		return size;
	}
	block_id -= 16;
	if (block_id < BLOCKSIZE / sizeof(BlockInfo_t)) {
		DataBlock* second = reg_slab_manager_->GetBlock(inode_[inode_id].second);
		BlockInfo_t* list = (BlockInfo_t*)(second->data);
		DataBlock* src = reg_slab_manager_->GetBlock(list[block_id]);
		size = relative_off + size < BLOCKSIZE ? size : BLOCKSIZE - relative_off;
		memcpy(src->data + relative_off, ptr, size);
		src->last_bit = size + relative_off;
		inode_[inode_id].filesize_ += size;
		return size;
	}
	block_id -= BLOCKSIZE / sizeof(BlockInfo_t);
	DataBlock* third = reg_slab_manager_->GetBlock(inode_[inode_id].third);
	BlockInfo_t* third_list = (BlockInfo_t*)(third->data);
	DataBlock* second = reg_slab_manager_->GetBlock(third_list[block_id / (BLOCKSIZE / sizeof(BlockInfo_t))]);
	BlockInfo_t* second_list = (BlockInfo_t*)(second->data);
	DataBlock* first = reg_slab_manager_->GetBlock(second_list[block_id % (BLOCKSIZE / sizeof(BlockInfo_t))]);
	size = relative_off + size < BLOCKSIZE ? size : BLOCKSIZE - relative_off;
	memcpy(first->data + relative_off, ptr, size);
	first->last_bit = size + relative_off;
	inode_[inode_id].filesize_ += size;
	return size;
}
size_t InodeManager::size() {


}
int InodeManager::InodeInit(int inode_id) {
	pthread_mutexattr_t mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&(inode_[inode_id].m), &mattr);
	inode_[inode_id].block_count_ = 0;
	memset(inode_[inode_id].direct, 0, sizeof(BlockInfo_t) * 16);
	inode_[inode_id].second.block_index = 0;
	inode_[inode_id].second.slab_index = 0;
	inode_[inode_id].filesize_ = 0;
	inode_[inode_id].reference_count_ = 0;
	inode_[inode_id].status_ = 0;
	inode_[inode_id].third.block_index = 0;
	inode_[inode_id].third.slab_index = 0;
}
int InodeManager::InodeErase(int inode_id) {
	pthread_mutex_destroy(&(inode_[inode_id].m));
}
int InodeManager::Lock(int inode_id) {
	pthread_mutex_lock(&(inode_[inode_id].m));
	return 0;
}
int InodeManager::Unlock(int inode_id) {
	pthread_mutex_unlock(&(inode_[inode_id].m));
	return 0;
}

int InodeManager::AppendBlock(int inode_id, int start_block_id, int num) {

	int request_size = AppendBlockNums(inode_id, start_block_id, num);
	if (request_size == 0) return 0;
	vector<BlockInfo_t> block_list = reg_slab_manager_->RequestEmptyBlock(request_size, inode_id);
	if (block_list.size() == 0) return -1;
	int cur_free_block = 0;
	int end_block_id = start_block_id + num;
	
	if (start_block_id < inode_[inode_id].block_count_)
		start_block_id = inode_[inode_id].block_count_;
	int increase = end_block_id - start_block_id;
	if (start_block_id >= 0 && start_block_id < 16) {
		for (int i = start_block_id; i < (end_block_id < 16 ? end_block_id : 16); i++) {
			inode_[inode_id].direct[i] = block_list[cur_free_block++];
		}
		
	}

	start_block_id -= 16;
	end_block_id -= 16;
	if (start_block_id >= 0 && start_block_id < (BLOCKSIZE / sizeof(BlockInfo_t))) {
		if (start_block_id == 0) {
			inode_[inode_id].second = block_list[cur_free_block++];
		}
		for (int i = start_block_id; i < (end_block_id < BLOCKSIZE / sizeof(BlockInfo_t) ? end_block_id : BLOCKSIZE / sizeof(BlockInfo_t)); i++) {
			//inode_[inode_id].second[i] = block_list[cur_free_block++];
			DataBlock* second = reg_slab_manager_->GetBlock(inode_[inode_id].second);
			BlockInfo_t* second_list = (BlockInfo_t*)(second->data);
			second_list[i] = block_list[cur_free_block++];
		}
		

	}
	start_block_id -= (BLOCKSIZE / sizeof(BlockInfo_t));
	end_block_id -= BLOCKSIZE / sizeof(BlockInfo_t);
	if (start_block_id >= 0) {
		if (start_block_id == 0) {
			inode_[inode_id].third = block_list[cur_free_block++];
		}
		DataBlock* third = reg_slab_manager_->GetBlock(inode_[inode_id].third);
		BlockInfo_t* third_list = (BlockInfo_t*)(third->data);
		
		
		for (int i = start_block_id; i < end_block_id; i++) {
			if (i % (BLOCKSIZE / sizeof(BlockInfo_t)) == 0) {
				third_list[i / (BLOCKSIZE / sizeof(BlockInfo_t))] = block_list[cur_free_block++];
			}
			BlockInfo_t* second_list = (BlockInfo_t*)reg_slab_manager_->GetBlock(third_list[i / (BLOCKSIZE / sizeof(BlockInfo_t))])->data;
			//this is so terrible

			second_list[i % (BLOCKSIZE / sizeof(BlockInfo_t))] = block_list[cur_free_block++];
		}
	}

	if (cur_free_block != block_list.size()) {
		while (1);
	}
	inode_[inode_id].block_count_ += increase;
	return 0;

}

int InodeManager::AppendBlockNums(int inode_id, int start_block_id, int num) {
	int end_block_id = start_block_id + num;
	if (start_block_id < inode_[inode_id].block_count_)
		start_block_id = inode_[inode_id].block_count_;
	int requestSize = 0;
	if (start_block_id >= 0 && start_block_id < 16) {
		requestSize += end_block_id < 16 ? end_block_id - start_block_id : 16 - start_block_id;
		//vector<BlockInfo_t> block_list = reg_slab_manager_->RequestEmptyBlock(requestSize);
		//for (int i = 0; i < block_list.size(); i++) {
		//inode_[inode_id].direct[start_block_id + i] = block_list[i];
		//}
	}
	start_block_id -= 16;
	end_block_id -= 16;
	if (start_block_id >= 0 && start_block_id < BLOCKSIZE / sizeof(BlockInfo_t)) {
		if (start_block_id == 0)
			requestSize++;
		requestSize += end_block_id < BLOCKSIZE / sizeof(BlockInfo_t) ? end_block_id - start_block_id : BLOCKSIZE / sizeof(BlockInfo_t) - start_block_id;

	}
	start_block_id -= BLOCKSIZE / sizeof(BlockInfo_t);
	end_block_id -=  BLOCKSIZE / sizeof(BlockInfo_t);
	if (start_block_id >= 0) {
		if (start_block_id == 0)
			requestSize++;
		if (start_block_id % (BLOCKSIZE / sizeof(BlockInfo_t)) == 0)
			requestSize++;
		requestSize += (end_block_id - start_block_id) / (BLOCKSIZE / sizeof(BlockInfo_t));
		requestSize += (end_block_id - start_block_id);
	}
	return requestSize;
}


size_t InodeManager::file_size(int inode_id) {

	return inode_[inode_id].filesize_;
}