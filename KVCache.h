#include <iostream>
#include <list>
#include <utility>
#include <unordered_map>

#include "KVServer.h"

using namespace std;

class KVCache_pair{
	string key;
	string value;
	int readerCount;
	pthread_mutex_t lock;
	pthread_cond_t reader_can_enter, writer_can_enter;
	bool writer_present;

public:
	KVCache_pair(){
		readerCount = 0;
		writer_present = false;
		pthread_cond_init(&reader_can_enter,NULL);
		pthread_cond_init(&writer_can_enter,NULL);
		pthread_mutex_init(&lock,NULL);
	}
	KVCache_pair(string k,string v){
		readerCount = 0;
		writer_present = false;
		pthread_cond_init(&reader_can_enter,NULL);
		pthread_cond_init(&writer_can_enter,NULL);
		pthread_mutex_init(&lock,NULL);
		key = k;
		value = v;
	}
	string getKey(){
		return key;
	}
	string getValue(){
		return value;
	}
	void setKey(string k){
		key = k;
	}
	void setValue(string v){
		value = v;
	}
	void read_lock(){
		pthread_mutex_lock(&lock);
		while(writer_present)
			pthread_cond_wait(&reader_can_enter,&lock);
		readerCount++;
		pthread_mutex_unlock(&lock);
	}
	void read_unlock(){
		pthread_mutex_lock(&lock);
		readerCount--;
		if(readerCount==0)
			pthread_cond_signal(&writer_can_enter);
		pthread_mutex_unlock(&lock);
	}
	void writer_lock(){
		pthread_mutex_lock(&lock);
		while(writer_present || readerCount!=0)
			pthread_cond_wait(&writer_can_enter,&lock);
		writer_present = true;
		pthread_mutex_unlock(&lock);
	}
	void writer_unlock(){
		pthread_mutex_lock(&lock);
		writer_present = false;
		pthread_cond_broadcast(&reader_can_enter);
		pthread_cond_signal(&writer_can_enter);
		pthread_mutex_unlock(&lock);
	}
};

class KVCache{
	list< KVCache_pair > KVCache_list;
	unordered_map<string, list< KVCache_pair > :: iterator > cache_map;
	int cache_size;

public:
	KVCache(int n);
	pair<char, string> KVCache_put(string key,string value);
	pair<char, string> KVCache_get(string key);
	pair<char, string> KVCache_del(string key);
private:
	KVCache_pair KVCache_replace();
	string KVCache_remove(string key);
	int KVCache_insert(string key,string value);
};

KVCache::KVCache(int n){
	cache_size = n;
}
KVCache_pair KVCache::KVCache_replace(){
	KVCache_pair replaced_KV = KVCache_list.back();
	replaced_KV.writer_lock();
	KVCache_list.pop_back();
	cache_map.erase(replaced_KV.getKey());
	replaced_KV.writer_unlock();
	return replaced_KV;
}

string KVCache::KVCache_remove(string key){
	if(cache_map.find(key) == cache_map.end())
		return NULL;
	KVCache_pair kvpair = (*cache_map[key]);
	kvpair.writer_lock();
	KVCache_list.erase(cache_map[key]);
	cache_map.erase(key);
	kvpair.writer_unlock();
	return kvpair.getValue();
}

int KVCache::KVCache_insert(string key, string value){
	KVCache_pair kvpair(key,value);
 	KVCache_list.push_front(kvpair);
	cache_map[key] = KVCache_list.begin();
	return 0;
}

pair<char, string> KVCache::KVCache_put(string key,string value){
	pair<char, string> status_val;	
	if(cache_map.find(key) == cache_map.end()){
		if(KVCache_list.size() == cache_size)
			KVCache_replace();
	}
	else{
		// cout<<"found in cache\n";
		KVCache_remove(key);
	}
	KVCache_insert(key,value);
	status_val = put(key,value);
	return status_val;
}

pair<char, string> KVCache::KVCache_get(string key){
	pair<char, string> status_val;
	string value;
	if(cache_map.find(key) == cache_map.end()){
		//GET from KVStore if not found return null else proceed
		// cout<<"not found in cache\n";
		status_val = get(key);

		if(status_val.first == 200){
			// cout<<"found in store\n";
			if(KVCache_list.size() == cache_size){
				KVCache_replace();
			}
			KVCache_insert(key,value);
		}
	}
	else{
		// cout<<"found in cache\n";
		value = KVCache_remove(key);
		KVCache_insert(key,value);
		status_val.first = 200;
		status_val.second = value;
	}
	return status_val;
}

pair<char, string> KVCache::KVCache_del(string key){
	pair<char, string> status_val;
	string value;
	if(cache_map.find(key) == cache_map.end()){
		// cout<<"not found in cache\n";
	}
	else{
		// cout<<"found in cache\n";
		value = KVCache_remove(key);
	}
	// in any case need to del from KVStore so calling here
	status_val = deletekv(key);
	return status_val;
}