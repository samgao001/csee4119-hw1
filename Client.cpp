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
#include <string>
#include <iostream>
#include <sstream>

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>		
#include <arpa/inet.h> 
#include <pthread.h> 

using namespace std;

/******************* User Defines ********************************/
#define BUFFER_SIZE				512

/******************* Global Variables ****************************/
int client_socket;
int port_number;

/******************* Function Prototype **************************/
void error(string str);
void quitHandler(int exit_code);

/******************* Main program ********************************/

void quitHandler(int signal_code)
{
	shutdown(client_socket, SHUT_RDWR);
	cout << endl << ">User terminated client process." << endl;	
	exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[])
{
	struct sockaddr_in server_addr;
	string addr;
    string server_msg = "";
    char msg[BUFFER_SIZE];
    bool logout = false;
    
    if (argc < 3) {
    	error("Did not specify address and port number.\n>Usage: ./Client <IP Address> <Port #>");
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
	
	server_addr.sin_addr.s_addr = inet_addr(addr.c_str());
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(port_number);
	
	//Connect to remote server
    if(connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        error("Failed to connect to the server.");
        exit(EXIT_FAILURE);
    }
    
    cout << ">Connected to " << inet_ntoa(server_addr.sin_addr) << endl;
    
    while(!logout)
    {
    	// clear buffer
    	memset(msg, 0, BUFFER_SIZE);
        if(recv(client_socket, msg, BUFFER_SIZE, 0) > 0)
        {
        	server_msg = string(msg);
        	if(server_msg.find("Welcome") != string::npos)
        	{
        		cout << server_msg << endl;
        	}
        	else if(server_msg.find("failed") != string::npos)
        	{
        		cout << server_msg << endl;
        	}
        	else if(server_msg.find("already logged in") != string::npos)
        	{
        		cout << server_msg << endl;
        	}
        	else
        	{
        		string msg1;
        		cout << server_msg;
				getline(cin, msg1);
				strcpy(msg, msg1.c_str());
				
				if(strlen(msg) == 0)
				{
					sprintf(msg, "nop");
				}
				
				if(send(client_socket, msg, strlen(msg), 0) < 0)
				{
					error("Connection lost, please reconnect.");
					logout = true;
				}
				
				if(msg1.compare("logout") == 0)
				{
					logout = true;
				}
        	}
        }
        else
        {
        	error("Connection lost, please reconnect.");
       		logout = true;
		}
    }
	
	cout << ">Exited chat room." << endl;
	shutdown(client_socket, SHUT_RDWR);
	exit(EXIT_SUCCESS);
}

void error(string str)
{
	cout << ">ERROR: " << str << endl;
}
