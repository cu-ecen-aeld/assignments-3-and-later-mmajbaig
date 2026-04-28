#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>

#define OUTFILE_PATH "/var/tmp/aesdsocketdata"

volatile sig_atomic_t stop = 0;

void handle_signal(int signo)
{
    stop = 1;
}

int main(int argc, char*argv[]){
	struct addrinfo hints, *res;
	int sockfd, accept_fd;
	struct sockaddr_storage client_addr;
	socklen_t addr_size;
	char tmp[4096];
	int recv_length = 0;
	int opt = 1;
	
	int daemon_mode = 0;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-d") == 0) {
			daemon_mode = 1;
		}
	}
		
	// Set up signal handler
	
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));

	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	
	// Open syslog
    openlog(NULL, 0, LOG_USER);

	// first, load up address structs with getaddrinfo():

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	getaddrinfo(NULL, "9000", &hints, &res);

	// make a socket:

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd == -1)
	{
		printf("Socket opening error: %s\n", strerror(errno));
		free(res);
		return EXIT_FAILURE;
	}
	
	// Set SO_REUSEADDR
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        goto CLOSE_SOCKET_AND_EXIT_FAILURE;
    }

	// bind it to the port we passed in to getaddrinfo():

	int ret = bind(sockfd, res->ai_addr, res->ai_addrlen);
	if(ret == -1)
	{
		printf("Socket bind error: %s\n", strerror(errno));
		goto CLOSE_SOCKET_AND_EXIT_FAILURE;
	}
	
	if(daemon_mode == 1)
	{
		pid_t process_id = fork();
		
		if(process_id != 0)
		{
			close(sockfd);
			free(res);
			return EXIT_SUCCESS;
		}
	}
	
	ret = listen(sockfd, 5);
	if(ret == -1)
	{
		printf("Socket listen error: %s\n", strerror(errno));
		goto CLOSE_SOCKET_AND_EXIT_FAILURE;
	}
	
	addr_size = sizeof client_addr;
	
	while(!stop)
	{
		accept_fd = accept(sockfd, (struct sockaddr *)&client_addr,
														   &addr_size);
		if(accept_fd == -1)
		{
			if (errno == EINTR) {
				// interrupted by signal
				if (stop)
					break;
				else
					continue;
			} else {
				printf("Socket accept error: %s\n", strerror(errno));
				continue;
			}			
		}
		
		char ipstr[INET6_ADDRSTRLEN];

		if (client_addr.ss_family == AF_INET) {
			// IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)&client_addr;
			inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
		} else if (client_addr.ss_family == AF_INET6) {
			// IPv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&client_addr;
			inet_ntop(AF_INET6, &(ipv6->sin6_addr), ipstr, sizeof(ipstr));
		} else {		
			printf("Unknown address family\n");
			goto CLOSE_ACCEPTED_SOCKET_AND_EXIT_FAILURE;
		}
		
		printf("Accepted connection from %s\n", ipstr); 
		syslog(LOG_INFO, "Accepted connection from %s\n", ipstr); 
		
		recv_length = 0;
		
		do{
			recv_length = recv(accept_fd, tmp, 4096, 0);
			if(recv_length<0 && errno == EINTR)
			{
				printf("Socket recv error: %s\n", strerror(errno));
				if (stop) break;
				continue;
			}
			// Search or \n and if found then fix recv_length
			char* found_byte = memchr(tmp, '\n', recv_length);
			if(found_byte)
			{
				printf("Found endline %s\n", tmp);
				recv_length = (char *)found_byte - tmp + 1;
			}
				
			// Open and Write the data to the file
			
			FILE* outfile = fopen(OUTFILE_PATH, "a+");
			if (!outfile )
			{
				perror("fopen");
				break;
				//goto CLOSE_ACCEPTED_SOCKET_AND_EXIT_FAILURE;
			}
			
			fwrite(tmp, 1, recv_length, outfile);
			
			// If \n is found, read and send back the whole file
			if(found_byte)
			{
				fflush(outfile);
				fseek(outfile, 0, SEEK_SET);
				
				while(!feof(outfile))
				{
					int read_bytes = fread(tmp, 1, 4096, outfile);
					
					// Send the data back
					int sent_length = send(accept_fd, tmp, read_bytes, 0);
					if(sent_length<0)
					{
						printf("Socket send error: %s\n", strerror(errno));
						break;
					}
				}			
				
				fclose(outfile);
				break;
			}
			fclose(outfile);
		}while(!stop);
		
		close(accept_fd);
		printf("Closed connection from %s\n", ipstr); 
		syslog(LOG_INFO, "Closed connection from %s\n", ipstr); 
	}
	close (sockfd);
	free(res);
	printf("Caught signal, exiting\n");
	syslog(LOG_INFO, "Caught signal, exiting\n");
	
	if(remove(OUTFILE_PATH)!= 0)
		printf("Error during file deletion: %s\n", strerror(errno));
	return EXIT_SUCCESS;
	
CLOSE_ACCEPTED_SOCKET_AND_EXIT_FAILURE:
	close(accept_fd);
CLOSE_SOCKET_AND_EXIT_FAILURE:
	close (sockfd);
	free(res);
	if(remove(OUTFILE_PATH)!= 0)
		printf("Error during file deletion: %s\n", strerror(errno));
	return EXIT_FAILURE;
}
