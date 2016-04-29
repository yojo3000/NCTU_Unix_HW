#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <sys/socket.h>
#define COLOR_RED  "\x1B[31m"
#define COLOR_END "\033[0m"
#include <netinet/in.h>
#include <arpa/inet.h>

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){

	if(addr->sa_family == AF_INET)
	{
		struct sockaddr_in *test = (struct sockaddr_in *)addr;
		char *ip = inet_ntoa(test->sin_addr);
		

		FILE *f = fopen("info_output", "w");
		
		fprintf(f, "==========\n");		
		fprintf(f, "I am in connect function\n");	
		fprintf(f, "int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)\n\n");
		fprintf(f, "ip address: %s\n", ip);
		fprintf(f, "==========\n");
		fclose(f);
	}
	
	int *(*original_connect)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	// declare a function pointer

	original_connect = dlsym(RTLD_NEXT, "connect");
	// save the origin connect
		
	return (*original_connect)(sockfd, addr, addrlen);
}
