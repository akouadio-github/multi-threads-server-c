#include "io_helper.h"
#include "request.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int buffer_current_size = 0;
int buffer_max_size;
int buffer[INFINITY];

void request_scheduler(int fd)
{

	int result;

	pthread_mutex_lock(&lock);
	result = insert_in_requests_buffer(fd, buffer_max_size);
	
	while (result == -1)
	{
		result = insert_in_requests_buffer(fd, buffer_max_size); //keep checking if the request can be added or not till the point request finally gets added
	}

	// signaling the waiting thread
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&lock);
}

int insert_in_requests_buffer(int element, int buffer_max_size)
{

	if (buffer_current_size == buffer_max_size)
		return -1;
	else
	{
		buffer[buffer_current_size] = element;
		buffer_current_size++;
	}
	printf("--> New insertion : (Socket Id) %d \n", buffer[buffer_current_size - 1]);
	return 0;
}

int remove_from_requests_buffer()
{
	
	int req_output;

	req_output = NULL;

	if (buffer_current_size == 0)
    	return req_output;

	req_output = buffer[0];

	for (int i = 0; i < buffer_current_size - 1; i++)
	{
		buffer[i] = buffer[i + 1];
	}

	buffer_current_size--;

	return req_output;
}



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
	int is_static;
	struct stat sbuf;
	char temp[MAXBUF];
	char method[MAXBUF], version[MAXBUF];
	char filename[MAXBUF];
	char cgiargs[MAXBUF];
	char uri[MAXBUF];

	int has_error;
	int current_request;

	while (1)
	{
		has_error = 0 ;
		pthread_mutex_lock(&lock);
		current_request = remove_from_requests_buffer();
		while (current_request == NULL)
		{
			pthread_cond_wait(&cond, &lock);
			current_request = remove_from_requests_buffer();
		}
		pthread_mutex_unlock(&lock);

		readline_or_die(current_request, temp, MAXBUF);
		sscanf(temp, "%s %s %s", method, uri, version);

		if (strcasecmp(method, "GET"))
		{
			request_error(current_request, method, "501", "Not Implemented", "server does not implement this method");
			has_error = 1;
		}
		request_read_headers(current_request);
		is_static = request_parse_uri(uri, filename, cgiargs);

		if (stat(filename, &sbuf) < 0 && !has_error)
		{
			printf("--> Thread %d works with socket %d (File Error %s) \n", gettid(), current_request, filename);
			request_error(current_request, filename, "404", "Not found", "server could not find this file");
			has_error = 1;

		}
		if (is_static)
		{
			if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode) && !has_error)
			{
				printf("--> Thread %d works with socket %d ( Could not read this file %s)\n", gettid(), current_request, filename);
				request_error(current_request, filename, "403", "Forbidden", "server could not read this file");
				has_error = 1;

			}
		}
		else
		{
			if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode) && !has_error)
			{
				printf("--> Thread %d works with socket %d (Could not execute this program %s)\n", gettid(), current_request, filename);
				request_error(current_request, filename, "403", "Forbidden", "server could not run this CGI program");
				has_error = 1;
			}
		}
		if (has_error){
			close_or_die(current_request);
		}
		else {
			if (is_static)
			{
				printf("--> Thread %d works with socket %d (static filename %s, size %d)\n", gettid(), current_request, filename, sbuf.st_size);
				request_serve_static(current_request, filename, sbuf.st_size);
			}
			else
			{
				printf("--> Thread %d works with socket %d (dynamic executable %s, args %s)\n", gettid(), current_request, filename, cgiargs);
				request_serve_dynamic(current_request, filename, cgiargs);
			}
			close_or_die(current_request);
		}
	}
}
