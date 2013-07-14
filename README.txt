********************************************************************************************

CS 454 Distributed System

Assignment 3

By Guotian Ye, Yiyao Liu

********************************************************************************************
********************************************************************************************
How to compile:


	Type the following command to compile the librpc.a lib and binder

		make

	Then you can use

	    g++ -L. client.o -lrpc -o client -lpthread

	to link librpc.a with client.oto produce the binary file for client. The only difference of this line of command with the original one provoidede in the lab description is that, we use the -lpthread flag to explicitly link with pthread's lib to make sure the linking works on linux.student.cs machines.

	You can use
	
	    g++ -L. server_functions.o server_function_skels.o server.o -lrpc -o server -lpthread

	to produce the binary code for server, which also has the flag -lpthread.
	

********************************************************************************************
How to run:

Step 1:
	On binder machine, type ./binder to run, and the screen should shows the following
	SERVER_ADDRESS <hostname>
	SERVER_PORT <hostport>

Step 2:
	Manully set the BINDER_ADDRESS and BINDER_PORT environment variables on the client and server machines 
    This should be done by TA. 

Step 3:
	On server machine(s), type ./server to run server(s) 
	On client machine(s), type ./client to run client(s)

Step 4:
	Check out output message on clients' machine(s)

********************************************************************************************
NOTES:

1. remember to add "lrpc" when compiling the rpc lib, because threads are used. 
2. We always have to run Binder machine first.  It's not necessary to run servers ahead of clients


********************************************************************************************
********************************************************************************************
Group Infomation:
Name 			Userid			ID
Guotian Ye		gye				20381209
Yiyao Liu		y435liu			20420033
