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
#include <signal.h>

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>		
#include <arpa/inet.h> 

#include "Client.h"

/******************* User Defines ********************************/
#define BUFFER_SIZE				512

/******************* Global Variables ****************************/
int client_socket;
int port_number;

/******************* Function Prototype **************************/
void error(char* str);

void quitHandler();

/******************* Main program ********************************/

void quitHandler()
{
	printf("\nUser teminated client process.\n");
	shutdown(client_socket, 2);
	exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[])
{
	struct sockaddr_in server_addr;
	int len = 0;
	int count = 0;
	char* addr = "";
    char msg[BUFFER_SIZE] = "";
    char server_msg[BUFFER_SIZE] = "";
    
    if (argc < 3) {
    	error("Did not specify address and port number.");
    	exit(EXIT_FAILURE);
    }
    
    // setup to capture process terminate signals
	signal(SIGINT, quitHandler);
	signal(SIGTERM, quitHandler);
	signal(SIGKILL, quitHandler);
	signal(SIGQUIT, quitHandler);
	signal(SIGTSTP, quitHandler);
    
    memset(&server_addr, 0, sizeof(server_addr));
    addr = argv[1];
	port_number = atoi(argv[2]);
	
    client_socket = socket(PF_INET, SOCK_STREAM, 0);
    
    if(client_socket < 0)
	{
		error("Failed to open socket.");
		exit(EXIT_FAILURE);
	}
	
	server_addr.sin_addr.s_addr = inet_addr(addr);
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(port_number);
	
	//Connect to remote server
    if(connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        error("Failed to connect to the server.");
        exit(EXIT_FAILURE);
    }
    
    printf("Connected to %s.\n", inet_ntoa(server_addr.sin_addr));
    
    while(1)
    {
    	// clear buffer
    	memset(server_msg, 0, BUFFER_SIZE);
        len = recv(client_socket, server_msg, BUFFER_SIZE, 0);
        
        if(len > 0)
        {
        	if(strstr(server_msg, "Welcome") != NULL)
        	{
        		puts(server_msg);
        	}
        	else if(strstr(server_msg, "failed") != NULL)
        	{
        		puts(server_msg);
        	}
        	else
        	{
        		printf("%s", server_msg);
				fgets(msg, sizeof(msg), stdin);
				
				count = send(client_socket, msg, (strlen(msg)-1)/*remove the newline character*/, 0);
			
				if(count < 0)
				{
					error("Failed to send a message to server.");
					break;
				}
        	}
        }
        else
        {
        	error("Failed to receive a message from server.");
       		break;
		}
    }
	
	shutdown(client_socket, 2);
	exit(EXIT_SUCCESS);
}

void error(char* str)
{
  fprintf(stderr, "ERROR: %s\n", str);
}
