#include "cio.h"


int VCIO::init(const char * filename, int fix_slab_num) {
	if (fix_slab_list_.size() == 0)
		fix_slab_list_.push_back(FixSlab());
	if (fix_slab_list_[0].create(filename, 0, 10000, 10000) == -1) {
		cout << "fail to create first slab" << endl;
		return -1;
	}

	reg_slab_manager_ = new RegularSlabManager(&fix_slab_list_[0]);
	inode_manager_ = new InodeManager(&fix_slab_list_[0], reg_slab_manager_);
	hash_manager_ = new HashManager(&fix_slab_list_[0]);
	fd_manager_ = new FileDescriptorManager(inode_manager_);
}
int VCIO::connect(const char * filename) {
	if (fix_slab_list_.size() == 0)
		fix_slab_list_.push_back(FixSlab());
	fix_slab_list_[0].connect(filename);
	reg_slab_manager_ = new RegularSlabManager(&fix_slab_list_[0]);

	inode_manager_ = new InodeManager(&fix_slab_list_[0], reg_slab_manager_);
	hash_manager_ = new HashManager(&fix_slab_list_[0]);
	fd_manager_ = new FileDescriptorManager(inode_manager_);
}

/* for this demo, mode is ignored
 * also assume a file descriptor will not be used for both read and write (read only file system)
 */
int VCIO::cio_fopen(const char * filename, const char * mode) {
	/*InodeEntry* inode;
	if (hash_manager_->find(filename) == 0) {
		
		int inode_index = inode_manager_->CreateInode(inode);
		(*hash_manager_)[filename] = inode_index;
	}
	else {
		inode = inode_manager_->GetInode((*hash_manager_)[filename]);
	}
	
	int fd = fd_manager_->GetFileDescriptor(inode);
	return fd;*/
	int inode_id;
	if (hash_manager_->find(filename) == 0) {

		if (inode_manager_->CreateInode(inode_id) != 0) {
			cout << "ERROR:Fopen could create new node" << endl;
		}
		(*hash_manager_)[filename] = inode_id;
	}
	inode_id = (*hash_manager_)[filename];
	int ret = fd_manager_->GetFileDescriptor(inode_id);
	return ret;


}
int VCIO::cio_fclose(int fd) {
	fd_manager_->ReleaseFileDescriptor(fd);

}
int VCIO::cio_fwrite(const void * ptr, size_t size, size_t count, int fd) {
	if (size*count > BLOCKSIZE) {
		return fd_manager_->DirectWrite(ptr, size, count, fd);
	}
	return fd_manager_->CacheWrite(ptr, size, count, fd);
}
int VCIO::cio_fread(void * ptr, size_t size, size_t count, int fd) {
	if (size*count > BLOCKSIZE) {
		return fd_manager_->DirectRead(ptr, size, count, fd);
	}
	return fd_manager_->CacheRead(ptr, size, count, fd);
}
int VCIO::cio_unlink(const char *filename) {
	int inode_id;
	if (hash_manager_->find(filename) == 0) {

		cout << "file not exist" << endl;
		return -1;
	}
	inode_manager_->DeleteInode(inode_id);		//no link allowed right now
	hash_manager_->Erase(filename);
	return 0;
}
int VCIO::cio_lseek(int fd, long int offset, int origin) {
	return fd_manager_->Seek(fd, offset, origin);
}





//other APIs
int VCIO::cio_rename() {

}
int VCIO::cio_link() {

}
int VCIO::cio_stat() {

}
int VCIO::cio_flush(const char * filename) {

}