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
//char* rcv_argTypes;
//char** rcv_args; 
char reasonCode; 

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

// get the total length of args in bytes
int getArgsLength(int* argTypes) {
	int* it = argTypes; 
	int total_len = 0; 	       // # of bytes
	while(*it != 0) {          // last element of argTypes is always ZERO
        // Type_mask = (255 << 16)
		unsigned int current_type = ((*it) & Type_mask) >> 16; 

        // TO_DO: Length_mask is not sure
		unsigned int num = ((*it) & Length_mask)  // # of current arg of current_type
		if (num == 0) {
			num = 1; 
		}

		switch(current_type) {
			case ARG_CHAR:
				// type: char
				total_len += 1 * num; 
				break; 
			case ARG_SHORT:
				// type: short
				total_len += 2 * num; 
				break;
			case ARG_INT:
				// type: int
				total_len +=  4 * num; 
				break;
			case ARG_LONG:
				// type: long
				total_len += 4 * num; 
				break;
			case ARG_DOUBLE:
				// type: double
				total_len += 8 * num; 
				break; 
			case ARG_FLOAT:
				// type: float
				total_len += 4 * num; 
				break;
			default:
				break;
		}
		it++;
	}
	return total_len; 
}

//////////////////////////////////////////////////////////////////////////////////////////
// Extract from received （argTypes + args) combination from server
// Assuming the argTypes & args are from rpcCall() 
/*
void extract_args(char *buffer, int *argTypes, void** args) {

    int length = getTypeLength(argTypes);   // in bytes

    char* it = buffer + length;  

    int j = 0; 
    while(argTypes[i] != 0) {          // last element of argTypes is always ZERO
        // Type_mask = (255 << 16)
        unsigned int current_type = ((argTypes[j]) & Type_mask) >> 16; 
        unsigned int num = ((argTypes[j]) & Length_mask)  // # of current arg of current_type
        int flag = 0; 
        if (num == 0) {
            num = 1; 
            flag = 1; 
        }

        switch(current_type) {
            case ARG_CHAR:
                // type: char
                char *temp2 = new char[num];
                memcpy(temp2, it, len);
                if (flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = (void *) temp2; 
                }
                it += num; 
                break; 

            case ARG_SHORT:
                // type: short
                short *temp2 = new short[num]
                memcpy (temp2, it, 2*num); 
                if(flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = (void *) temp2; 
                }
                it += 2*num; 
                break;
            case ARG_INT:
                // type: int
                short *temp2 = new short[num]
                memcpy (temp2, it, 4*num); 
                if(flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = (void *) temp2; 
                }
                it += 4*num; 
                break;
            case ARG_LONG:
                // type: long
                short *temp2 = new short[num]
                memcpy (temp2, it, 4*num); 
                if(flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = (void *) temp2; 
                }
                it += 4*num; 
                break;
            case ARG_DOUBLE:
                // type: double
                short *temp2 = new short[num]
                memcpy (temp2, it, 8*num); 
                if(flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = (void *) temp2; 
                }
                it += 8*num; 
                break; 
            case ARG_FLOAT:
                // type: float
                short *temp2 = new short[num]
                memcpy (temp2, it, 4*num); 
                if(flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = (void *) temp2; 
                }
                it += 4*num; 
                break;
            default:
                break;
        }
        j++; 
    }
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// 1.extract from buffer(message, and then put the data correspondly 
//   into argTypes & args
// 2.argTypes & args are NOT from rpcCall()
void pack(char* buffer, int* argTypes, void** args) {
    int num = 0;            // number of argTypes
    int argLen = 0; 
    char* it = buffer; 
    for (char* it = buffer; atoi(it) != 0; it = it+4) {
        num++; 
    }
    argTypes = new int[num+1];

    int i = 0; 
    char* it = buffer; 
    for ( ; atoi(it) != 0; it = it+4) {
        argTypes[i] = atoi(it); 
        i++; 
    }
    it += 4;                    // it now points to args in buffer
    argTypes[i] = 0; 
    argLen = getArgsLength(argTypes); 

    args = new (void*)[num * sizeof(void *)];           // TO_DO: not sure here

    int j = 0; 
    while(argTypes[j] != 0) {          // last element of argTypes is always ZERO
        // Type_mask:  (255 << 16)
        unsigned int current_type = (argTypes[j] & Type_mask) >> 16; 
        unsigned int num = ((*temp) & Length_mask)  // # of current arg of current_type
        
        int flag = 0; 

        if (num == 0) {
            num = 1; 
            flag = 1;       // 1 means the arg is a scalar, not an array
        }

        switch(current_type) {
            case ARG_CHAR:
                // type: char
                char *temp2 = new char[num];
                memcpy(temp2, it, len);
                if (flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    arg[j] = new char[num];
                    args[j] = (void *) temp2; 
                }
                it += num; 
                break; 
            case ARG_SHORT:
                // type: short
                short *temp2 = new short[num]
                memcpy (temp2, it, 2*num); 
                if(flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = new short[num];
                    args[j] = (void *) temp2; 
                }
                it += 2*num;
                break;
            case ARG_INT:
                // type: int
                int *temp2 = new int[num]
                if(flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = (void *) temp2; 
                }
                it += 4*num;
                break;
            case ARG_LONG:
                // type: long
                long *temp2 = new long[num]
                if(flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = (void *) temp2; 
                }
                it += 4*num;
                break;
            case ARG_DOUBLE:
                // type: double
                double *temp2 = new double[num]
                if(flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = (void *) temp2; 
                }
                it += 8*num;
                break; 
            case ARG_FLOAT:
                // type: float
                int *temp2 = new int[num]
                if(flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = (void *) temp2; 
                }
                it += 4*num;
                break;
            default:
                break;
        }
        j++; 
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
        cerr << "getaddrinfo: " << gai_strerror(rv) << endl;
        return 1;
    }
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            cerr << "client: socket" << endl;
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            cerr << "ERROR in client connecting" << endl;
            continue;
        }
        break;
    }

    if (p == NULL) {
        cerr <<  "ERROR: client failed to connect" << endl;
        return -2;
    }

    freeaddrinfo(servinfo); // all done with this structure
}
//////////////////////////////////////////////////////////////////////////////////////////
void receiveMsg(int msgLen, int msgType, int sockfd) {
    
    int numbytes = recv(sockfd, rcv_buffer, msgLen, 0);
    if (numbytes <= 0) {
        if (numbytes == 0) {
            cout << "Target server is closed" << endl;
        } else {
            cerr << "ERROR in recving LOC reply from Binder" << endl;
        }
    } 

    if (msgType == LOC_SUCCESS) {
        // Received msg type: LOC_SUCCESS from Binder
        char rcv_buffer[msgLen];
        if (recv(sockfd, rcv_buffer, msgLen, 0) < 0) {
            cerr << "ERROR in recving LOC reply from Binder" << endl;
        } else {
            memcpy(server_id, rcv_buffer, 128); 
            memcpy(server_port, rcv_buffer+128, 2); 

            close(sockfd);          // finish receiving data from Binder
        }
    } else if (msgType == EXECUTE_SUCCESS) {
        // Received msg type: EXECUTE_SUCCESS from Server
        char rcv_buffer[msgLen];
        if (recv(sockfd, rcv_buffer, , 0) < 0) {
            cerr << "ERROR in recving LOC reply from Binder" << endl;
        } else {
            memcpy(rcv_name, rcv_buffer, 100);
            memcpy(rcv_argTypes, rcv_buffer, )
            ///////////////////////
            // need implement here
            ///////////////////////

            close(sockfd);          // finish receiving data from Binder
        }
    } else if (msgType == 0) {
        // Received msg type: FAILURE
    } 
}
//////////////////////////////////////////////////////////////////////////////////////////
// rpcCall 
int rpcCall(char* name, int* argTypes, void** args) { 
    //*************************************************
    // check whether arguments are valid
    // Note: right now no need to check
    //*************************************************
	int sockfd; 

    string Binder_id = getenv("BINDER_ADDRESS");
    string Binder_port = getenv("BINDER_PORT"); 
    // connect to Binder
    connection(Binder_id.c_str(), BINDER_PORT.c_str(), sockfd); 

    // send LOC_REQUEST message to Binder
    int msgLen = 100 + getTypeLength(argTypes);  // name, argTypes
    char buffer[msgLen + 8];
    memcpy(buffer, &msgLen, 4);                 // first 4 bytes stores length of msg
    memcpy(buffer+4, &LOC_REQUEST, 4);          // next 4 bytes stores types info
    memcpy(buffer+8, name, 100);                // and then msg = name + argTypes
    memcpy(buffer+108, argTypes, getTypeLength(argTypes)); 

    // send LOC_REQUEST msg to Binder
    if (send(sockfd, buffer, msgLen+8, 0) == -1) {
        cerr << "ERROR in sending LOC_REQUEST to Binder" << endl;
    } 
    // wait for reply msg from Binder
    char rcv_buffer[8]; 
    int numbytes = recv(sockfd, rcv_buffer, 8, 0);
    if (numbytes < 0) {
    	cerr << "ERROR in recving LOC reply from Binder" << endl;
    	exit(1); 
    } else {
        // extract first 8 bytes from Binder
    	char rcv_len[4];
    	char rcv_type[4]; 
    	memcpy(rcv_len, rcv_buffer, 4); 
    	memcpy(rcv_type, rcv_buffer+4, 4); 

        int len = atoi(rcv_len);
        int type = atoi(rcv_type);

    	if (type == LOC_SUCCESS) {                 
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

    		int messageLen = msgLen + getArgsLength(argTypes);  // name, argTypes, args
            int requestType = EXECUTE;

    		char buffer[8 + messageLen];
    		memcpy(buffer, &messageLen, 4);
    		memcpy(buffer+4, &reqeustType, 4);
    		memcpy(buffer+8, name, 100); 
    		memcpy(buffer+108, argTypes, getTypeLength(argTypes)); 
            memcpy(buffer+108+getTypeLength(argTypes), args, getArgsLength(argTypes));

            // send EXECUTE request to server
    		if (send(sockfd, buffer, length+8, 0) == -1) {
        		cout << "ERROR in sending LOC_REQUEST to Binder" << endl;
                return -1; 
    		} 
            // wait for reply msg from server
            char rcv_buffer[8]; 
            int numbytes = recv(sockfd, rcv_buffer, 8, 0);
            if (numbytes < 0) {
                cerr << "ERROR in recving LOC reply from Binder" << endl;
                exit(1); 
            } else {
                // extract first 8 bytes from Server
                char rcv_len[4];
                char rcv_type[4]; 
                memcpy(rcv_len, rcv_buffer, 4); 
                memcpy(rcv_type, rcv_buffer+4, 4); 

                len = atoi(rcv_len);
                type = atoi(rcv_type);
                
                if (type == EXECUTE_SUCCESS) {         
                    
                    char *rcv_buffer[len];
                    if (recv(sockfd, rcv_buffer, len, 0) < 0) {

                    }
                    //
                    int *new_argTypes;
                    void** new_args; 
                    pack(rcvMsg, argTypes, args); 

                    memcpy(args, new_args, getArgsLength(argTypes));
                    //extract_args(rcv_buffer, argTypes, args);  //argTypes & args are from rpcCall 

                    close(sockfd);  

                    return RPCCALL_SUCCESS;
                } else if (type == FAILURE) {
                    cout << "EXECUTE FAILURE" << endl;
                    return RPCCALL_FAILURE;
                } else {
                    cout << "Should not come here 1" << endl;
                }
            }
    	} else if (type == FAILURE) {
            cout << "LOC FAILURE" << endl;
            return RPCCALL_FAILURE;
    	} else {
            cout << "Shoudl not come here 2" << endl; 
        }
    }
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
        cerr << "selectserver: " << gai_strerror(rv) << endl;
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
        cerr << "listen" << endl;
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    int len; 
    int type; 

    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            cerr << "select" << endl;
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
                        cerr << "accept" << endl;
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        //printf("selectserver: new connection from %s on "
                        //    "socket %d\n",
                        //    inet_ntop(remoteaddr.ss_family,
                        //        get_in_addr((struct sockaddr*)&remoteaddr),
                        //        remoteIP, INET6_ADDRSTRLEN),
                        //    newfd);
                    }
                } else {
                    // handle data from a client
                    char buf[8];    // buffer for client data
                    if ((nbytes = recv(i, buf, sizeof (buf), 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            cout << "selectserver: socket " << i << "hung up" << endl;
                        } else {
                            cerr << "recv" << endl;
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        // we got some data from a client
                        char rcv_len[4];
                        char rcv_type[4]; 
                        memcpy(rcv_len, buf, 4); 
                        memcpy(rcv_type, buf+4, 4); 

                        int len = atoi(*rcv_len);  
                        int type = atoi(*rcv_type);
                        char *rcvMsg[len];

                        if (recv(i, rcvMsg, len, 0) < 0) {
                            cerr << "ERROR in receiving msg from client" << endl;
                        }


                        // search for skeleton here
                        // To-do:  searching for skeleton in local DB
                        // skeleton search_skel(char* name, int* argTypes)
                        char * name = new char[100]; 
                        memcpy(name, rcvMsg, 100); 
                        rcvMsg += 100; 

                        int *argTypes;
                        void** args; 

                        pack(rcvMsg, argTypes, args); 
                        
                        skeleton skel_func = search_skel(name, argTypes);
                        skel_func(argTypes, args);


                        int messageLen = 100 + getTypeLength(argTypes) + getArgsLength(argTypes);  // name, argTypes, args

                        int exeResult = EXECUTE_SUCCESS; // TO_DO: miss EXECUTE_FAILURE

                        char buffer[8 + messageLen];
                        memcpy(buffer, &messageLen, 4);
                        memcpy(buffer+4, &exeResult, 4);
                        memcpy(buffer+8, name, 100); 
                        memcpy(buffer+108, argTypes, getTypeLength(argTypes)); 
                        memcpy(buffer+108+getTypeLength(argTypes), args, getArgsLength(argTypes));
                        
                        if (FD_ISSET(i, &master)) {
                            if (send(i, buffer, nbytes, 0) == -1) {
                                cerr << "send" << endl;
                            }
                            
                        }
                        
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    return 0;
}




