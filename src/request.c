#include "io_helper.h"
#include "request.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int buffer_current_size = 0;
int policy = DEFAULT_POLICY;
int buffer_max_size;
request buffer[INFINTY];

void request_scheduler(int fd)
{
	char filename[MAXBUF];
	char cgiargs[MAXBUF];
	char uri[MAXBUF];

	int result;
	int is_static;
	struct stat sbuf;
	char temp[MAXBUF];
	char method[MAXBUF], version[MAXBUF];
	request temp_;

	readline_or_die(fd, temp, MAXBUF);
	// Remove them, don't forget
	sscanf(temp, "%s %s %s", method, uri, version);
	// printf("method:%s uri:%s version:%s\n", method, uri, version);

	if (strcasecmp(method, "GET"))
	{
		request_error(fd, method, "501", "Not Implemented", "server does not implement this method");
		return;
	}
	request_read_headers(fd);
	is_static = request_parse_uri(uri, filename, cgiargs);

	if (stat(filename, &sbuf) < 0)
	{
		request_error(fd, filename, "404", "Not found", "server could not find this file");
		return;
	}
	if (is_static)
	{
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
		{
			request_error(fd, filename, "403", "Forbidden", "server could not read this file");
			return;
		}
	}
	else
	{
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
		{
			request_error(fd, filename, "403", "Forbidden", "server could not run this CGI program");
			return;
		}
	}

	temp_.fd = fd;
	temp_.filename = filename;
	temp_.filesize = sbuf.st_size;
	temp_.is_static = is_static;
	temp_.cgiargs = cgiargs;

	pthread_mutex_lock(&lock);
	// adding requests to buffer
	result = insert_fifo_sff(temp_, buffer_max_size);
	// printf("%d-------\n", result);

	while (result == -1)
	{
		result = insert_fifo_sff(temp_, buffer_max_size); //keep checking if the request can be added or not till the point request finally gets added
	}

	// signaling the waiting thread
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&lock);
}

int insert_fifo_sff(request element, int buffer_max_size)
{

	if (buffer_current_size == buffer_max_size)
		return -1;
	else
	{
		buffer[buffer_current_size] = element;
		buffer_current_size++;
	}
	printf(" \a New insertion : (Socket Id) %d (filename) %s \n", buffer[buffer_current_size - 1].fd, buffer[buffer_current_size - 1].filename);
	// print_buffer(buffer, buffer_current_size);
	return 0;
}

request remove_sff()
{
	int small_req_filesize = buffer[0].filesize;
	int small_index = 0;
	request req_output;

	for (int i = 1; i < buffer_current_size; i++)
	{
		if (buffer[i].filesize < small_req_filesize)
		{
			small_req_filesize = buffer[i].filesize;
			small_index = i;
		}
	}
	req_output.fd = buffer[small_index].fd;
	req_output.filename = buffer[small_index].filename;
	req_output.filesize = buffer[small_index].filesize;

	for (int i = small_index; i < buffer_current_size - 1; i++)
	{
		buffer[i].fd = buffer[i + 1].fd;
		strcpy(buffer[i].filename, buffer[i + 1].filename);
		buffer[i].filesize = buffer[i + 1].filesize;
	}
	printf("Request for %s is removed from the buffer.\n", req_output.filename);
	printf("\n\n");
	buffer_current_size--;
	return req_output;
}

request remove_fifo()
{
	
	request req_output;

	// req_output = buffer[0];

	req_output.fd = NULL;
	req_output.filename = NULL;
	req_output.filesize = 0;
	req_output.is_static = 0;
	req_output.cgiargs = NULL;

	if (buffer_current_size == 0)
    	return req_output;

	req_output.fd = buffer[0].fd ;
	req_output.filename = buffer[0].filename;
	req_output.filesize = buffer[0].filesize;
	req_output.is_static = buffer[0].is_static;
	req_output.cgiargs = buffer[0].cgiargs;

	for (int i = 0; i < buffer_current_size - 1; i++)
	{
		buffer[i] = buffer[i + 1];
	}
	printf("\t\t Manage request : (Socket Id) %d (filename) %s \n", req_output.fd, req_output.filename);

	buffer_current_size--;

	return req_output;
}

// void print_buffer(request *buffer, int last)
// {

// 	for (int i = 0; i < last; i++)
// 	{
// 		fflush(stdout);
// 		fflush(stdout);
// 	}
// }

void request_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
	char buf[MAXBUF], body[MAXBUF];

	// Create the body of error message first (have to know its length for header)
	sprintf(body, ""
				  "<!doctype html>\r\n"
				  "<head>\r\n"
				  "  <title> ToBi ToBa WebServer Error</title>\r\n"
				  "</head>\r\n"
				  "<body>\r\n"
				  "  <h2>%s: %s</h2>\r\n"
				  "  <p>%s: %s</p>\r\n"
				  "</body>\r\n"
				  "</html>\r\n",
			errnum, shortmsg, longmsg, cause);

	// Write out the header requestrmation for this response
	sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	write_or_die(fd, buf, strlen(buf));

	sprintf(buf, "Content-Type: text/html\r\n");
	write_or_die(fd, buf, strlen(buf));

	sprintf(buf, "Content-Length: %lu\r\n\r\n", strlen(body));
	write_or_die(fd, buf, strlen(buf));

	// Write out the body last
	write_or_die(fd, body, strlen(body));
}

void request_read_headers(int fd)
{
	char buf[MAXBUF];

	readline_or_die(fd, buf, MAXBUF);
	while (strcmp(buf, "\r\n"))
	{
		readline_or_die(fd, buf, MAXBUF);
	}
	return;
}

int request_parse_uri(char *uri, char *filename, char *cgiargs)
{
	char *ptr;

	if (!strstr(uri, "cgi"))
	{
		// static
		strcpy(cgiargs, "");
		sprintf(filename, ".%s", uri);
		if (uri[strlen(uri) - 1] == '/')
		{
			strcat(filename, "index.html");
		}
		return 1;
	}
	else
	{
		// dynamic
		ptr = index(uri, '?');
		if (ptr)
		{
			strcpy(cgiargs, ptr + 1);
			*ptr = '\0';
		}
		else
		{
			strcpy(cgiargs, "");
		}
		sprintf(filename, ".%s", uri);
		return 0;
	}
}

void request_get_filetype(char *filename, char *filetype)
{
	if (strstr(filename, ".html"))
		strcpy(filetype, "text/html");
	else if (strstr(filename, ".gif"))
		strcpy(filetype, "image/gif");
	else if (strstr(filename, ".jpg"))
		strcpy(filetype, "image/jpeg");
	else
		strcpy(filetype, "text/plain");
}

void request_serve_dynamic(int fd, char *filename, char *cgiargs)
{
	char buf[MAXBUF], *argv[] = {NULL};

	// The server does only a little bit of the header.
	// The CGI script has to finish writing out the header.
	sprintf(buf, ""
				 "HTTP/1.0 200 OK\r\n"
				 "Server: OSTEP WebServer\r\n");

	write_or_die(fd, buf, strlen(buf));

	if (fork_or_die() == 0)
	{											   // child
		setenv_or_die("QUERY_STRING", cgiargs, 1); // args to cgi go here
		dup2_or_die(fd, STDOUT_FILENO);			   // make cgi writes go to socket (not screen)
		extern char **environ;					   // defined by libc
		execve_or_die(filename, argv, environ);
	}
	else
	{
		wait_or_die(NULL);
	}
}

void request_serve_static(int fd, char *filename, int filesize)
{
	int srcfd;
	char *srcp, filetype[MAXBUF], buf[MAXBUF];

	request_get_filetype(filename, filetype);
	srcfd = open_or_die(filename, O_RDONLY, 0);

	// Rather than call read() to read the file into memory,
	// which would require that we allocate a buffer, we memory-map the file
	srcp = mmap_or_die(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	close_or_die(srcfd);

	// put together response
	sprintf(buf, ""
				 "HTTP/1.0 200 OK\r\n"
				 "Server: OSTEP WebServer\r\n"
				 "Content-Length: %d\r\n"
				 "Content-Type: %s\r\n\r\n",
			filesize, filetype);

	write_or_die(fd, buf, strlen(buf));

	//  Writes out to the client socket the memory-mapped file
	write_or_die(fd, srcp, filesize);
	munmap_or_die(srcp, filesize);
}

void *request_salve_handle()
{

	while (policy == 1)
	{
		request current_request;
		pthread_mutex_lock(&lock);
		current_request = remove_fifo();
		while (current_request.fd == NULL)
		{
			pthread_cond_wait(&cond, &lock);
			current_request = remove_fifo();
		}
		pthread_mutex_unlock(&lock);
		// printf("%d\n", current_request.is_static);
		// printf("%s\n", current_request.filename);
		// printf("%d\n", current_request.filesize);

		if (current_request.is_static)
		{
			request_serve_static(current_request.fd, current_request.filename, current_request.filesize);
		}
		else
		{
			request_serve_dynamic(current_request.fd, current_request.filename, current_request.cgiargs);
		}
		close_or_die(current_request.fd);
	}
	// 	while (policy == 2)
	// 	{
	// 		current_request = remove_sff(buffer);
	// 		while (current_request.fd == -1)
	// 		{
	// 			printf("Empty buffer... Waiting");
	// 			pthread_cond_wait(&cond, &lock);
	// 			current_request = remove_sff(buffer);
	// 		}
	// 	}
	//
}
