#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <time.h>

using namespace std;

int init_socket();
vector<string> decode_response(char* response_buffer);
string encode_request(pair<char, string> keyval);
int get_from_server(string key,int socket_fd);
int delete_from_server(string key,int socket_fd);
int put_to_server(string key,int socket_fd);
void close_connection(int socket_fd);





vector<string> decode_response(char* response_buffer)
{

	  /********REQUEST MESSAGE HANDLING CODE**********/
	  string operation = string(1,response_buffer[0]);
	  string out = string(response_buffer+1,response_buffer+513);
	  int out_end = out.find('-');
	  string out_exact = out.substr(0,out_end);
	  
	  vector<string> result(2);
	  result[0] = operation;
	  result[1] = out_exact;
	  return result;
}


int get_from_server(char* key, char *value, char *error, int socket_fd){
	char request_buffer[513];
	char operation[2];
	operation[0] = '1';
	int status_code;
	// char key[256]; //Actual user entered key
	if(strlen(key)>256){
		status_code = -1;
		cout<<"\nKey size should not exceed 256 bytes/chars"<<endl;
		cout<<"STATUS RETURNED:Error:-1"<<endl;
		cout<<"ERROR MESSAGE RETURNED::Key size too large"<<endl;
		strcpy(error,string("\nKey size too large").c_str());
		return status_code;
	}
	else{
		int key_len = strlen(key);
		string key_str = key;
		/*Append key with padding to become 512B*/
		key_str.append(512 - key_str.length(), '-');
		/*Combine operation and padded request*/
		strcpy(request_buffer,operation);
		strcat(request_buffer,key_str.c_str());
	}

	write(socket_fd, request_buffer, 513);
	char response_buffer[1000];
	int len = read(socket_fd, response_buffer,999);
	response_buffer[len] = '\0';
	vector<string> response = decode_response(response_buffer);

	status_code = 256 + (int)response[0].at(0);
	if(status_code==240){
		cout<<"STATUS RETURNED:Error:"<<status_code<<endl;
		cout<<"ERROR MESSAGE RETURNED::"<<response[1]<<endl;
		strcpy(error,response[1].c_str());
	}
		
	else{
		cout<<"STATUS RETURNED:Success:"<<status_code<<endl;
		cout<<"VALUE RETURNED::"<<response[1]<<endl;
		strcpy(value,response[1].c_str());
	}

	return status_code;	


}


int delete_from_server(char* key, char *error,int socket_fd){
	char request_buffer[513];
	char operation[2];
	operation[0] = '3';
	int status_code;
	// char key[256]; //Actual user entered key
	if(strlen(key)>256){
		status_code = -1;
		cout<<"\nKey size should not exceed 256 bytes/chars"<<endl;
		cout<<"STATUS RETURNED:Error:-1"<<endl;
		cout<<"ERROR MESSAGE RETURNED::Key size too large"<<endl;
		strcpy(error,string("\nKey size too large").c_str());
		return status_code;
	}
	else{
		int key_len = strlen(key);
		string key_str = key;
		/*Append key with padding to become 512B*/
		key_str.append(512 - key_str.length(), '-');
		/*Combine operation and padded request*/
		strcpy(request_buffer,operation);
		strcat(request_buffer,key_str.c_str());
	}

	write(socket_fd, request_buffer, 513);
	char response_buffer[1000];
	int len = read(socket_fd, response_buffer,999);
	response_buffer[len] = '\0';
	vector<string> response = decode_response(response_buffer);

	status_code = 256 + (int)response[0].at(0);
	if(status_code==240){

		cout<<"STATUS RETURNED:Error:"<<status_code<<endl;
		cout<<"ERROR MESSAGE RETURNED::"<<response[1]<<endl;
		strcpy(error,response[1].c_str());
	}
		
	else{
		cout<<"STATUS RETURNED:Success:"<<status_code<<endl;
		cout<<"MESSAGE RETURNED::"<<response[1]<<endl;
	}

	return status_code;	
}


int put_to_server(char* key, char* value, char *error, int socket_fd){
	char request_buffer[513];
	char operation[2];
	operation[0] = '2';
	int status_code;
	if(strlen(key)>256){
		status_code = -1;
		cout<<"\nKey size should not exceed 256 bytes/chars"<<endl;
		cout<<"STATUS RETURNED:Error:-1"<<endl;
		cout<<"ERROR MESSAGE RETURNED::Key size too large"<<endl;
		strcpy(error,string("\nKey size too large").c_str());
		return status_code;
		// continue;
	}

	if(strlen(value)>256){
		status_code = -1;
		cout<<"\nValue size should not exceed 256 bytes/chars"<<endl;
		cout<<"STATUS RETURNED:Error:-1"<<"-1"<<endl;
		cout<<"ERROR MESSAGE RETURNED::"<<"Value too large"<<endl;
		strcpy(error,string("\nValue size too large").c_str());
		return status_code;
		// continue;
	}
	/*Pad key to 256B*/
	int key_len = strlen(key);
	string key_str = key;
	key_str.append(256 - key_str.length(), '-');
	
	/*Pad value to 256B*/
	int val_len = strlen(value);
	string val_str = value;
	val_str.append(256 - val_str.length(), '-');
	
	/*Combine operation and padded request and padded value*/
	strcpy(request_buffer,operation);
	strcat(request_buffer,key_str.c_str());
	strcat(request_buffer,val_str.c_str());

	write(socket_fd, request_buffer, 513);
	char response_buffer[1000];
	int len = read(socket_fd, response_buffer,999);
	response_buffer[len] = '\0';
	vector<string> response = decode_response(response_buffer);

	status_code = 256 + (int)response[0].at(0);
	if(status_code==240){
		cout<<"STATUS RETURNED:Error:"<<status_code<<endl;
		cout<<"ERROR MESSAGE RETURNED::"<<response[1]<<endl;
		strcpy(error,response[1].c_str());
	}
		
	else{
		cout<<"STATUS RETURNED:Success:"<<status_code<<endl;
		cout<<"MESSAGE RETURNED::"<<response[1]<<endl;
		
	}	


	return status_code;


}


void close_connection(int socket_fd){
	char request_buffer[513];
	char operation[2];
	operation[0] = '4';
	string keyval = "";
	keyval.append(512,'-');
	strcpy(request_buffer,operation);
	strcat(request_buffer,keyval.c_str());
	write(socket_fd, request_buffer, 513);
	char response_buffer[1000];
	int len = read(socket_fd, response_buffer,999);
	response_buffer[len] = '\0';
	vector<string> response = decode_response(response_buffer);
	close(socket_fd);

	return;

}