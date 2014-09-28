#include <stdio.h>
#include <string.h>
#include <stdlib.h>    		// for exit

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>		
#include <arpa/inet.h> //inet_addr

#include <unistd.h>    		// for getopt 
#include <getopt.h>    		// for getopt_long
#include <pthread.h> //for threading , link with lpthread

#define user_file			"user_pass.txt"

char* user[1024];
char* pass[1024];

void load_user_info(void);

int main(int argc, char* argv[])
{
	int socket, new_socket, port_number, client_len;
	char buffer[256];
	struct sockaddr_in, server_addr, client_addr;	

	load_user_info();

	exit(0);
}

void load_user_info(void)
{
	
}
