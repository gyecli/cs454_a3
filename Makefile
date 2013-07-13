CC := g++ -ggdb -Wall

requirements: librpc.a binder

test: test_server.run test_client.run binder

librpc.a: rpc.cpp server_loc.cpp prosig.cpp helper.cpp serverDB.cpp binderDB.cpp
	$(CC) -c -o rpc.o rpc.cpp
	$(CC) -c -o prosig.o prosig.cpp
	$(CC) -c -o helper.o helper.cpp
	$(CC) -c -o server_loc.o server_loc.cpp
	$(CC) -c -o serverDB.o serverDB.cpp
	$(CC) -c -o binderDB.o binderDB.cpp
	ar rvs librpc.a rpc.o prosig.o helper.o server_loc.o serverDB.o binderDB.o

binder: librpc.a binder.cpp
	$(CC) -L. binder.cpp -lrpc -o binder

test_server.run: librpc.a my_test/my_server.c my_test/my_server_functions.c my_test/my_server_function_skels.c
	$(CC) -c -o server_functions.o my_test/my_server_functions.c
	$(CC) -c -o server_function_skels.o my_test/my_server_function_skels.c
	$(CC) -c -o server.o my_test/my_server.c
	$(CC) -L. server_functions.o server_function_skels.o server.o -lrpc -o $@

test_client.run: librpc.a my_test/my_client.c
	$(CC) -c -o client.o my_test/my_client.c
	$(CC) -L. client.o -lrpc -o $@


.PHONY: clean
clean:
	rm -rf *.a *.o *.run binder server client
