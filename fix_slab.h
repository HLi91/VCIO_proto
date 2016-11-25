#ifndef CIO_FIX_SLAB_H
#define CIO_FIX_SLAB_H

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "inode.h"
#include "islab.h"



#define FAKE_FILE_SIZE 2000000000;
#define NAME_SIZE 255
struct InodeInfo {
	int cur_inode_index;
	void * Inode_free_list;
};


struct HashInfo {
	int hash_size;
};
class HeadInfo {
public:
	int hash_off;
	int inode_off;
	int hash_size;
	int inode_size;
	char name[NAME_SIZE];
	size_t cur_block_size;
	size_t max_block_size;
	struct InodeInfo inode_info;
	struct HashInfo hash_info;

	int max_reg_slab_num;
	int reg_slab_num;

	pthread_mutex_t m;
};

//this class should be move to hashmanager.h
class HashEntry {
public:
	int key_;
	int value_;
	HashEntry(int key, int value) {
		key_ = key;
		value_ = value;
	}
};



class FixSlab : public iSlab{
public:
	int create(const char * filepath, size_t file_size, int hash_entries, int inode_entries);
	int connect(const char * filepath);

	int GetHashTableRegion(HashEntry * & head);	
	HashEntry * GetHash(int idx);

	int GetInodeRegion(void *&head);

	void Lock();
	void Unlock();
	struct InodeInfo * GetInodeInfo();
	struct HashInfo * GetHashInfo();

	int & GetRegSlabMax();
	int & GetRegSlabCur();
	const char* GetName();

	size_t GetCurBlockSize();
	size_t GetMaxBlockSize();
	int SetCurBlockSize(size_t size);
	int SetMaxBlockSize(size_t size);
	/*		functions moved to InodeManager class
	InodeEntry * GetInode(int idx);
	int GetIndex(InodeEntry * inode);
	int CreateInode(InodeEntry* & new_node);
	int ReleaseInode(int node_id);
	int ReleaseInode(InodeEntry* node);
	*/
	

private:
	size_t GetFileSize(const char* filepath);
	int connect(int fd, size_t filesize);
	int InitMutex();
	

	HeadInfo * header_;
	HashEntry * hashs_;
	void* inodes_;
};

/* 
class MasterFixSlab_t : public FixSlab_t {
};

*/
#endif