#ifndef FILEDESCRIPTOR_H_
#define FILEDESCRIPTOR_H_


#include <mutex>
#include <vector>
#include <stdio.h>
#include <iostream>

#include "islab.h"
#include "InodeManager.h"

#define SEEK_SET 0
#define SEET_CUR 1
#define SEEK_END 2

using namespace std;
class FileDescriptor{
	friend class FileDescriptorManager;
	int init(int inode_index);
	int clear();
private:
	int file_inode_;
	size_t file_offset;

	char buff_ [BLOCKSIZE];
	int buff_start_position_;
	int buff_cur_position_;
	int buff_end_position_;

	
	int block_id_logical;
	//BlockInfo_t block_info_;



	int cur_mode_;		//currently not in use
	int dirty_;
	
	int fd_id_;
	mutex mut;



};

class FileDescriptorManager {
public:
	FileDescriptorManager(InodeManager * inode_manager) : inode_manager_(inode_manager) {}

	~FileDescriptorManager();

	int GetFileDescriptor(int inode_index);
	int ReleaseFileDescriptor(int fd);

	int CacheRead(void * ptr, size_t size, size_t count, int fd);
	int DirectRead(void * ptr, size_t size, size_t count, int fd);
	int CacheWrite(const void * ptr, size_t size, size_t count, int fd);
	int DirectWrite(const void * ptr, size_t size, size_t count, int fd);
	

	int Seek(int fd, long int offset, int origin);

	int GetInode(int fd);

private:
	
	int CacheBlockLocked(FileDescriptor* fd);

	int Cache2FileLocked(FileDescriptor* fd);
	int DirectReadLocked(void * ptr, size_t size, size_t count, FileDescriptor* file_descriptor);


private:
	vector<FileDescriptor*> in_use_;
	vector<int> in_use_holes_;
	vector<FileDescriptor*> free_;
	mutex m;
	InodeManager* inode_manager_;
};

#endif // !FILEDESCRIPTOR_H_
