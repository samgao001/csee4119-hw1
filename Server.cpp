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
#include <time.h>

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>		
#include <arpa/inet.h> 
#include <pthread.h> 

using namespace std;

/******************* User Defines ********************************/

#define BLOCK_TIME			60
#define TIME_OUT			30

#define USER_FILE			"user_pass.txt"
#define MAX_USER			256
#define USER_PASS_SIZE		32
#define CLIENT_BUFF_SIZE	512

#define TIMEOUT_SECONDS		TIME_OUT * 60

typedef struct client_t
{
	int socket_id;
	string username;
	struct sockaddr_in client_addr;
	time_t time_stamp;
	pthread_t client_tr;
}client;

typedef struct blocked_info_t
{
	string username;
	map<string, time_t> ip_address;
}blocked_info;

/******************* Global Variables ****************************/
map<string, string> user_id;
map<string, client> login_users;

map<string, blocked_info> blocked_users;

int server_socket;
int client_socket;
int port_number;
int client_len;

/******************* Function Prototype **************************/
void load_user_info(void);
void error(string str);

void* client_handler(void* current);
void quitHandler(int signal_code);

/******************* Main program ********************************/
int main(int argc, char* argv[])
{
	int new_socket;
	struct sockaddr_in server_addr, client_addr;
	//int status;
	
	// check if the user specify a port for the server to use
	if(argc <2)
	{
		error("No port provided.\nUsage: ./Server <Port #>");
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
	
	listen(server_socket, user_id.size());
	
	// waiting for incoming connection
	cout << ">server address:  " << inet_ntoa(server_addr.sin_addr) << endl;
	cout << ">Waiting for incoming connections..." << endl;
	client_len = sizeof(struct sockaddr_in);
	
	while( (client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t*)&client_len)) )
	{	
		cout << ">Client(" << inet_ntoa(client_addr.sin_addr) << ") accepted." << endl;
		
		client new_client;
		pthread_t client_tr;
		
        new_client.socket_id = client_socket;
        new_client.client_tr = client_tr;
        new_client.client_addr = client_addr;
        
        if(pthread_create(&(new_client.client_tr), NULL, client_handler, &new_client) < 0)
        {
            error("Could not create thread.");
            shutdown(server_socket, SHUT_RDWR);
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

void quitHandler(int signal_code)
{
	for(map<string, client>::iterator itr=login_users.begin(); itr!=login_users.end(); ++itr)
	{
		shutdown((*itr).second.socket_id, SHUT_RDWR);
	}

	printf("\n>User terminated server process.\n");
	shutdown(server_socket, SHUT_RDWR);
	exit(EXIT_SUCCESS);
}

void error(string str)
{
	cout << ">ERROR: " << str << endl;
}

void load_user_info(void)
{
	string user = "";
	string pass = "";
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
		inFile >> user >> pass;
		
		if(user.size() != 0 || pass.size() != 0)
		{
			//end of the file reached, do not add user count
			user_id[user] = pass;
		}
	}
	
	// close file and free up memory
	inFile.close();
}

void* client_handler(void* cur_client)
{
	client current = *(client*) cur_client;
	ostringstream oss;
	int login_count = 0;
	bool logged_in = false;
	bool already_in = false;
	
	char msg[CLIENT_BUFF_SIZE];
	string username = "";
	string pre_username = "";
	string password = "";
	
	do
	{
		memset(msg, 0, CLIENT_BUFF_SIZE);
		sprintf(msg, ">Username: ");
		if(send(current.socket_id, msg, strlen(msg), 0) < 0)
    	{
    		error("Connection lost.");
        	break;
    	}
		// Receive a message from client 
		memset(msg, 0, CLIENT_BUFF_SIZE);
		if(recv(current.socket_id, msg, USER_PASS_SIZE, 0) > 0)
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
		
		sprintf(msg, ">Password: ");
		if(send(current.socket_id, msg, strlen(msg), 0) < 0)
    	{
    		error("Connection lost.");
        	break;
    	}
	
		// Receive a message from client
		memset(msg, 0, CLIENT_BUFF_SIZE);
		if(recv(current.socket_id, msg, USER_PASS_SIZE, 0) > 0)
		{
			password = string(msg);
		}
		else
		{
			error("Connection lost.");
        	break;
		}
		
		if(blocked_users.count(username) == 1)
		{
			string current_ip = string(inet_ntoa(current.client_addr.sin_addr));
			
			if(blocked_users[username].ip_address.count(current_ip) == 1)
			{
				time_t current_time;
				time(&current_time);			//get the current time
				
				double seconds = difftime(current_time, blocked_users[username].ip_address[current_ip]);
				
				if(seconds <= BLOCK_TIME)
				{
					login_count = 0;
					sprintf(msg, ">%s is blocked for %.1f second(s) from current ip.", username.c_str(), ((double)BLOCK_TIME - seconds));
				}
				else
				{
					// lift the block on this user from blocked list 
					// if there is only 1 ip block, 
					// otherwise remove the blocking ip
					if(blocked_users[username].ip_address.size() == 1)
					{
						blocked_users.erase(username);
					}
					else
					{
						blocked_users[username].ip_address.erase(current_ip);
					}
				}
			}
		}
		// check if the username exists in the list
		if(user_id.count(username) == 0)
		{
			login_count++;
		}
		else
		{
			// check the password associated with the username
			if(user_id[username].compare(password) != 0)
			{
				login_count++;
			}
			else if(blocked_users.count(username) == 0)
			{	
				if(login_users.count(username) == 1) 
				{
					logged_in = false;
					already_in = true;
					sprintf(msg, ">%s has already logged in.", username.c_str());
				}
				else 
				{
					current.username = username;
					logged_in = true;
					
					sprintf(msg, ">Logged in successfully. Welcome to TheChat!\n>Type --help for commands.");
				}
				login_count = 0;
			}
		}
		
		if(!logged_in)
		{
			if(login_count < 3 && !already_in && blocked_users.count(username) == 0)
			{
				sprintf(msg, ">Authentication failed, please try again. Attempt = %d.", login_count);
			}
			else if(!already_in && blocked_users.count(username) == 0)
			{
				blocked_info block_ip_user;	
				time_t time_stamp;
				time(&time_stamp);
				
				block_ip_user.username = username;
				block_ip_user.ip_address[string(inet_ntoa(current.client_addr.sin_addr))] = time_stamp;
				blocked_users[username] = block_ip_user;
				
				login_count = 0;
				sprintf(msg, ">Authentication failed 3 times, %s is blocked for %d second(s) from current ip.", username.c_str(), BLOCK_TIME);
			}
			else if(blocked_users.count(username) == 1)
			{
				//adding new blocking ip to existing bloced user
				string current_ip = string(inet_ntoa(current.client_addr.sin_addr));
				
				if(blocked_users[username].ip_address.count(current_ip) == 0)
				{
					time_t time_stamp;
					time(&time_stamp);
				
					blocked_users[username].ip_address[current_ip] = time_stamp;
					login_count = 0;
					sprintf(msg, ">Authentication failed 3 times, %s is blocked for %d second(s) from current ip.", username.c_str(), BLOCK_TIME);
				}
			}
		}
		
		if(send(current.socket_id, msg, strlen(msg), 0) < 0)
    	{
    		error("Connection lost.");
        	break;
    	}
		
	}while(!logged_in);
	
	if(logged_in)
	{
		login_users[username] = current;
		memset(msg, 0, CLIENT_BUFF_SIZE);
		sprintf(msg, ">%s: ", (current.username).c_str());
		if(send(current.socket_id, msg, strlen(msg), 0) < 0)
		{
			error("Connection lost.");
			logged_in = false;
		}
	}
	
	int msg_len = 0;
	
	while(logged_in)
	{
		// Receive a message from client
		memset(msg, 0, CLIENT_BUFF_SIZE);
		msg_len = recv(current.socket_id, msg, CLIENT_BUFF_SIZE, 0);
		if(msg_len > 0)
    	{
    		string rec_msg = string(msg);
    		
    		if((rec_msg.size() < 6) && (rec_msg.compare("nop") != 0))
    		{
    			memset(msg, 0, CLIENT_BUFF_SIZE);
    			sprintf(msg, ">Invalid command.\n>%s: ", (current.username).c_str());
    			
    			if(send(current.socket_id, msg, strlen(msg), 0) < 0)
				{
					error("Connection lost.");
					logged_in = false;
				}
    		}
    		else
    		{
    			if(rec_msg.compare("nop") == 0)
    			{
    				memset(msg, 0, CLIENT_BUFF_SIZE);
					sprintf(msg, ">%s: ", (current.username).c_str());
					if(send(current.socket_id, msg, strlen(msg), 0) < 0)
					{
						error("Connection lost.");
						logged_in = false;
					}
    			}
    			else if(rec_msg.compare("--help") == 0)
    			{
    				sprintf(msg, ">whoelse                  : Displays name of other connected users\n");
    				sprintf(msg, "%s>wholasthr                : Displays name of only those users that connected withint last hr.\n", msg);
					sprintf(msg, "%s>broadcast <message>      : Broadcast <message> too all connected users.\n", msg);
					sprintf(msg, "%s>message <user> <message> : Private <message> to a <user>.\n", msg);
					sprintf(msg, "%s>logout                   : log out this user.\n>%s: ", msg, (current.username).c_str());
					
					if(send(current.socket_id, msg, strlen(msg), 0) < 0)
					{
						error("Connection lost.");
						logged_in = false;
					}
    			}
    			else if(rec_msg.compare("logout") == 0)
    			{
    				logged_in = false;
    			}
    			else if(rec_msg.compare("whoelse") == 0)
    			{
    				memset(msg, 0, CLIENT_BUFF_SIZE);
     				for(map<string, client>::iterator itr=login_users.begin(); itr!=login_users.end(); ++itr)
 					{
 						if(itr == login_users.begin())
 							sprintf(msg, "\t%s\n", ((*itr).first).c_str());
 						else
 							sprintf(msg, "%s\t%s\n", msg, ((*itr).first).c_str());
 					}
 					sprintf(msg, "%s>%s: ", msg, (current.username).c_str());
 					
 					if(send(current.socket_id, msg, strlen(msg), 0) < 0)
 					{
 						error("Connection lost.");
 						logged_in = false;
 					}
    			}
    			else if(rec_msg.compare("wholasthr") == 0)
    			{
    				memset(msg, 0, CLIENT_BUFF_SIZE);
     				for(map<string, client>::iterator itr=login_users.begin(); itr!=login_users.end(); ++itr)
 					{
 						if(itr == login_users.begin())
 							sprintf(msg, "\t%s\n", ((*itr).first).c_str());
 						else
 							sprintf(msg, "%s\t%s\n", msg, ((*itr).first).c_str());
 					}
 					sprintf(msg, "%s>%s: ", msg, (current.username).c_str());
 					
 					if(send(current.socket_id, msg, strlen(msg), 0) < 0)
 					{
 						error("Connection lost.");
 						logged_in = false;
 					}
    			}
    			else
    			{	
    				int first_white_space = rec_msg.find_first_of(' ');
    				string command = rec_msg.substr(0, first_white_space);
    				string remain_msg = " "; 
    				
    				if(rec_msg.size() > first_white_space + 1)
					{
						remain_msg = rec_msg.substr(first_white_space + 1);
					}
    				
    				if(command.compare("broadcast") == 0)
    				{
    					for(map<string, client>::iterator itr=login_users.begin(); itr!=login_users.end(); ++itr)
						{
							string usr = (*itr).first;
							int usr_socket = (*itr).second.socket_id;
							memset(msg, 0, CLIENT_BUFF_SIZE);
    						sprintf(msg, "\n\t<BROADCAST> %s: %s\n", (current.username).c_str(), remain_msg.c_str());
							sprintf(msg, "%s>%s: ", msg, usr.c_str());
							
							if(send(usr_socket, msg, strlen(msg), 0) < 0)
							{
								cout << ">" << usr << " has lost connection." <<endl;
								login_users.erase(usr);
								shutdown(usr_socket, SHUT_RDWR);
							}
						}
    				}
    				else if(command.compare("message") == 0)
    				{
    					int next_white_space = remain_msg.find_first_of(' ');
    					string target = remain_msg.substr(0, next_white_space);
    					string msg_to_target = " ";
    					
    					if(remain_msg.size() > next_white_space + 1)
    					{
    						msg_to_target = remain_msg.substr(next_white_space + 1);
    					}

    					if(login_users.count(target) == 1)
    					{
    						memset(msg, 0, CLIENT_BUFF_SIZE);
    						sprintf(msg, "\n>%s: %s\n", (current.username).c_str(), msg_to_target.c_str());
    						if(send(login_users[target].socket_id, msg, strlen(msg), 0) < 0)
							{
								cout << ">" << target << " has lost connection." <<endl;
								shutdown(login_users[target].socket_id, SHUT_RDWR);
								login_users.erase(target);
							}
							
							memset(msg, 0, CLIENT_BUFF_SIZE);
							sprintf(msg, ">%s: ", target.c_str());
							if(send(login_users[target].socket_id, msg, strlen(msg), 0) < 0)
							{
								cout << ">" << target << " has lost connection." <<endl;
								shutdown(login_users[target].socket_id, SHUT_RDWR);
								login_users.erase(target);
							}
    					}
    					else
    					{
    						memset(msg, 0, CLIENT_BUFF_SIZE);
    						sprintf(msg, ">%s is currently offline.\n", target.c_str());
    						if(send(current.socket_id, msg, strlen(msg), 0) < 0)
							{
								error("Connection lost.");
								logged_in = false;
							}
    					}
    					
    					memset(msg, 0, CLIENT_BUFF_SIZE);
						sprintf(msg, ">%s: ", (current.username).c_str());
						if(send(current.socket_id, msg, strlen(msg), 0) < 0)
						{
							error("Connection lost.");
							logged_in = false;
						}
    				}
    			}
    		}
    	}
    	else
    	{
    		error("Connection lost.");
        	logged_in = false;
    	}
	}
	
	cout << ">" << username << " has exit the chat room." <<endl;
	login_users.erase(username);
	shutdown(current.socket_id, SHUT_RDWR);
	return 0;
}
