#include "filedescriptor.h" 

int FileDescriptor::init(int inode_index) {
	
	clear();
	file_inode_ = inode_index;
}

int FileDescriptor::clear() {
	file_inode_ = -1;
	file_offset = 0;
	memset(buff_, 0, BLOCKSIZE);
	buff_start_position_ = 0;
	buff_cur_position_ = 0;
	block_id_logical = -1;
	dirty_ = 0;
}


FileDescriptorManager::~FileDescriptorManager() {
	m.lock();
	for (auto & it : free_) {
		delete it;
	}
	if (in_use_.size() != 0) {
		printf("ERROR: FileDescriptorManager destructor\n");
	}
	m.unlock();
}
//todo: get rid off mm allocation, use a pointer or something
int FileDescriptorManager::GetFileDescriptor(int inode_index) {
	inode_manager_->Open(inode_index);
	m.lock();
	FileDescriptor* cur;

	if (free_.size() == 0) {
		cur = new FileDescriptor();
		cur->init(inode_index);

	}
	else {
		cur = free_.back();
		free_.pop_back();
		cur->init(inode_index);
	}
	int ret = -1;
	if (in_use_holes_.size() != 0) {
		ret = in_use_holes_.back();
		in_use_holes_.pop_back();
		in_use_[ret] = cur;
	}
	else {
		ret = in_use_.size();
		in_use_.push_back(cur);
	}
	cur->fd_id_ = ret;
	m.unlock();
	return ret;
}
int FileDescriptorManager::ReleaseFileDescriptor(int fd) {
	FileDescriptor* file_descriptor = in_use_[fd];
	//file_descriptor->mut.lock();
	Cache2FileLocked(file_descriptor);
	//file_descriptor->mut.unlock();
	int inode_id = in_use_[fd]->file_inode_;
	inode_manager_->Close(inode_id);

	m.lock();
	if (in_use_.size() < fd || in_use_[fd] == NULL) {
		m.unlock();
		return -1;
	}

	in_use_holes_.push_back(fd);
	free_.push_back(in_use_[fd]);
	in_use_[fd] = NULL;

	m.unlock();

	
	return 0;
}

int FileDescriptorManager::CacheRead(void * ptr, size_t size, size_t count, int fd) {
	FileDescriptor* file_descriptor = in_use_[fd];
	//file_descriptor->mut.lock();
	if (inode_manager_->file_size(file_descriptor->file_inode_) <= file_descriptor->file_offset) {
		//file_descriptor->mut.unlock();
		return 0;
	}
	if ((size*count + file_descriptor->file_offset - 1) / BLOCKSIZE != (file_descriptor->file_offset) / BLOCKSIZE) {	//todo: size*count - 1
		//cross block boundry
		int ret = DirectReadLocked(ptr, size, count, file_descriptor);
		//file_descriptor->mut.unlock();
		return ret;
	}

	if (file_descriptor->block_id_logical == -1) {
		//no cache
		file_descriptor->block_id_logical = (file_descriptor->file_offset) / BLOCKSIZE;
		if (CacheBlockLocked(file_descriptor) == -1) {
			//file_descriptor->mut.unlock();
			return -1;
		}
	}

	register int read_size = file_descriptor->buff_end_position_ - file_descriptor->buff_cur_position_;
	read_size = read_size > size*count ? size*count : read_size;
	//if (read_size > 20 )
		memcpy(ptr, file_descriptor->buff_ + file_descriptor->buff_cur_position_, read_size);
	/*else {
		register char * p = (char *)ptr;
		register size_t i;
		register char * src_ptr = file_descriptor->buff_ + file_descriptor->buff_cur_position_;
		for (i = 0; i < read_size; ++i) {
			p[i] = src_ptr[i];
		}
	}*/
	file_descriptor->buff_cur_position_ += read_size;
	file_descriptor->file_offset += read_size;
	if (file_descriptor->buff_cur_position_ >= file_descriptor->buff_end_position_) {
		if (file_descriptor->buff_cur_position_ > file_descriptor->buff_end_position_) {
			cout << "ERROR: cache read passed boundry" << endl;
			return  -1;
		}
		file_descriptor->buff_cur_position_ = 0;
		file_descriptor->block_id_logical = -1;
	}
	//file_descriptor->mut.unlock();
	return read_size;
}
int FileDescriptorManager::CacheWrite(const void * ptr, size_t size, size_t count, int fd) {
	FileDescriptor* file_descriptor = in_use_[fd];
	//file_descriptor->mut.lock();
	
	if (file_descriptor->block_id_logical == -1) {
		file_descriptor->block_id_logical = file_descriptor->file_offset/BLOCKSIZE;
		file_descriptor->buff_cur_position_ = file_descriptor->file_offset % BLOCKSIZE;
		file_descriptor->buff_start_position_ = file_descriptor->file_offset % BLOCKSIZE;
		//file_descriptor->file_offset = 0;
	}
	if (size*count + file_descriptor->buff_cur_position_ < BLOCKSIZE) {
		memcpy(file_descriptor->buff_ + file_descriptor->buff_cur_position_, ptr, size*count);
		file_descriptor->dirty_ = 1;
		file_descriptor->buff_cur_position_ += size*count;
		file_descriptor->file_offset += size*count;

	}
	else {
		if (Cache2FileLocked(file_descriptor) == -1) {
			//file_descriptor->mut.unlock();
			return -1;
		}
		int write_size = BLOCKSIZE - file_descriptor->buff_cur_position_;
		
		inode_manager_->WriteBlock(file_descriptor->file_inode_, file_descriptor->block_id_logical, write_size, ptr, file_descriptor->buff_cur_position_);
		ptr = (char*)ptr + write_size;
		file_descriptor->block_id_logical++;
		file_descriptor->buff_cur_position_ = 0;
		file_descriptor->buff_start_position_ = 0;

		memcpy(file_descriptor->buff_ + file_descriptor->buff_cur_position_, ptr, size*count - write_size);
		file_descriptor->dirty_ = 1;
		file_descriptor->buff_cur_position_ += size*count - write_size;
		file_descriptor->file_offset += size*count;

	}


	//file_descriptor->mut.unlock();
	return size*count;
}

int FileDescriptorManager::DirectRead(void * ptr, size_t size, size_t count, int fd) {
	FileDescriptor* file_descriptor = in_use_[fd];
	//file_descriptor->mut.lock();
	int ret = DirectReadLocked(ptr, size, count, file_descriptor);
	//file_descriptor->mut.unlock();
	return ret;
}
int FileDescriptorManager::DirectWrite(const void * ptr, size_t size, size_t count, int fd) {
	FileDescriptor* file_descriptor = in_use_[fd];
	//file_descriptor->mut.lock();
	Cache2FileLocked(file_descriptor);
	int ret = inode_manager_->Write(file_descriptor->file_inode_, file_descriptor->file_offset, size*count, ptr);
	file_descriptor->file_offset += ret;
	
	//file_descriptor->mut.unlock();
	return ret;
}


//only support SEEK_SET
int FileDescriptorManager::Seek(int fd, long int offset, int origin) {
	if (origin != SEEK_SET) {
		cout << "ERROR: seek currently only support SEEK_SET" << endl;
		return -1;
	}

	FileDescriptor* file_descriptor = in_use_[fd];
	//file_descriptor->mut.lock();
	if (inode_manager_->file_size(file_descriptor->file_inode_) < offset) {
		//file_descriptor->mut.unlock();
		return -1;
	}
	if (file_descriptor->dirty_) {
		Cache2FileLocked(file_descriptor);
		file_descriptor->file_offset = offset;
		file_descriptor->block_id_logical = -1;
		//file_descriptor->mut.unlock();
		return 0;
		//I am not expecting a seek function call when doing write
	}
	if (offset / BLOCKSIZE == file_descriptor->file_offset / BLOCKSIZE) {
		file_descriptor->buff_cur_position_ += offset - file_descriptor->file_offset;
		file_descriptor->file_offset = offset;

		//file_descriptor->mut.unlock();
		return 0;
	}
	file_descriptor->file_offset = offset;
	file_descriptor->block_id_logical = -1;
	//file_descriptor->mut.unlock();
	return 0;


}


int FileDescriptorManager::CacheBlockLocked(FileDescriptor* fd) {
	fd->buff_end_position_ = inode_manager_->ReadBlock(fd->file_inode_, fd->block_id_logical, fd->buff_);
	if (fd->buff_end_position_ == -1) return -1;
	fd->buff_cur_position_ = 0;
	
}

int FileDescriptorManager::Cache2FileLocked(FileDescriptor* fd) {
	if (fd->dirty_ != 1) return 0;
	size_t size = fd->buff_cur_position_ - fd->buff_start_position_;
	void* ptr = fd->buff_ + fd->buff_start_position_;
	if (inode_manager_->WriteBlock(fd->file_inode_, fd->block_id_logical, size, ptr, fd->buff_start_position_) != size) {
		cout << "ERROR: FIleDescriptorManager failed to write cache to file" << endl;
		return -1;
	}
	//fd->block_id_logical = -1;
	fd->dirty_ = 0;
	return size;
}

int FileDescriptorManager::DirectReadLocked(void * ptr, size_t size, size_t count, FileDescriptor* file_descriptor) {
	int ret = inode_manager_->Read(file_descriptor->file_inode_, file_descriptor->file_offset, size*count, ptr);
	if (ret == -1) {
		return -1;
	}
	file_descriptor->block_id_logical = -1;
	file_descriptor->buff_cur_position_ = 0;
	file_descriptor->buff_start_position_ = 0;
	file_descriptor->buff_end_position_ = 0;
	file_descriptor->block_id_logical = -1;
	file_descriptor->file_offset += size*count;
	return ret;
}