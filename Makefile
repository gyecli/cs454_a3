requirements: librpc.a binder

test: test_server.run test_client.run binder

librpc.a: rpc.cpp server_loc.cpp prosig.cpp helper.cpp serverDB.cpp
	g++ -Wall -c -o rpc.o rpc.cpp
	g++ -Wall -c -o prosig.o prosig.cpp
	g++ -Wall -c -o helper.o helper.cpp
	g++ -Wall -c -o server_loc.o server_loc.cpp
	g++ -Wall -c -o serverDB.o serverDB.cpp
	g++ -Wall -c -o binderDB.o binderDB.cpp
	ar rvs librpc.a rpc.o prosig.o helper.o server_loc.o serverDB.o binderDB.o

binder: librpc.a binder.cpp
	g++ -Wall -L. binder.cpp -lrpc -o binder

test_server.run: librpc.a test/my_server.c test/my_server_functions.c test/my_server_function_skels.c
	g++ -Wall -c -o server_functions.o test/my_server_functions.c
	g++ -Wall -c -o server_function_skels.o test/my_server_function_skels.c
	g++ -Wall -c -o server.o test/my_server.c
	g++ -Wall -L. server_functions.o server_function_skels.o server.o -lrpc -o $@

test_client.run: librpc.a test/my_client.c
	g++ -Wall -c -o client.o test/my_client.c
	g++ -Wall -L. client.o -lrpc -o $@


.PHONY: clean
clean:
	rm -rf *.a *.o *.run binder server client
