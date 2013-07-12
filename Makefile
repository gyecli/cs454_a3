target: librpc.a

all: target test_server.run test_client.run test_binder.run

librpc.a: rpc.o server_loc.o prosig.o helper.o serverDB.o
	ar rvs $@ $^

testserver: librpc.a my_server.c server_functions.c server_function_skels.c
	g++ -Wall -o testserver.run $^

testclient: librpc.a client1.c
	g++ -Wall -o testclient.run $^

prosig: prosig.cpp
	g++ -Wall -c -o prosig.o $^

server_loc: server_loc.cpp
	g++ -Wall -c -o server_loc.o $^

binderDB: binderDB.cpp
	g++ -Wall -c -o binderDB.o $^

helper: helper.cpp
	g++ -Wall -c -o helper.o $^

serverDB: serverDB.cpp
	g++ -Wall -c -o binderDB.o $^

binder: binder.cpp prosig.o server_loc.o binderDB.o helper.o
	g++ -Wall $^ -o binder

rpcInit: rpcInit.cpp
	g++ -Wall -c $^ -o rpcInit.o

rpc: rpc.cpp
	g++ -Wall -lpthread -c $^ -o rpc.o
 

server: server_db.cpp
	g++ -Wall -c $^ -o server


.PHONY: clean
clean:
	rm -rf binder server *.o *.a
