#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <vector>
#include <pthread.h>
#include <queue>
#include <string>
#include <fstream>
#include <utility>
#include "KVStore.h"
#include "config.h"

using namespace std;

int init_socket();
int processRequest(int client_fd);
pair<char, string> put(string key,string val);
pair<char, string> get(string key);
pair<char, string> deletekv(string key);
vector<string> decode_request(char* request_buffer);
string encode_from_server(pair<char, string> return_values);

int init_socket(){
	int socket_fd,s;
	socket_fd = socket(AF_INET,SOCK_STREAM,0);

	struct addrinfo hints, *serverAdd;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	s = getaddrinfo(NULL, LISTENING_PORT , &hints, &serverAdd);
	if (s != 0){
	        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        	exit(1);
	}
	if ( bind(socket_fd, serverAdd->ai_addr, serverAdd->ai_addrlen) != 0 ){
		perror("bind()");
		exit(1);
	}

	if ( listen(socket_fd, 10) != 0 ){
		perror("listen()");
		exit(1);
	}
	return socket_fd;
}

pair<char, string> put(string key,string val)
{
	pair<string,string> fn_key = get_filename_key(key);
	string filename = fn_key.first;
	string key_exact = fn_key.second;

	KVStore *kvs = get_file_obj(filename);
	
	ofstream file,temp;
	string key_val;
	pair<char, string> status_val;
	long pos =kvs->get_key(key_exact);

	if(pos==-1){
		kvs->write_lock();
		file.open(filename,ios::app);
		pos = file.tellp();
		file<<key_exact+","+val<<endl;
		file.close();
		kvs->add_key(key_exact,pos);
		kvs->write_unlock();
	}
	else{
		pair<char, string> dummy = deletekv(key);
		return put(key,val);	
	}

	status_val.first = 200;
	status_val.second = "Put successful";
	return status_val;
}

pair<char, string> get(string key)
{
	
	pair<string,string> fn_key = get_filename_key(key);
	string filename = fn_key.first;
	string key_exact = fn_key.second;
	pair<char, string> status_val;

	KVStore *kvs = get_file_obj(filename);

	fstream file;
	string key_val;
	
	long pos = kvs->get_key(key_exact);
	if(pos == -1){
		status_val.first = 240;
		status_val.second = "Key does not exist";
	}
	else
	{
		
		kvs->read_lock();
		file.open(filename,ios::in);
		file.seekg(pos,ios::beg);
		file>>key_val;
		//kvs.read_unlock();
		int key_end = key_val.find(',');
		string key_in_file = key_val.substr(0,key_end);
		int val_end = key_val.find('\n');
		string val_in_file = key_val.substr(key_end+1,val_end);
		status_val.first = 200;
		status_val.second = val_in_file;
		file.close();
		kvs->read_unlock();
		
	}
	return status_val;
	
}

pair<char, string> deletekv(string key)
{
	pair<string,string> fn_key = get_filename_key(key);
	string filename = fn_key.first;
	string key_exact = fn_key.second;
	pair<char, string> status_val;

	KVStore *kvs = get_file_obj(filename);

	fstream file;
	string key_val;
	long pos = kvs->get_key(key_exact);
	if(pos == -1){
		status_val.first = 240;
		status_val.second = "Key does not exist";
	}
	else{
		kvs->write_lock();
		// file.open(filename,ios::out);
		file.open(filename,ios::out | ios::in);
		file.seekg(pos,ios::beg);
		file>>key_val;
		int key_end = key_val.find(',');
		int len = key_val.length();
		file.seekp(pos,ios::beg);
		for (int i = 0; i < len; ++i)
			file<<"-";
		kvs->del_key(key_exact);
		status_val.first = 200;
		status_val.second = "successful";
		file.close();
		kvs->write_unlock();
	}
	return status_val;
}

// string[] decode_request(char[] request_buffer){

vector<string> decode_request(char* request_buffer)
{

	  /********REQUEST MESSAGE HANDLING CODE**********/
	  string operation = string(1,request_buffer[0]);


	  string key = string(request_buffer+1,request_buffer+257);
	  int key_end = key.find('-');
	  string key_exact = key.substr(0,key_end);
	  
	  string val_exact = "";
	  if(request_buffer[0]=='2'){
	  	string val = string(request_buffer+257,request_buffer+513);
	  	int val_end = val.find('-');
	  	val_exact = val.substr(0,val_end);
	  }

	  vector<string> result(3);
	  result[0] = operation;
	  result[1] = key_exact;
	  result[2] = val_exact;
	  return result;
}

string encode_from_server(pair<char, string> return_values)
{

	char response[513];
	char status[2];
	status[0] = return_values.first;
	string status_str = status;
	string val = return_values.second;
	val.append(512 - val.length(), '-');
	strcpy(response,status_str.c_str());
	strcat(response,val.c_str());

	string returning = response;
	return returning;
}

