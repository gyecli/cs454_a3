all: binder server

binder: binder.cpp
	g++ -Wall -c $^ -o binder

server: server_db.cpp
	g++ -Wall -c $^ -o server


.PHONY: clean
clean:
	rm -rf binder server *.o
