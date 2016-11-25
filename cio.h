#ifndef CIO_H
#define CIO_H

#include <stdio.h>
#include <vector>
#include <map>
#include <iostream>

#include "fix_slab.h"
#include "regular_slab.h"
#include "filedescriptor.h"
#include "InodeManager.h"
#include "HashManager.h"
#include "RegSlabManager.h"

using namespace std;







class VCIO {
public:
	int init(const char * filename, int fix_slab_num);
	int connect(const char * filename);
	
	
	int cio_fopen(const char * filename, const char * mode);
	int cio_fclose(int fd);
	int cio_fwrite(const void * ptr, size_t size, size_t count, int fd);
	int cio_fread(void * ptr, size_t size, size_t count, int fd);
	int cio_unlink(const char *filename);
	int cio_lseek(int fd, long int offset, int origin);

	int cio_flush(const char * filename);

	

	//other APIs
	int cio_rename();
	int cio_link();
	int cio_stat();


private:
	

	vector<FixSlab> fix_slab_list_;		//lets assume only one is allowed for this demo project
	RegularSlabManager * reg_slab_manager_;
	InodeManager* inode_manager_;
	HashManager* hash_manager_;
	FileDescriptorManager* fd_manager_;
	
};





#endif