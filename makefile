all: librpc.a

librpc.a: my_rpc.o rpcInit.o
	ar rvs $@ $^

test: librpc server.c server_functions.c server_function_skels.c
	g++ -Wall -o test.run $^

my_rpc.o: my_rpc.cpp
	g++ -Wall -c $^ -o $@

rpcInit.o: rpc.o rpcInit.cpp
	g++ -Wall -c $^ -o $@

rpc: rpc.cpp
	g++ -Wall -c -lpthread $^ -o rpc

binder: binder.cpp
	g++ -Wall $^ 

server: server_db.cpp
	g++ -Wall -c $^ -o server


.PHONY: clean
clean:
	rm -rf binder server *.o *.a
