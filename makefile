all: binder server

binder_lib: binder_lib.cpp
	g++ -Wall -c $^ -o binder_lib

binder: binder.cpp binder_lib
	g++ -Wall $^ 

server: server_db.cpp
	g++ -Wall -c $^ -o server


.PHONY: clean
clean:
	rm -rf binder server *.o
