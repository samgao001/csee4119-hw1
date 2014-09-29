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
#include <signal.h>

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>		
#include <arpa/inet.h> 

#include <pthread.h> 

/******************* User Defines ********************************/

#define USER_FILE			"user_pass.txt"
#define MAX_USER			256
#define USER_PASS_SIZE		32
#define CLIENT_BUFF_LEN		512

#define BLOCK_TIME			60

/******************* Global Variables ****************************/
char user[MAX_USER][USER_PASS_SIZE];
char pass[MAX_USER][USER_PASS_SIZE];
char login_list[MAX_USER][USER_PASS_SIZE];

int user_count = 0;
int login_count = 0;

int server_socket;
int client_socket;
int port_number;
int client_len;
int* new_socket;

/******************* Function Prototype **************************/
void load_user_info(void);
void error(char* str);
int find_index(char (*str)[USER_PASS_SIZE], char* str1, int len);

void* client_handler(void*);
void quitHandler();

/******************* Main program ********************************/

void quitHandler()
{
	printf("\nUser teminated server process.\n");
	pthread_exit(NULL);
	shutdown(server_socket, 2);
	exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[])
{
	struct sockaddr_in server_addr, client_addr;
	//int status;
	
	// check if the user specify a port for the server to use
	if(argc <2)
	{
		error("No port provided.");
		exit(EXIT_FAILURE);
	}	
	
	// setup to capture process terminate signals
	signal(SIGINT, quitHandler);
	signal(SIGTERM, quitHandler);
	signal(SIGKILL, quitHandler);
	signal(SIGQUIT, quitHandler);
	signal(SIGTSTP, quitHandler);
	
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
		shutdown(server_socket, 2);
		exit(EXIT_FAILURE);
	}
	
	listen(server_socket, user_count);
	
	// waiting for incoming connection
	puts("Waiting for incoming connections...");
	client_len = sizeof(struct sockaddr_in);
	
	while( (client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t*)&client_len)) )
	{	
		printf("Client(%s) accepted.\n", inet_ntoa(client_addr.sin_addr));
		
		new_socket = malloc(1);
        *new_socket = client_socket;
        
        // creates a new thread for each new client trying to connect.
		pthread_t client_thread;
		
        if(pthread_create(&client_thread, NULL, client_handler, (void*) new_socket) < 0)
        {
            error("Could not create thread.");
            shutdown(server_socket, 2);
            exit(EXIT_FAILURE);
        }
        
        pthread_join(client_thread , NULL);
	}
	
	if(client_socket < 0)
	{
		error("Failed to accept client.");
		shutdown(server_socket, 2);
		exit(EXIT_FAILURE);
	}
	
	pthread_exit(NULL);
	shutdown(server_socket, 2);
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
		//printf("%s %s\n",user[user_count], pass[user_count]);
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

int find_index(char (*str)[USER_PASS_SIZE], char* str1, int len)
{
	int listed = false;
	int index = -1;
	
	for(int i = 0; i < len; i++)
	{
		listed = strcmp(str[i], str1);
		
		if (listed == 0)
		{
			index = i;
			break;
		}
	}
	
	return index;
}

void* client_handler(void* cli_socket)
{
	int socket_id = *(int*) cli_socket;
	int len;
	int login_count = 0;
	bool logged_in = false;
	
	char msg[CLIENT_BUFF_LEN];
	char client_msg[CLIENT_BUFF_LEN];
	
	int user_index;
	char username[CLIENT_BUFF_LEN] = "";
	char pre_username[CLIENT_BUFF_LEN] = "";
	char password[CLIENT_BUFF_LEN] = "";
	
	do
	{
		memset(msg, 0, CLIENT_BUFF_LEN);
		sprintf(msg, "Username: ");
		len = send(socket_id, msg, strlen(msg), 0);
		
		if(len < 0)
    	{
    		error("Connection lost.");
        	break;
    	}
		
		// Receive a message from client
		memset(client_msg, 0, CLIENT_BUFF_LEN);
		len = recv(socket_id, client_msg, USER_PASS_SIZE, 0);
		if(len > 0)
		{
			memset(username, 0, CLIENT_BUFF_LEN);
			memcpy(username, client_msg, len);
			
			if(strcmp(username, pre_username) != 0)
				login_count = 0;
				
			memset(pre_username, 0, CLIENT_BUFF_LEN);
			memcpy(pre_username, username, len);
		}
		else
		{
			error("Connection lost.");
        	break;
		}
		
		memset(msg, 0, CLIENT_BUFF_LEN);
		sprintf(msg, "Password: ");
		len = send(socket_id, msg, strlen(msg), 0);
		
		if(len < 0)
    	{
    		error("Connection lost.");
        	break;
    	}
	
		// Receive a message from client
		memset(client_msg, 0, CLIENT_BUFF_LEN);
		len = recv(socket_id, client_msg, USER_PASS_SIZE, 0);
		if(len > 0)
		{
			memset(password, 0, CLIENT_BUFF_LEN);
			memcpy(password, client_msg, len);
		}
		else
		{
			error("Connection lost.");
        	break;
		}

		user_index = find_index(user, username, user_count);
		
		// check if the username exists in the list
		if(user_index < 0)
		{
			login_count++;
		}
		else
		{
			// check the password associated with the username
			if(strcmp(pass[user_index], password) != 0)
			{
				login_count++;
			}
			else
			{
				logged_in = true;
				login_count = 0;
				memset(msg, 0, CLIENT_BUFF_LEN);
				sprintf(msg, "Logged in successfully. Welcome to TheChat!");
			}
		}
		
		if(!logged_in)
		{
			memset(msg, 0, CLIENT_BUFF_LEN);
			if(login_count < 3)
				sprintf(msg, "Authentication failed, please try again. Attempt = %d.", login_count);
			else
				sprintf(msg, "Authentication failed 3 times, you are now blocked for %d second(s).", BLOCK_TIME);
		}
		
		send(socket_id, msg, strlen(msg), 0);
		
	}while((!logged_in) && (login_count < 3));
	
	shutdown(socket_id, 2);
	return 0;
}
