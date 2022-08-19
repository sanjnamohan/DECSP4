#include "KVClient.h"
#include <vector>
#include <time.h>

#define LISTENING_PORT "1234"

using namespace std;
vector<string> decode_response(char* response_buffer);


int main(){
	int socket_fd;
	socket_fd = init_socket();
	char request_buffer[513]; 
	char operation[2];
	struct timespec start, finish;
	double elapsed;
	char value[500],error[500];

	// /**************************START OF MESSAGE HANDLING**************************/
	while(1){

		while(1){

			cout<<"Select the operation(1/2/3):\n1) GET \n2) PUT\n3) DELETE\n4) CLOSE CONNECTION\n";
			cin>>operation[0];
			cin.ignore(100, '\n'); 
			
			if(operation[0]=='1' || operation[0]=='3'){
				char key[256]; //Actual user entered key
				cout<<"\nEnter the key: ";
				cin>>key;
				if(operation[0]=='1')
					get_from_server(key,value,error,socket_fd);
				else
					delete_from_server(key,error,socket_fd);
				break;
			}
			else if(operation[0]=='2'){
				char key[256],value[256]; //Actual user entered key
				cout<<"\nEnter the key: ";
				cin>>key;
				cout<<"\nEnter the value: ";
				cin>>value;

				put_to_server(key,value,error,socket_fd);

				break;
			}
			else if(operation[0]=='4'){
				close_connection(socket_fd);
				return 0;
			}
			else{
				cout<<"\nEnter a valid operation";
				break;
			}
		}

	}

	return 0;
}

int init_socket(){
	int socket_fd,s;
	socket_fd = socket(AF_INET,SOCK_STREAM,0);

	struct addrinfo hints, *serverAdd;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	// hints.ai_addr = "127.0.0.1";

	s = getaddrinfo(NULL, LISTENING_PORT, &hints, &serverAdd);
	if (s != 0){
	        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        	exit(1);
	}
	connect(socket_fd,serverAdd->ai_addr, serverAdd->ai_addrlen);
	return socket_fd;
}


