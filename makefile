all: KVServer KVClient

KVServer:KVServer.h KVCache.h KVStore.h KVServer.cpp config.h
	g++ $^ -lpthread -o $@

KVClient:KVClient.cpp KVClient.h
	g++ $^ -lpthread -o $@

runServer:KVServer
	./$<

runClient:KVClient
	./$<

clean:
	rm -rf KVServer
	rm -rf KVClient