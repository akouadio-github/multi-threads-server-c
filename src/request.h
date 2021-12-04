#ifndef __REQUEST_H__

#define INFINTY 5000


#define MAXBUF 1024 
#define DEFAULT_BUFFER_SIZE 1 
#define DEFAULT_PORT 10000 
#define DEFAULT_SLAVES_THREADS_NUMBER 1

typedef struct request {
  int fd;
  char *filename;
  int filesize;
  char *cgiargs;
  int is_static;

} request ;

typedef struct master_thread_args
	{
		int s_threads_num; 
		int buffer_size;
		char root_dir[MAXBUF];
		int port ; 

	} master_thread_args;


extern int buffer_max_size;
extern request buffer[INFINTY] ;

#define DEFAULT_BUFFER_SIZE 1 
#define DEFAULT_SLAVES_THREADS_NUMBER 1
#define MAXBUF 8192



void *request_salve_handle();
void request_scheduler(int fd);

#endif // __REQUEST_H__
