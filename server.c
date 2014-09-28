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

#define USER_FILE			"user_pass.txt"
#define MAX_USER_NUMBER		256
#define PASS_SIZE			32

char user[MAX_USER_NUMBER][PASS_SIZE];
char pass[MAX_USER_NUMBER][PASS_SIZE];
int user_count;

void load_user_info(void);
void error(char* str);

int main(int argc, char* argv[])
{
	int socket_id, new_socket_id, port_number, client_len;
	char buffer[256];
	struct sockaddr_in server_addr, client_addr;
	
	if(argc <2)
	{
		error("No port provided");
		exit(EXIT_FAILURE);
	}	

	printf("%s\n", argv[1]);

	load_user_info();

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

	//close file and free up memory
	fclose(inFile);
}
