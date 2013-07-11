all: librpc.a

librpc.a: my_rpc.o rpcInit.o prosig.o server_loc.o binderDB.o
	ar rvs $@ $^

test: librpc.a server.c server_functions.c server_function_skels.c
	g++ -Wall -o test.run $^

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
	g++ -Wall $^



rpcInit: rpcInit.cpp
	g++ -Wall -c $^ -o rpcInit.o


my_rpc.o: my_rpc.cpp
	g++ -Wall -c $^ -o $@

rpc: rpc.o rpc.cpp
	g++ -Wall $^

 

server: server_db.cpp
	g++ -Wall -c $^ -o server


.PHONY: clean
clean:
	rm -rf binder server *.o *.a
