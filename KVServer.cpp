// #include "KVServer.h"
#include "KVCache.h"

using namespace std;

vector<pthread_cond_t> client_added_cond(THREAD_POOL_SIZE_INITIAL);
vector<pthread_mutex_t> client_added_lock(THREAD_POOL_SIZE_INITIAL);
vector<vector<int>> clients(THREAD_POOL_SIZE_INITIAL);
KVCache myCache(CACHE_SIZE);

void *worker_thread(void *id){
	int tid = *((int *)id);
	int no_of_clients = 0;
	
	int epollfd,nfds,client_status=0;
	struct epoll_event curr_event, events[MAX_EVENTS];

	pthread_mutex_lock(&client_added_lock[tid]);
	while(clients[tid].size()==0){
		cout<<"Waiting for first client"<<endl;
		pthread_cond_wait(&client_added_cond[tid],&client_added_lock[tid]);
	}
	no_of_clients++;

	epollfd = epoll_create1(0);
	if(epollfd==-1){
		perror("epoll_create1");
		exit(1);
	}

	curr_event.events = EPOLLIN;
	curr_event.data.fd = clients[tid][0];

	// adding first client to monitor 
	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, clients[tid][0], &curr_event) == -1){
		perror("epoll_ctl");
		exit(1);
	}
	// cout<<"ADDED first CLIENT to thread : "<<tid<<endl;

	while(1){
		cout<<"waiting for event thread"<<tid<<endl;
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, 5000);
		if(nfds==-1){
			perror("epoll_wait");
			exit(1);
		}
		for(int i=0 ; i<nfds ; i++){
			client_status = processRequest(events[i].data.fd);
			if(client_status == 1){
				// client connection is closed so remove it
				for (int i = 0; i < clients[tid].size(); ++i){
					if(clients[tid][i] == events[i].data.fd){
						clients[tid].erase(clients[tid].begin()+i);
						no_of_clients--;
						close(events[i].data.fd);
						cout<<"client connection closed"<<endl;
						break;
					}
				}
			}
			client_status = 0;
		}
		// to check new clients are added or not
		if(clients[tid].size() > no_of_clients){
			for(int i=no_of_clients; i<clients[tid].size() ; i++){
				// cout<<"ADDING A CLIENT to thread : "<<tid << " client "<<clients[tid][i]<<endl;
				curr_event.events = EPOLLIN;
				curr_event.data.fd = clients[tid][i];
				// adding clients to monitor
				if(epoll_ctl(epollfd, EPOLL_CTL_ADD, clients[tid][i], &curr_event) == -1){
					perror("epoll_ctl");
					exit(1);
				}
				no_of_clients++;
			}
		}
	}

	return NULL;
}

int main(int argc, char const *argv[])
{
	pthread_t* threads;
	int tid[THREAD_POOL_SIZE_INITIAL];
	threads = (pthread_t *)malloc(THREAD_POOL_SIZE_INITIAL*sizeof(pthread_t));
	
	for (int i = 0; i < THREAD_POOL_SIZE_INITIAL; ++i){
		pthread_cond_init(&client_added_cond[i],NULL);
		pthread_mutex_init(&client_added_lock[i],NULL);
	}
	
	for (int i = 0; i < THREAD_POOL_SIZE_INITIAL; ++i){
		tid[i] = i;
		pthread_create(&(threads[tid[i]]),NULL,&worker_thread,(void *)&tid[i]);
	}
	
	int socket_fd = init_socket();
	int client_fd;
	int curr_tid = 0;
	vector<int> thread_fd(THREAD_POOL_SIZE_INITIAL);
	while(1){
		client_fd = accept(socket_fd, NULL, NULL);
		int i = curr_tid%THREAD_POOL_SIZE_INITIAL;
		if(curr_tid<THREAD_POOL_SIZE_INITIAL){
			pthread_mutex_lock(&client_added_lock[i]);
			clients[i].push_back(client_fd);
			pthread_cond_signal(&client_added_cond[i]);
			pthread_mutex_unlock(&client_added_lock[i]);
		}
		else
			clients[i].push_back(client_fd);
		curr_tid++;
	}

	for (int i = 0; i < THREAD_POOL_SIZE_INITIAL; ++i)
		pthread_join(threads[i],NULL);

	close(socket_fd);

	return 0;
}

int processRequest(int client_fd){
	char request_buffer[1000];
	string response="";

	int len = read(client_fd, request_buffer,513);
	cout<<"recieved client request"<<endl;
	request_buffer[len] = '\0';
	
	vector<string> split_request = decode_request(request_buffer);

	char operation = split_request[0][0];
	string key = split_request[1];
	string val = split_request[2];

	if(operation=='2')
		response = encode_from_server(myCache.KVCache_put(key,val));
	if(operation=='1')
		response = encode_from_server(myCache.KVCache_get(key));
	if(operation=='3')
		response = encode_from_server(myCache.KVCache_del(key));

	// response.append(513 - response.length(), '-');

	write(client_fd,response.c_str(), 513);
	if(operation=='4'){
		close(client_fd);
		return 1;
	}
	cout<<"completed client request"<<endl;
	return 0;
}
