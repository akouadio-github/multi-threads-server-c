#ifndef __REQUEST_H__

#define INFINTY 5000


typedef struct request {
  int fd;
  char *filename;
  int filesize;
  char *cgiargs;
  int is_static;

} request ;

extern int policy;
extern int buffer_max_size;
extern request buffer[INFINTY] ;

#define DEFAULT_BUFFER_SIZE 1 
#define DEFAULT_SLAVES_THREADS_NUMBER 1
#define DEFAULT_POLICY 1

#define MAXBUF 8192



void *request_salve_handle();
void request_scheduler(int fd);
int insert_fifo_sff(request element, int buffer_max_size);
request remove_sff();
request remove_fifo();

#endif // __REQUEST_H__
