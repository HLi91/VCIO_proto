#ifndef INODE_H_
#define INODE_H_

#include <pthread.h>
#include "islab.h"



#define FILEDELETE 0x01
#define FILEMOVE 0x02
#define FILENORMAL 0x3
#define FILEUNINIT 0x0

class InodeEntry {		
	

public:
	InodeEntry* next_ = NULL;
	int reference_count_ = 0;
	int block_count_;
	int filesize_;
	int status_;

	pthread_mutex_t m;

	
	

	
	//not in use

	//tree might be a better, but link list should be fine
	BlockInfo_t direct[16];
	BlockInfo_t second;
	BlockInfo_t third;

};


#endif // !INODE_H_
