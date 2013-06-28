// Please put the rcp functions here
#include <iostream>
#include <string>
#include <stdlib.h>
#include <cstdlib>
#include <cassert>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "rpc.h"
#include "my_rpc.h"

#define MAX_CLIENTS 20

using namespace std;

int rpcInit();
int rpcCall(char* name, int* argTypes, void** args);
int rpcCacheCall(char* name, int* argTypes, void** args);
int rpcRegister(char* name, int* argTypes, skeleton f);
int rpcExecute();
int rpcTerminate();


//////////////////////////////////////////////////////////////////////////////////////////
// figure out the size of argTypes, including the "0" at the end; 
int getTypeLength(int* argTypes) {
	int size = 0;
	int* it = argTypes;
	while (*it != 0) {
		size += 4;
		it = it+1;
	}
	return (size +4);
}

int getArgsLength(int* argTypes, void** args) {
	int* it = argTypes; 
	int total_len = 0; 	// in bytes
	while(*it != 0) {
		unsigned int current_type = ((*it) & Type_mask) >> 16; 
		unsigned int len = ((*it) & Length_mask)
		if (len == 0) {
			len = 1; 
		}
		switch(current_type) {
			case 1:
				// type: char
				total_len += len; 
				break; 
			case 2:
				// type: short
				total_len += 2 * len; 
				break;
			case 3:
				// type: int
				total_len +=  4 * len; 
				break;
			case 4:
				// type: long
				total_len += 4 * len; 
				break;
			case 5:
				// type: double
				total_len += 8 * len; 
				break; 
			case 6:
				// type: float
				total_len += 4 * len; 
				break;
			default:
				break;
		}
		it++;
	}
	return total_len; 
}
//////////////////////////////////////////////////////////////////////////////////////////
// Helper function to create a connection 
void connection(char* hostname, char* port, int sockfd) {
	struct addrinfo hints, *servinfo, *p;
    int rv;

	memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
        
    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("ERROR in client connecting");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "ERROR: client failed to connect\n");
        return -2;
    }

    freeaddrinfo(servinfo); // all done with this structure
}

//////////////////////////////////////////////////////////////////////////////////////////
int rpcCall(char* name, int* argTypes, void** args) { 
    //*************************************************
    // check whether arguments are valid
    // add code here

    //*************************************************
	int sockfd; 

    string Binder_id = getenv("BINDER_ADDRESS");
    string Binder_port = getenv("BINDER_PORT"); 
    connection(Binder_id.c_str(), BINDER_PORT.c_str(), sockfd); 

    //******************************************************
    // send LOC_REQUEST message to Binder
    int msgLen = 100 + getTypeLength(argTypes);  // name, argTypes
    char buffer[msgLen + 8];
    memcpy(buffer, &msgLen, 4);
    memcpy(buffer+4, &LOC_REQUEST, 4);
    memcpy(buffer+8, name, 100); 
    memcpy(buffer+108, argTypes, getTypeLength(argTypes)); 

    // send LOC_REQUEST msg to Binder
    if (send(sockfd, buffer, length+8, 0) == -1) {
        perror("ERROR in sending LOC_REQUEST to Binder");
    } 
    // wait for reply msg from Binder
    char rcv_buffer[8]; 
    int numbytes = recv(sockfd, rcv_buffer, 8, 0);
    if (numbytes < 0) {
    	perror("ERROR in recving LOC reply from Binder")
    	exit(1); 
    } else {
    	char rcv_len[4];
    	char rcv_type[4]; 
    	memcpy(rcv_len, rcv_buffer, 4); 
    	memcpy(rcv_type, rcv_buffer, 4); 

    	if (atoi(rcv_type) == 3) {
    		// now extract server name and server port
    		char *rcv_buffer[130];
    		if recv(sockfd, rcv_buffer, 130, 0);
    		char server_id[128];
    		char server_port[2]; 
    		memcpy(server_id, rcv_buffer, 128); 
    		memcpy(server_port, rcv_buffer+128, 2); 

    		close(sockfd); 

    		connection(server_id, server_port, sockfd); 

    		int msgLen = 100 + getTypeLength(argTypes);  // name, argTypes
    		char buffer[msgLen + 8];
    		memcpy(buffer, &msgLen, 4);
    		memcpy(buffer+4, &LOC_REQUEST, 4);
    		memcpy(buffer+8, name, 100); 
    		memcpy(buffer+108, argTypes, getTypeLength(argTypes)); 
    		if (send(sockfd, buffer, length+8, 0) == -1) {
        		perror("ERROR in sending LOC_REQUEST to Binder");
    		} 

    	} else if (atoi(rcv_buffer+4) == -1) {
    		// LOC_FAILURE
    		// add code here
    	}
    }

    // receive LOC_SUCCESS/LOC_FAILURE message from Binder
    // 
    // if succeed, close socket, connect to the server. 
    // send EXECUTE_REQUEST to server
    // receive result from server
    //******************************************************

    return 0; 
}