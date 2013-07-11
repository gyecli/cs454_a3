all: binder server

my_rpc: my_rpc.cpp
	g++ -Wall -c $^ -o my_rpc

rpcInit: my_rpc rpcInit.cpp
	g++ -Wall -c $^ 

rpc: rpc.cpp
	g++ -Wall -c -lpthread $^ -o rpc

#binder_lib: binder_lib.cpp
#	g++ -Wall -c $^ -o binder_lib

binder: binder.cpp
	g++ -Wall $^ 

server: server_db.cpp
	g++ -Wall -c $^ -o server


.PHONY: clean
clean:
	rm -rf binder server *.o
