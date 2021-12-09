#ifndef __REQUEST_H__



#define INFINITY 5000
#define DEFAULT_BUFFER_SIZE 1 
#define DEFAULT_SLAVES_THREADS_NUMBER 1
#define MAXBUF 8192


extern int buffer_max_size;
extern int buffer[INFINITY] ;



void *request_salve_handle();
void request_scheduler(int fd);

#endif // __REQUEST_H__
