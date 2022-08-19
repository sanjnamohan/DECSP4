APOORVA AGARWAL - 203050018
NUNNA APOORVA - 203050028
SANJNA MOHAN - 20305R006 

TO BUILD - 
make KVServer
make KVClient

TO RUN - 
make runServer
make runClient

==================
config.h
==================
A configuration file named config.h has been included to set different parameters required for the server
The following parameters have been included 

LISTENING_PORT 
CACHE_SIZE 
THREAD_POOL_SIZE_INITIAL 
MAX_EVENTS 




==========================
Client API (in KVClient.h)
==========================
1) get_from_server(key, value, error ,socket_id) : The method prints the value for a corresponding key (if exists in the store).

RETURN VALUE : int status code ( 200 for success, 240 for error) and populates error pointer with error message (if any), and value pointer with value (if exists)

2) put_to_server(key, value, error, socket_id) : The method inserts the given key, value pair into the store and cache.

RETURN VALUE : int status code ( 200 for success, 240 for error) and populates error pointer with error message (if any)

3) delete_from_server(key, value, error ,socket_id) : The method deletes the value for a corresponding key (if exists in the store).

RETURN VALUE : int status code ( 200 for success, 240 for error) and populates error pointer with error message (if any)


===================
KVClient.cpp
===================
A socket is created and user inputs are taken for different operations (Get/Put/Delete/Close connection)
and response is received


==================
KVStore
==================
Keys are stored in different files based on their leading 2 characters. All of these files are reset when a server restarts and are re-referenced.

Keys and Values are expected to be alphanumeric 

=====================
Error Analysis
=====================
I) Attached a graph showing response time Vs No. of requests from a client for 
1. Consecutive Puts
2. Alternate Puts and Gets
3. Alternate Puts and Deletes

II) Second graph attached shows throughput of server plotted against different no. of clients, where each client sends 1000 requests Alternate Puts and Deletes



==============
NOTE
==============
1) CLient can end connection through operation '4' in KVClient.cpp
2) Server runs continuously until explicitly signalled with SIGTERM or SIGINT
