/*****************************************************************
**	File name: 		client.c
**	Author:			Xusheng Gao (xg2193)
**  Description:	Simple server-client socket programming 
**					assignment.
**
*****************************************************************/

/*********************** Includes *******************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>    	
#include <stdbool.h>	

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>		
#include <arpa/inet.h> 

#include <pthread.h> 

/******************* User Defines ********************************/

/******************* Global Variables ****************************/

/******************* Function Prototype **************************/
void error(char* str);

/******************* Main program ********************************/

int main(int argc, char* argv[])
{
	/*int client_socket, port_number, n;
	struct sockaddr_in server_addr;
    struct hostent *server;
    
    if (argc < 3) {
    	error("Did not specify address and port number.");
    	exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
	port_number = atoi(argv[1]);
	
    client_socket =  socket(PF_INET, SOCK_STREAM, 0);
    
    if(client_socket < 0)
	{
		error("Failed to open socket.");
		exit(EXIT_FAILURE);
	}*/
	
	exit(EXIT_SUCCESS);
}

void error(char* str)
{
  fprintf(stderr, "ERROR: %s\n", str);
}


