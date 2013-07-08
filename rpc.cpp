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

#define MAX_CLIENTS 50

using namespace std;

int rpcInit();
int rpcCall(char* name, int* argTypes, void** args);
int rpcCacheCall(char* name, int* argTypes, void** args);
int rpcRegister(char* name, int* argTypes, skeleton f);
int rpcExecute();
int rpcTerminate();


char server_id[128];
char server_port[2]; 
char rcv_name[100];
int* rcv_argTypes;
void** rcv_args; 
int reasonCode; 

//////////////////////////////////////////////////////////////////////////////////////////
// figure out the size (in bytes）of argTypes array, including the "0" at the end; 
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
	int total_len = 0; 	       // # of bytes
	while(*it != 0) {          // last element of argTypes is always ZERO
        // Type_mask = (255 << 16)
		unsigned int current_type = ((*it) & Type_mask) >> 16; 
		unsigned int len = ((*it) & Length_mask)  // # of current arg of current_type
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
// extract from received （argTypes + args) combination
void extract_args(char *buffer, int* argTypes, void** args) {
    int argTypesLen = getTypeLength(argTypes); 
    char *buf_ptr = buffer + argTypesLen;

    for (int i = 0; i < argTypesLen/4; i++) {
        args[i] = buf_ptr
    }
    
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
void receiveMsg(int MsgLen, int MsgType, int sockfd) {
    char rcv_buffer[8];
    
    int numbytes = recv(sockfd, rcv_buffer, 8, 0);
    if (numbytes <= 0) {
        if (numbytes == 0) {
            cout << "Target server is closed" << endl;
        } else {
            perror("ERROR in recving LOC reply from Binder");
        }
    } 
    char rcv_len[4];
    char rcv_type[4]; 
    memcpy(rcv_len, rcv_buffer, 4); 
    memcpy(rcv_type, rcv_buffer+4, 4); 

    if (atoi(rcv_type) == LOC_SUCCESS) {
        // Received msg type: LOC_SUCCESS from Binder
        char *rcv_buffer[130];
        if (recv(sockfd, rcv_buffer, 130, 0) < 0) {
            perror("ERROR in recving LOC reply from Binder");
        } else {
            memcpy(server_id, rcv_buffer, 128); 
            memcpy(server_port, rcv_buffer+128, 2); 

            close(sockfd);          // finish receiving data from Binder
        }
    } else if (atoi(rcv_type) == EXECUTE_SUCCESS) {
        // Received msg type: EXECUTE_SUCCESS from Server
        char *rcv_buffer[atoi(rcv_len)];
        if (recv(sockfd, rcv_buffer, , 0) < 0) {
            perror("ERROR in recving LOC reply from Binder");
        } else {

            memcpy(rcv_name, rcv_buffer, 100); 
            
            int temp_len = 0; 
            for (char* buf_ptr = rcv_buffer+100; atoi(*buf_ptr) != 0; buf_ptr++) {
                temp_len += 4; 
            }

            memcpy(rcv_argTypes, rcv_buffer+100, temp_len+4);
            memcpy(rcv_args, rcv_buffer+temp_len+4, temp_len); 

            close(sockfd);          // finish receiving data from Binder
        }
    } else if (atoi(rcv_type) == 0) {
        // Received msg type: FAILURE
    } 
}
//////////////////////////////////////////////////////////////////////////////////////////
// rpcCall 
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
    memcpy(buffer, &msgLen, 4);                 // first 4 bytes stores length of msg
    memcpy(buffer+4, &LOC_REQUEST, 4);          // next 4 bytes stores types info
    memcpy(buffer+8, name, 100);                // and then msg = name + argTypes
    memcpy(buffer+108, argTypes, getTypeLength(argTypes)); 

    // send LOC_REQUEST msg to Binder
    if (send(sockfd, buffer, msgLen+8, 0) == -1) {
        perror("ERROR in sending LOC_REQUEST to Binder");
    } 
    // wait for reply msg from Binder
    char rcv_buffer[8]; 
    int numbytes = recv(sockfd, rcv_buffer, 8, 0);
    if (numbytes < 0) {
    	perror("ERROR in recving LOC reply from Binder")
    	exit(1); 
    } else {
        // extract first 8 bytes
    	char rcv_len[4];
    	char rcv_type[4]; 
    	memcpy(rcv_len, rcv_buffer, 4); 
    	memcpy(rcv_type, rcv_buffer+4, 4); 

    	if (atoi(rcv_type) == 3) {                       // Type: LOC_SUCCESS
    		// now extract server name (128 bytes) and server port (2 bytes)
    		char *rcv_buffer[130];
    		recv(sockfd, rcv_buffer, 130, 0);
    		char server_id[128];
    		char server_port[2]; 
    		memcpy(server_id, rcv_buffer, 128); 
    		memcpy(server_port, rcv_buffer+128, 2); 

    		close(sockfd); 

            // Now connect to target server
    		connection(server_id, server_port, sockfd); 

    		int messageLen = msgLen + getArgsLength(argTypes, args);  // name, argTypes, args

    		char buffer[8 + messageLen];
    		memcpy(buffer, &messageLen, 4);
    		memcpy(buffer+4, &LOC_EXUCUTE, 4);
    		memcpy(buffer+8, name, 100); 
    		memcpy(buffer+108, argTypes, getTypeLength(argTypes)); 
            memcpy(buffer+108+getTypeLength(argTypes), args, getArgsLength(argTypes, args));

            // send EXECUTE request to server
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
                // extract first 8 bytes
                char rcv_len[4];
                char rcv_type[4]; 
                memcpy(rcv_len, rcv_buffer, 4); 
                memcpy(rcv_type, rcv_buffer+4, 4); 

                if (atoi(rcv_type) == 3) {                       // Type: LOC_SUCCESS
                    // now extract server name (128 bytes) and server port (2 bytes)
                    char *rcv_buffer[130];
                    if recv(sockfd, rcv_buffer, 130, 0);
                    char server_id[128];
                    char server_port[2]; 
                    memcpy(server_id, rcv_buffer, 128); 
                    memcpy(server_port, rcv_buffer+128, 2); 

                    close(sockfd);  
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

//////////////////////////////////////////////////////////////////////////////////////////
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//////////////////////////////////////////////////////////////////////////////////////////
// server calls rpcExecute: wait for and receive request 
// forward them to skeletons, and send back teh results
int rpcExecute(void) {
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                            "socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            newfd);
                    }
                } else {
                    // handle data from a client
                    char buf[8];    // buffer for client data
                    if ((nbytes = recv(i, buf, sizeof (buf), 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        // we got some data from a client
                        char rcv_len[4];
                        char rcv_type[4]; 
                        memcpy(rcv_len, buf, 4); 
                        memcpy(rcv_type, buf+4, 4); 

                        int rcvLen = atoi(*rcv_len);  
                        int rcvType = atoi(*rcv_type);
                        char *rcvMsg[rcvLen];
                            if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if (j != listener && j != i) {
                                    if (send(j, buf, nbytes, 0) == -1) {
                                        perror("send");
                                    }
                                }
                            }
                        
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    return 0;
}




