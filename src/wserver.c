#include <stdio.h>
#include <sys/stat.h>
#include "request.h"
#include "io_helper.h"

#define MAXBUF 1024 
#define DEFAULT_BUFFER_SIZE 1 
#define DEFAULT_PORT 10000 
#define DEFAULT_SLAVES_THREADS_NUMBER 1

char DEFAULT_ROOT[] = ".";


//
// ./wserver [-d <basedir>] [-p <portnum>] [-t s_threads_num] [-b buffer_size] [-s policy]
//

int main(int argc, char *argv[])
{
	int c;
	char *root_dir = DEFAULT_ROOT;
	int port = DEFAULT_PORT;
	int s_threads_num = DEFAULT_SLAVES_THREADS_NUMBER;
	int buffer_size = DEFAULT_BUFFER_SIZE;
	

	while ((c = getopt(argc, argv, "d:p:t:b:s:")) != -1)
		switch (c)
		{
		case 'd':
			root_dir = optarg;
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 't':
			s_threads_num = atoi(optarg);
			break;
		case 'b':
			buffer_size = atoi(optarg);
			break;
		case 's':
			if(strcmp(optarg, "SFF") == 0)
			policy = 2 ; // SFF
			break;
		default:
			fprintf(stderr, "usage: ./wserver [-d <basedir>] [-p <portnum>] [-t s_threads_num] [-b buffer_size] [-s policy (FIFO or SFF)]\n");
			exit(1);
		}
		//Create threads and buffers
		pthread_t s_threads_pool[s_threads_num];		
		buffer_max_size = buffer_size ;
		

		// create the thread pool
		for(int i = 0; i<s_threads_num; i++)
    		pthread_create(&s_threads_pool[i], NULL, request_salve_handle, NULL);

		// run out of this directory
		chdir_or_die(root_dir);

		// now, get to work
		int listen_fd = open_listen_fd_or_die(port);

	while (1) {
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
		request_scheduler(conn_fd);
		// request_handle(conn_fd);
	}

	return 0;
}
