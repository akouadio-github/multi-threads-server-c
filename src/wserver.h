#ifndef __WSERVER_H__

#include <dirent.h>
#include <errno.h>

#define ROOT_DIR_MAX_BUF 1024
#define DEFAULT_BUFFER_SIZE 1 
#define DEFAULT_PORT 10000 
#define DEFAULT_SLAVES_THREADS_NUMBER 1
char DEFAULT_ROOT[] = ".";


typedef struct master_thread_args
	{
		int s_threads_num; 
		int buffer_size;
		char root_dir[ROOT_DIR_MAX_BUF];
		int port ; 

} master_thread_args;

void *master_thread(void *input);


#endif // __WSERVER_H__