#include "fix_slab.h"

int FixSlab::create(const char * filepath, size_t file_size, int hash_entries, int inode_entries) {
	if (hash_entries*sizeof(HashEntry) + inode_entries*sizeof(InodeEntry) + sizeof(HeadInfo) > file_size)
		file_size = hash_entries*sizeof(HashEntry) + inode_entries*sizeof(InodeEntry) + sizeof(HeadInfo);
	int fd = shm_open(filepath, O_RDWR | O_CREAT | O_EXCL, S_IRWXU);
	if (fd == -1) return -1;
	if (ftruncate(fd, file_size) != 0) return -1;
	if (lseek(fd, 0, SEEK_SET) != 0) return -1;



	data_ = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	header_ = (HeadInfo*)data_;

	header_->hash_off = sizeof(HeadInfo);
	header_->hash_size = hash_entries;
	header_->inode_off = header_->hash_off + sizeof(HashEntry)*hash_entries;
	strncpy(header_->name, filepath, NAME_SIZE);
	header_->inode_size = inode_entries;
	header_->hash_info.hash_size = hash_entries;
	header_->inode_info.cur_inode_index = 0;
	header_->inode_info.Inode_free_list = NULL;
	header_->max_reg_slab_num = -1;
	header_->reg_slab_num = 0;


	hashs_ = (HashEntry*)((size_t)data_ + (size_t)(header_->hash_off));
	inodes_ = (InodeEntry*)((size_t)data_ + (size_t)(header_->inode_off));

	InitMutex();
	close(fd);

	return 0;

	
}
int FixSlab::connect(const char * filepath) {
	int fd = shm_open(filepath, O_RDWR, S_IRWXU);
	if (fd == -1) return -1;
	size_t filesize = lseek(fd, 0, SEEK_END);
	if (filesize < 0) return -1;

	return connect(fd, filesize);
}

int FixSlab::connect(int fd, size_t filesize) {
	
	data_ = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	header_ = (HeadInfo*)data_;
	hashs_ = (HashEntry*)((size_t)data_ + (size_t)(header_->hash_off));
	inodes_ = (InodeEntry*)((size_t)data_ + (size_t)(header_->inode_off));
	return 0;

}

int FixSlab::GetHashTableRegion(HashEntry * & head) {

	head = hashs_;
	return header_->hash_size;

}
int FixSlab::GetInodeRegion(void * & head) {
	if (head != NULL)
		head = inodes_;
	return header_->inode_size;
}


HashEntry * FixSlab::GetHash(int idx) {
	if (idx > header_->hash_size) return NULL;
	return &hashs_[idx];
}


size_t FixSlab::GetFileSize(const char* filepath) {
	return FAKE_FILE_SIZE;
}

int FixSlab::InitMutex() {
	pthread_mutexattr_t mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	if (pthread_mutex_init(&(header_->m), &mattr) != 0) {
		perror("mutux error");
	}
	return 0;
}

/*
int FixSlab::CreateInode(InodeEntry* & new_node) {
	Lock();
	if (header_->inode_info.Inode_free_list != NULL) {
		new_node = (InodeEntry*)header_->inode_info.Inode_free_list;
		header_->inode_info.Inode_free_list = ((InodeEntry*)(header_->inode_info.Inode_free_list))->GetNext();
		new_node->SetNext(NULL);
	}
	else if (header_->inode_size > header_->inode_info.cur_inode_index) {
		new_node = GetInode(header_->inode_info.cur_inode_index);
		header_->inode_info.cur_inode_index++;
	}
	else {
		Unlock();
		return -1;
	}

	Unlock();
	return 0;
}
int FixSlab::ReleaseInode(int node_id) {
	return ReleaseNode(GetInode(node_id));
}
int FixSlab::ReleaseInode(InodeEntry* node) {
	if (node == NULL) return -1;
	node->erase();
	Lock();
	node->SetNext((InodeEntry*)(header_->inode_info.Inode_free_list));
	header_->inode_info.Inode_free_list = node;

	Unlock();
}

int FixSlab::GetIndex(InodeEntry * inode) {
	return inode - inodes_;
}
*/
void FixSlab::Lock() {
	pthread_mutex_lock(&(header_->m));
}
void FixSlab::Unlock() {
	pthread_mutex_unlock(&(header_->m));
}

struct InodeInfo * FixSlab::GetInodeInfo() {
	return &(header_->inode_info);
}

struct HashInfo * FixSlab::GetHashInfo() {
	return &(header_->hash_info);
}
/*return the max num of allowed regular slab*/
int & FixSlab::GetRegSlabMax() {
	return header_->max_reg_slab_num;
}
int & FixSlab::GetRegSlabCur() {
	return header_->reg_slab_num;
}
const char* FixSlab::GetName() {
	return header_->name;
}

size_t FixSlab::GetCurBlockSize() {
	return header_->cur_block_size;
}
size_t FixSlab::GetMaxBlockSize() {
	return header_->max_block_size;
}
int FixSlab::SetCurBlockSize(size_t size) {
	header_->cur_block_size = size;
}
int FixSlab::SetMaxBlockSize(size_t size) {
	header_->max_block_size = size;
}