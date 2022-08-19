#include <string>
#include <string.h>
#include <unordered_map>
#include <utility>
#include <iostream>
#include <filesystem>

using namespace std;

class KVStore{
	unordered_map<string,long int> key_position;
	int key_count;
	int readerCount;
	pthread_mutex_t lock;
	pthread_cond_t reader_can_enter, writer_can_enter;
	bool writer_present;

public:
	KVStore(){
		key_count = 0;
		readerCount = 0;
		writer_present = false;
		pthread_cond_init(&reader_can_enter,NULL);
		pthread_cond_init(&writer_can_enter,NULL);
		pthread_mutex_init(&lock,NULL);
	}
	// return new poition of added or if already exists return existing position
	long int add_key(string key, long int pos){
		if(key_position.find(key) == key_position.end())
			key_count++;
		key_position[key] = pos;
		return pos;
	}

	long int get_key(string key){
		if(key_position.find(key) == key_position.end()){
			return -1;
		} 
		return key_position[key];
	}

	long int del_key(string key){
		if(key_position.find(key) == key_position.end()){
			// cout<<"del key not found in metadata"<<endl;
			return -1;
		} 
		else{
			// cout<<"del key found in metadata"<<endl;
			key_count--;
			key_position.erase(key);
		}
		return 0;
	}
	int get_key_count(){
		return key_count;
	}

	void read_lock(){
		pthread_mutex_lock(&lock);
		while(writer_present)
			pthread_cond_wait(&reader_can_enter,&lock);
		readerCount++;
		pthread_mutex_unlock(&lock);
	}

	void read_unlock(){
		pthread_mutex_init(&lock,NULL);
		pthread_mutex_lock(&lock);
		readerCount--;
		if(readerCount==0)
			pthread_cond_signal(&writer_can_enter);
		pthread_mutex_unlock(&lock);
	}
	void write_lock(){
		pthread_mutex_lock(&lock);
		while(writer_present || readerCount!=0)
			pthread_cond_wait(&writer_can_enter,&lock);
		writer_present = true;
		pthread_mutex_unlock(&lock);
	}
	void write_unlock(){
		pthread_mutex_lock(&lock);
		writer_present = false;
		pthread_cond_broadcast(&reader_can_enter);
		pthread_cond_signal(&writer_can_enter);
		pthread_mutex_unlock(&lock);
	}

};

// store KVStore files name and class object instance
unordered_map<string,KVStore *> KVStore_files;

pair<string,string> get_filename_key(string key){
	string KVKey;
	string KVFilename;
	if(key.length() <= 4){
		KVKey = key;
		KVFilename = "KVStore_";
	}
	else{
		int key_len = key.length();
		KVKey = key.substr(key_len-4,4);
		KVFilename = "KVStore_" + key.substr(0,key_len-4);
	}
	return make_pair(KVFilename,KVKey);
}

KVStore *get_file_obj(string filename){
	if(KVStore_files.find(filename)==KVStore_files.end()){
		//first time creating file
		remove(filename.c_str());
		KVStore_files[filename] = new KVStore();
		return KVStore_files[filename];
	}

	return KVStore_files[filename];
}

void print_KVStore_file(){
	unordered_map<string,KVStore *>::iterator itr; 
	for (itr = KVStore_files.begin(); itr != KVStore_files.end(); ++itr) { 
        cout << itr->first << '\t' << (itr->second)->get_key_count() << '\n'; 
    } 
}
