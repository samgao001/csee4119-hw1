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
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>		
#include <arpa/inet.h> 
#include <pthread.h> 

using namespace std;

/******************* User Defines ********************************/

#define USER_FILE			"user_pass.txt"
#define MAX_USER			256
#define USER_PASS_SIZE		32
#define CLIENT_BUFF_SIZE	512
#define BLOCK_TIME			60

typedef struct server_t
{
	int socket_id;
	string username;
	string password;
	time_t time_stamp;
	pthread_t client_tr;
}server;

/******************* Global Variables ****************************/
string user[MAX_USER];
string pass[MAX_USER];

map<string, server> login_users;

pthread_t client_tr[MAX_USER];
int thread_count = 0;

int user_count = 0;
int user_online = 0;

int server_socket;
int client_socket;
int port_number;
int client_len;

/******************* Function Prototype **************************/
void load_user_info(void);
void error(string str);
int find_index(string str[MAX_USER], string str1, int len);

void* client_handler(void*);
void quitHandler(int signal_code);

/******************* Main program ********************************/

void quitHandler(int signal_code)
{
	printf("\nUser terminated server process.\n");
	shutdown(server_socket, SHUT_RDWR);
	exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[])
{
	int new_socket;
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
		shutdown(server_socket, SHUT_RDWR);
		exit(EXIT_FAILURE);
	}
	
	listen(server_socket, user_count);
	
	// waiting for incoming connection
	cout << "server address : " << inet_ntoa(server_addr.sin_addr) << endl;
	cout << "Waiting for incoming connections..." << endl;
	client_len = sizeof(struct sockaddr_in);
	
	while( (client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t*)&client_len)) )
	{	
		cout << "Client(" << inet_ntoa(client_addr.sin_addr) << ") accepted." << endl;
        new_socket = client_socket;
        
        if(pthread_create(&client_tr[thread_count], NULL, client_handler, &new_socket) < 0)
        {
            error("Could not create thread.");
            shutdown(server_socket, SHUT_RDWR);
        }
        else
        {
        	thread_count++;
        }
	}
	
	if(client_socket < 0)
	{
		error("Failed to accept client.");
		shutdown(server_socket, SHUT_RDWR);
		exit(EXIT_FAILURE);
	}
	
	pthread_exit(NULL);
	shutdown(server_socket, SHUT_RDWR);
	exit(EXIT_SUCCESS);
}

void error(string str)
{
	cout << "ERROR: " << str << endl;
}

void load_user_info(void)
{
	user_count = 0;
	
	ifstream inFile;
	inFile.open(USER_FILE);
	
	if(!(inFile.is_open()))
	{
	   error("Failed to open input file, please check if such file exist.");
	   exit(EXIT_FAILURE);
	}

	//keep reading until end of the file
	while(!(inFile.eof())) 
	{
		inFile >> user[user_count] >> pass[user_count];
		/*cout << user[user_count] << ", " << user[user_count].size() << ", ";
		cout << pass[user_count] << ", " << pass[user_count].size() << endl;*/
		if(user[user_count].size() != 0 || pass[user_count].size() != 0)
		{
			//end of the file reached, do not increment user count
			user_count++;
		}
	}

	// close file and free up memory
	inFile.close();
}

int find_index(string str[MAX_USER], string str1, int len)
{
	int index = -1;
	
	for(int i = 0; i < len; i++)
	{		
		if ((str[i].compare(str1)) == 0)
		{
			index = i;
			break;
		}
	}
	return index;
}

void* client_handler(void* cli_socket)
{
	ostringstream oss;
	int socket_id = *(int*) cli_socket;
	int login_count = 0;
	bool logged_in = false;
	
	char msg[CLIENT_BUFF_SIZE];
	int user_index;
	string username = "";
	string pre_username = "";
	string password = "";
	
	do
	{
		memset(msg, 0, CLIENT_BUFF_SIZE);
		sprintf(msg, "Username: ");
		if(send(socket_id, msg, strlen(msg), 0) < 0)
    	{
    		error("Connection lost.");
        	break;
    	}
		// Receive a message from client 
		memset(msg, 0, CLIENT_BUFF_SIZE);
		if(recv(socket_id, msg, USER_PASS_SIZE, 0) > 0)
		{
			username = string(msg);
			if(username.compare(pre_username) != 0)
				login_count = 0;
			pre_username = username;
		}
		else
		{
			error("Connection lost.");
        	break;
		}
		
		sprintf(msg, "Password: ");
		if(send(socket_id, msg, strlen(msg), 0) < 0)
    	{
    		error("Connection lost.");
        	break;
    	}
	
		// Receive a message from client
		memset(msg, 0, CLIENT_BUFF_SIZE);
		if(recv(socket_id, msg, USER_PASS_SIZE, 0) > 0)
		{
			password = string(msg);
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
			if(pass[user_index].compare(password) != 0)
			{
				login_count++;
			}
			else
			{
				logged_in = true;
				login_count = 0;
				sprintf(msg, "Logged in successfully. Welcome to TheChat!");
			}
		}
		
		if(!logged_in)
		{
			if(login_count < 3)
				sprintf(msg, "Authentication failed, please try again. Attempt = %d.", login_count);
			else
				sprintf(msg, "Authentication failed 3 times, you are now blocked for %d second(s).", BLOCK_TIME);
		}
		
		if(send(socket_id, msg, strlen(msg), 0) < 0)
    	{
    		error("Connection lost.");
        	break;
    	}
		
	}while((!logged_in) && (login_count < 3));
	
	while(logged_in)
	{
		memset(msg, 0, CLIENT_BUFF_SIZE);
		sprintf(msg, "%s :", username.c_str());
		if(send(socket_id, msg, strlen(msg), 0) < 0)
    	{
    		error("Connection lost.");
        	break;
    	}
		
		// Receive a message from client
		memset(msg, 0, CLIENT_BUFF_SIZE);
		if(recv(socket_id, msg, CLIENT_BUFF_SIZE, 0) < 0)
    	{
    		error("Connection lost.");
        	break;
    	}
    	else
    	{
    		cout << msg;
    	}
	}
	
	shutdown(socket_id, SHUT_RDWR);
	return 0;
}
