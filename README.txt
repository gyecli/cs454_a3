********************************************************************************************

CS 454 Distributed System

Assignment 3

By Guotian Ye, Yiyao Liu

********************************************************************************************
********************************************************************************************
How to compile:  (TODO)


	Type the following command to compile
		"make all"
	and then the screen should show the following:
		"gcc -o stringServer stringServer.c
		 g++ stringClient.cpp -lpthread -o stringClient"
	

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
	On client machine(s), type .client to run client(s)

Step 4:
	Checkout output message on clients' machine(s)

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
