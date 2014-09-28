/*****************************************************************
**	File name: 		server.c
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

#define USER_FILE			"user_pass.txt"
#define MAX_USER			256
#define USER_PASS_SIZE		32
#define CLIENT_BUFF_LEN		1024

#define BLOCK_TIME			60

/******************* Global Variables ****************************/
char user[MAX_USER][USER_PASS_SIZE];
char pass[MAX_USER][USER_PASS_SIZE];
char login_list[MAX_USER][USER_PASS_SIZE];

int user_count = 0;
int login_count = 0;

/******************* Function Prototype **************************/
void load_user_info(void);
void error(char* str);
int find_index(char* str[], char* str1, int len);

void* client_handler(void*);

/******************* Main program ********************************/

int main(int argc, char* argv[])
{
	int server_socket, client_socket, port_number, client_len;
	int* new_socket;
	struct sockaddr_in server_addr, client_addr;
	//int status;
	
	// check if the user specify a port for the server to use
	if(argc <2)
	{
		error("No port provided.");
		exit(EXIT_FAILURE);
	}	
	
	// Load user name and password to the buffer
	load_user_info();

	// try to open a socket
	server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if(server_socket < 0)
	{
		error("Failed to open socket.");
		exit(EXIT_FAILURE);
	}
	
	// initiate structure for server and client as well as getting port number
	memset(&server_addr, 0, sizeof(server_addr));
	memset(&client_addr, 0, sizeof(client_addr));
	port_number = atoi(argv[1]);
	
	// setup server_addr structure
	server_addr.sin_family = PF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port_number);
	
	// attempt to bind the socket
	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
	{
		error("Failed to bind.");
		exit(EXIT_FAILURE);
	}
	
	listen(server_socket, user_count);
	
	// waiting for incoming connection
	puts("Waiting for incoming connections...");
	client_len = sizeof(struct sockaddr_in);
	
	while( (client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t*)&client_len)) )
	{	
		puts("Client accepted");
		
		new_socket = malloc(1);
        *new_socket = client_socket;
        
        // creates a new thread for each new client trying to connect.
		pthread_t client_thread;
		
        if(pthread_create(&client_thread, NULL, client_handler, (void*) new_socket) < 0)
        {
            error("Could not create thread.");
            exit(EXIT_FAILURE);
        }
        
        pthread_join(client_thread , NULL);
	}
	
	if(client_socket < 0)
	{
		error("Failed to accept client.");
		exit(EXIT_FAILURE);
	}
	
	pthread_exit(NULL);
	
	exit(EXIT_SUCCESS);
}

void error(char* str)
{
  fprintf(stderr, "ERROR: %s\n", str);
}

void load_user_info(void)
{
	FILE* inFile;
	int item_read = 0;
	user_count = 0;

	inFile = fopen(USER_FILE, "r");
	if(inFile == NULL)
	{
	   error("Failed to open input file, please check if such file exist.");
	   exit(EXIT_FAILURE);
	}

	//keep reading until end of the file
	while(!feof(inFile)) 
	{
		item_read = fscanf(inFile, "%s %s", user[user_count], pass[user_count]);
		user_count++;
		
		// check if it reads an empty line
		if(item_read == -1)
		{
			// decrement the line count whenever an empty line is reached
			// or end of the file is reached. if eof terminates the loop.
			user_count--;
			if(feof(inFile)) break;
		}
		else if(item_read != 2)
		{
			error("Failed to read user info.");
			exit(EXIT_FAILURE);
			break;
		}
	}

	// close file and free up memory
	fclose(inFile);
}

int find_index(char* str[], char* str1, int len)
{
	bool listed = false;
	int index = -1;
	
	for(int i = 0; i < len; i++)
	{
		listed = strcmp(str[i], str1);
		
		if (listed)
		{
			index = i;
			break;
		}
	}
	
	return index;
}

void* client_handler(void* client_socket)
{
	int socket_id = *(int*) client_socket;
	int len;
	char* msg, client_msg[CLIENT_BUFF_LEN];
	int user_index;
	
	msg = "Username: ";
	send(socket_id, msg, strlen(msg), 0);
	
	// Receive a message from client
    len = recv(socket_id, client_msg, USER_PASS_SIZE, 0);
    
    if(len > 0)
    {
    	user_index = find_index(user, &client_msg);
    }

	return 0;
}
