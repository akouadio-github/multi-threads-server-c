#include <stdio.h>
#include <sys/stat.h>

#include "wserver.h"
#include "request.h"
#include "io_helper.h"




//
// ./wserver [-d <base_dir>] [-p <port_num>] [-t s_threads_num] [-b buffer_size]
//

int main(int argc, char *argv[])
{
	int c;
	char *root_dir = DEFAULT_ROOT;
	int port = DEFAULT_PORT;
	int s_threads_num = DEFAULT_SLAVES_THREADS_NUMBER;
	int buffer_size = DEFAULT_BUFFER_SIZE;
	
	pthread_t m_thread;


	while ((c = getopt(argc, argv, "d:p:t:b:")) != -1)
		switch (c)
		{
		case 'd':
			root_dir = optarg;

			break;
		case 'p':
			port = atoi(optarg);
			if (port <= 1023 || port > 65535)
			{
				fprintf(stderr, "[-p <port>]  port number must be greater than 1023 and less than 65535 \n");
				exit(1);
			 }
			break;
		case 't':
			s_threads_num = atoi(optarg);

			break;
		case 'b':
			buffer_size = atoi(optarg);

			break;

		default:
			fprintf(stderr, "usage: ./wserver [-d <base_dir>] [-p <port_num>] [-t s_threads_num] [-b buffer_size]\n");
			exit(1);
		}

		master_thread_args master_args;
		master_args.s_threads_num = s_threads_num;
		strcpy (master_args.root_dir, root_dir);
		master_args.port = port;
		master_args.buffer_size = buffer_size;
		
		pthread_create(&m_thread, NULL, master_thread, &master_args);
		pthread_join(m_thread, NULL);

	return 0;
}

void *master_thread(void *input){

		char root_dir[ROOT_DIR_MAX_BUF];
		strcpy(root_dir, ((master_thread_args *) input)->root_dir);
		int port = ((master_thread_args *) input)->port;
	 	int s_threads_num = ((master_thread_args *) input)->s_threads_num;;
	 	int buffer_size = ((master_thread_args *) input)->buffer_size;;

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
		
	}
}
