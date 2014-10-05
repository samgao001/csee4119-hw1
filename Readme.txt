/*******************************************************************
**	File name: 		Readme.txt
**	Author:			Xusheng Gao (xg2193)
**  Description:	This file describes how simple chat server work.
**
*******************************************************************/

>>>>>>>>>>>>>>>>>>>>>>> Program Description <<<<<<<<<<<<<<<<<<<<<<<<<

Server.cpp is the source code to manage all the client connections 
and pass on private messages to target client. It also keep track of
which user is blocked on a specific ip address. If the user is not 
active for a period of time define by TIME_OUT, then the server will
force logout that user. 

All the client handling is done by multi-threading. In addition, 
timeout handling is done in a separate thread to keep track of all
the clients. 

Client.cpp is the source code to manage client connection. Only one 
user can log in from one console window. Login sequence is done by 
pulling. Once logged in, a thread is created to handle incoming 
message from the server. Communication to other client user is 
through server.

>>>>>>>>>>>>>>>>>>>>> Development Environment <<<<<<<<<<<<<<<<<<<<<<<

Both server and client is develop under ubuntu 12.04LTS in a virtual
machine. It is compile and build with g++ 4.6.3.

>>>>>>>>>>>>>>>>>>>>>>>>>>>>> HOW TO <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

First make sure you have all the require files. 
	Server.cpp
	Client.cpp
	makefile
	user_pass.txt
	
Then open a terminal, get into the directory containing all the files.

user following command to build and compile the source code.
	>make rebuild

To run server:
	> ./Server <PORT #>
	
	This will display init message for the server and it is listening 
	to any ip address assign to this server computer. It is display as
	(0.0.0.0). It will then display any incoming connection info as 
	well as termination info. 
	
	Use Key combination CTRL + C to terminate server process.
	
To run client:
	> ./Client <Server IP> <PORT #>
	
	Once launch and connect correctly, server should prompt to as user
	to input username and password. Once log in to the chat room, you 
	can use supported commands to perform different task. Type "--help"
	to display all the supported command if needed.
	
	if you want to exit without login, simply type "exit" and enter to 
	quit. Or use CTRL + C key combinations.
	
Examples of supported commands:

	>columbia: whoelse
		google
		windows
	>columbia: 

	>columbia wholasthr
		windows
	>columbia: 
		
	>columbia: broadcast test
		<BROADCASR> columbia: test
	>columbia: 
	
	One sender screen
	>columbia: message google hi
	>columbia: 
	
	>google: (this is still there because the limitation of console display
				better refinement can be done with GUI interface.)
	>columbia: hi
	>google: 
	
	>columbia: logout
	(this will logout columbia and exit the client process. You will 
	have to launch new client process to log in a new user.)