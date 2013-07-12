// This file has all the rpc funcitons

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <string>
#include <algorithm>    // std::max
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>  // ip
#include <unistd.h>  // write
#include <arpa/inet.h>  // inet_addr
#include <pthread.h>
#include <netdb.h>
#include <list>
#include <utility>

#include "rpc.h"
#include "prosig.h"
#include "helper.h"
#include "serverDB.h"
#include "const.h"

#define MAX_CLIENTS 20
using namespace std;

//yiyao
int binderSocket; 
int clientSocket; 
int sockfd;
char serverID[SIZE_IDENTIFIER];
char serverPort[SIZE_PORTNO];
ServerDB serverDatabase; 

//tim
char server_id[SIZE_IDENTIFIER];
char server_port[SIZE_PORTNO]; 
char rcv_name[SIZE_NAME];
//char* rcv_argTypes;
//char** rcv_args; 
int reasonCode; 
//ServerDB serverDatabase;    // TODO: already in rpcInit.cpp 
//int binderSocket;           // TODO: This is defined in rpcInit.cpp
fd_set master;    // master file descriptor list (used in rpcExecute())
int terminate_flag = 0;     // 1 means receive terminate request

//



// This is for pthread arguments passing (see rpcExecute() ) 
struct arg_struct {
    int sockfd;         // This is for send()

    char* name; 
    int* argTypes;
    void** args; 
};


void* execute(void* arguments);  // Prototype
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// 1.extract from buffer(message), and then put the data correspondly 
//   into argTypes & args
// 2.argTypes & args are NOT from rpcCall()
void pack(char* buffer, int** argTypes, void*** args) {
    int num = 0;            // number of argTypes
    int argLen = 0; 
    char* it = buffer; 

    // This loop is to get the size for argTypes array
    for (char* it = buffer; atoi(it) != 0; it = it+4) {
        num++; 
    }
    *argTypes = new int[num+1];

    int i = 0; 
    it = buffer;        // it re-points to the start of buffer
    // Assign corresponding value into argTypes array
    for ( ; atoi(it) != 0; it = it+4) {
        (*argTypes)[i] = atoi(it); 
        i++; 
    }
    (*argTypes)[i] = 0; 
    it += 4;                    // it now points to args in buffer

    argLen = getArgsLength(*argTypes); 

    (*args) = new void* [num * sizeof(void *)];           // TODO: not sure here

    int j = 0; 

    char *temp1;
    short *temp2; 
    int *temp3; 
    long *temp4;
    double *temp5;
    float *temp6;

    while((*argTypes)[j] != 0) {          // last element of argTypes is always ZERO
        // Type_mask:  (255 << 16)
        unsigned int current_type = ((*argTypes)[j] & Type_mask) >> 16; 
        unsigned int num = (((*argTypes)[j]) & array_size_mask);  // # of current arg of current_type
        
        int flag = 0; 

        if (num == 0) {
            num = 1; 
            flag = 1;       // 1 means the arg is a scalar, not an array
        }

        switch(current_type) {
            case ARG_CHAR:
                // type: char
                temp1 = new char[num];
                memcpy(temp1, it, 1*num);
                if (flag == 1) {
                    (*args)[j] = (void *) &(*temp1); 
                } else {
                    (*args)[j] = new char[num];
                    (*args)[j] = (void *) temp1; 
                }
                it += num; 
                break; 
            case ARG_SHORT:
                // type: short
                temp2 = new short[num];
                memcpy (temp2, it, 2*num); 
                if(flag == 1) {
                    (*args)[j] = (void *) &(*temp2); 
                } else {
                    (*args)[j] = new short[num];
                    (*args)[j] = (void *) temp2; 
                }
                it += 2*num;
                break;
            case ARG_INT:
                // type: int
                temp3 = new int[num];
                if(flag == 1) {
                    (*args)[j] = (void *) &(*temp3); 
                } else {
                    (*args)[j] = new int[num];
                    (*args)[j] = (void *) temp3; 
                }
                it += 4*num;
                break;
            case ARG_LONG:
                // type: long
                temp4 = new long[num];
                if(flag == 1) {
                    (*args)[j] = (void *) &(*temp4); 
                } else {
                    (*args)[j] = new long[num];
                    (*args)[j] = (void *) temp4; 
                }
                it += 4*num;
                break;
            case ARG_DOUBLE:
                // type: double
                temp5 = new double[num];
                if(flag == 1) {
                    (*args)[j] = (void *) &(*temp5); 
                } else {
                    (*args)[j] = new double[num];
                    (*args)[j] = (void *) temp5; 
                }
                it += 8*num;
                break; 
            case ARG_FLOAT:
                // type: float
                temp6 = new float[num];
                if(flag == 1) {
                    (*args)[j] = (void *) &(*temp6); 
                } else {
                    (*args)[j] = new float[num];
                    (*args)[j] = (void *) temp6; 
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
// return 0 on success, negative number on failure
// Assuming "sockfd" is already assigned
int connectServer(const char* hostAddr, const char* portno, int *socketnum) 
{

    struct sockaddr_in addr;
    struct hostent *he;

    if ( (he = gethostbyname(hostAddr) ) == NULL ) 
    {
        perror("ERROR gethostbyname");
        exit(-1);
    }

    /* copy the network address to sockaddr_in structure */
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(portno));
    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    *socketnum = socket(PF_INET, SOCK_STREAM, 0);

    if(*socketnum == 0)
    {
        perror("ERROR socket");
        exit(-1);
    }

    if(connect(*socketnum, (struct sockaddr *)&addr, sizeof(addr))<0)
    {
        perror("ERROR connect");
        exit(-1);
    }

    return 0; //TO-DO change return value, to indicate error
}


void ConnectBinder(int* socketnum)
{
    struct sockaddr_in addr;
    char* hostAddr, *portno; 

    if((hostAddr = getenv("BINDER_ADDRESS")) == 0)
    {
        perror("can't get env variable SERVER_ADDRESS"); 
        exit(-1);
    }
    if((portno = getenv("BINDER_PORT"))==0)
    {
        perror("can't get env variable SERVER_PORT");
        exit(-1);
    }
    struct hostent *he;
    if ( (he = gethostbyname(hostAddr) ) == NULL ) 
    {
        perror("ERROR gethostbyname");
        exit(-1);
    }

    /* copy the network address to sockaddr_in structure */
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(portno));
    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    *socketnum = socket(PF_INET, SOCK_STREAM, 0);

    if(*socketnum == 0)
    {
        perror("ERROR socket");
        exit(-1);
    }

    if(connect(*socketnum, (struct sockaddr *)&addr, sizeof(addr))<0)
    {
        perror("ERROR connect");
        exit(-1);
    }
}

//Determine the identifier & portno 
//that will be used for clients 
void GetSelfID()
{
    struct sockaddr_in addr;
    int addrlen; 
    char hostname[SIZE_IDENTIFIER];

    //server socket, which will be connected by clients later
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    if(clientSocket == 0)
    {
        perror("ERROR socket");
        exit(-1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = INADDR_ANY;
    addrlen = sizeof(addr); 

    if(bind(clientSocket, (struct sockaddr *)&addr, sizeof(addr))<0)
    {
        perror("ERROR bind"); 
        exit(-1);
    }

    //make this socket a passive one
    if(listen(clientSocket, MAX_CLIENTS))  // max 5 conns
    {
        perror("ERROR listen");
        exit(-1);
    }

    if( getsockname(
        clientSocket, 
        (struct sockaddr*)&addr, 
        (socklen_t*)&addrlen )
         == -1)
    {
        perror("ERROR socketname");
        exit(-1);
    }
    gethostname(hostname, sizeof(hostname));

    memcpy(serverID, hostname, SIZE_IDENTIFIER); 
    uint16_t pno = ntohs(addr.sin_port); 
    memcpy(serverPort, (char*)(&pno), SIZE_PORTNO); 
}

int rpcInit()
{
    GetSelfID();    
    ConnectBinder(&binderSocket); 
    //TODO: handle error cases
    return 0; 
}

int rpcRegister(char* name, int *argTypes, skeleton f)
{    
    //firstly send to binder 
    char* send_buff; 
    int valread;
    int argSize = getTypeLength(argTypes);
    int totalSize = SIZE_IDENTIFIER + SIZE_PORTNO + SIZE_NAME + argSize; 
    send_buff = new char[totalSize + 8];

    //marshall everything into the stream to binder 
    char sizeChar[4]; 
    //int2char4(totalSize, sizeChar);
    memcpy(send_buff, (char*)&totalSize, 4); 

    char typeChar[4];
    //int2char4(REGISTER, typeChar);
    int t = REGISTER;
    memcpy(send_buff+4, (char*)&t , 4);

    memcpy(send_buff+8, serverID, SIZE_IDENTIFIER);
    memcpy(send_buff+8+SIZE_IDENTIFIER, serverPort, SIZE_PORTNO); 
    memcpy(send_buff+8+SIZE_IDENTIFIER+SIZE_PORTNO, name, SIZE_NAME); 
    memcpy(send_buff+8+SIZE_IDENTIFIER+SIZE_PORTNO+SIZE_NAME, argTypes, argSize);
    write(binderSocket, (void*)send_buff, totalSize+8);
    cout<<"sent"<<endl;

    //TODO: error handling, eg: can't connect to binder
    //TODO: not sure if 'read' immediately after 'write' works
    char size_buff[4];
    valread = read(binderSocket, size_buff, 4);

    if(valread < 0)
    {
        error("ERROR read from socket, probably due to connection failure");
        return REGISTER_FAILURE; 
    }
    else if(valread == 0)
    {
        return REGISTER_FAILURE; 
    }
    else
    {
        uint32_t *size = (uint32_t*)size_buff; 
        char type_buff[4];
        valread = read(binderSocket, type_buff, 4);

        uint32_t *type = (uint32_t*)type_buff;
        if(*type == REGISTER_SUCCESS)
        {
            //cout << "Testing: REGISTER_SUCCESS in rpcInit.cpp" << endl;      // TO_DO: for testing, delete later
        }  
        else
        {
            //read error here
            //cout << "Testing: REGISTER_FAILURE in rpcInit.cpp" << endl;      // TO_DO: for testing, delete later
            return REGISTER_FAILURE; 
        }
    }
    //cout << "Testing: REGISTER_SUCCESS in rpcInit.cpp end" << endl;      // TO_DO: for testing, delete later
    //store to local DB
    serverDatabase.Add(name, argTypes, f);

    // TODO:  sorry, where is the registeration to Binder? (may be i missed it)
    return REGISTER_SUCCESS; 

}


//////////////////////////////////////////////////////////////////////////////////////////
// rpcCall 
int rpcCall(char* name, int* argTypes, void** args) { 

    //*************************************************
    // TO-DO check whether arguments are valid
    // Note: right now no need to check
    //*************************************************
    char* buff; 
    int sockfd; 
    int valread;
    char size_buff[4];
    char type_buff[4];
    ConnectBinder(&sockfd);

    // send LOC_REQUEST message to Binder
    int msgLen = (SIZE_NAME + getTypeLength(argTypes));  // name, argTypes
    cout<<"length:"<<msgLen;

    char send_buff[msgLen + 8];
    unsigned int requestType = LOC_REQUEST;

    memcpy(send_buff, (char *) &msgLen, 4);                 // first 4 bytes stores length of msg
    memcpy(send_buff + 4, (char *) &requestType, 4);          // next 4 bytes stores types info
    memcpy(send_buff + 8, name, SIZE_NAME);                // and then msg = name + argTypes
    memcpy(send_buff + 8 + SIZE_NAME, argTypes, getTypeLength(argTypes)); 

    // send LOC_REQUEST msg to Binder
    if (send(sockfd, send_buff, msgLen + 8, 0) == -1) {
        cerr << "ERROR in sending LOC_REQUEST to Binder" << endl;
    } 

    // wait for reply msg from Binder
    valread = read(binderSocket, size_buff, 4);
    if(valread < 0)
    {
        error("ERROR read from socket, probably due to connection failure");
        return LOC_FAILURE; 
    }
    else if(valread == 0)
    {
        //TODO figure out the error case
        return LOC_FAILURE; 
    }
    else
    {
        uint32_t *size = (uint32_t*)size_buff; 
        valread = read(binderSocket, type_buff, 4);
        uint32_t *type = (uint32_t*)type_buff;

    	if (*type == LOC_SUCCESS) 
        {                 
    		// now extract server name (128 bytes) and server port (2 bytes)
            cout << "LOC_SUCCESS in rpcCall()" << endl;
            buff = new char[*size+10]; 
            valread = read(sockfd, buff, *size+10);
            if(valread < 0)
            {
                error("ERROR read from socket, probably due to connection failure");
                return LOC_FAILURE; 
            }

    		char server_id[SIZE_IDENTIFIER];
    		char server_port[SIZE_PORTNO]; 
    		memcpy(server_id, buff, SIZE_IDENTIFIER); 
    		memcpy(server_port, buff+SIZE_IDENTIFIER, SIZE_PORTNO); 

            //cout<<"id:"<<string(server_id)<<endl;
            //cout<<"port:" << string(server_port) <<endl;

    		close(sockfd);    // close socket between client and binder

            // Now connect to target server
    		if (connectServer(server_id, server_port, &sockfd) < 0) {
                cout << "ERROR in connecting to server" << endl;
                return -1; 
            }

    		int messageLen = msgLen + getArgsLength(argTypes);  // name, argTypes, args
            requestType = EXECUTE;

    		char buffer[8 + messageLen];
    		memcpy(buffer, (char *) &messageLen, 4);
    		memcpy(buffer+4, (char *) &requestType, 4);
    		memcpy(buffer+8, name, SIZE_NAME); 
    		memcpy(buffer+8+SIZE_NAME, argTypes, getTypeLength(argTypes)); 
            memcpy(buffer+8+SIZE_NAME+getTypeLength(argTypes), args, getArgsLength(argTypes));

/*
            // send EXECUTE request to server
    		if (send(sockfd, buffer, messageLen+8, 0) == -1) {
        		cout << "ERROR in sending LOC_REQUEST to Binder" << endl;
                return -1; 
    		} 
            cout << "Sent EXECUTE request to server" << endl;

            // wait for reply msg from server
            char rcv_buffer1[8]; 
            int numbytes = recv(sockfd, rcv_buffer1, 8, 0);
            if (numbytes < 0) {
                cerr << "ERROR in recving LOC reply from Binder" << endl;
                exit(1); 
            } else {
                // extract first 8 bytes from Server
                char rcv_len[4];
                char rcv_type[4]; 
                memcpy(rcv_len, rcv_buffer1, 4); 
                memcpy(rcv_type, rcv_buffer1+4, 4); 

                len = atoi(rcv_len);
                type = atoi(rcv_type);
                
                if (type == EXECUTE_SUCCESS) {         
                    
                    char rcv_buffer2[len];
                    if (recv(sockfd, rcv_buffer2, len, 0) < 0) {

                    }
                    //
                    int *new_argTypes;
                    void** new_args; 
                    pack(rcv_buffer2, &new_argTypes, &new_args); 

                    memcpy(args, new_args, getArgsLength(argTypes));

                    close(sockfd);  

                    return RPCCALL_SUCCESS;
                } else if (type == EXECUTE_FAILURE) {
                    cout << "EXECUTE FAILURE" << endl;
                    return RPCCALL_FAILURE;           // TO_DO: should return EXECUTE_FAILURE??
                } else {
                    cout << "Should not come here 1" << endl;
                    return -10;             // TO_DO: haven't been determined
                }
            }
*/
    	} 
        else if (*type == LOC_FAILURE) 
        {
            cout << "LOC FAILURE" << endl;
            return RPCCALL_FAILURE;         // TO_DO: should return EXECUTE_FAILURE??
    	} 
        else 
        {
            cout << "Shoudl not come here 2" << endl; 
            return -10;             // TO_DO: haven't been determined
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
    //fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    //char hostName[SIZE_IDENTIFIER];   // host name of local machine

    int nbytes;

    struct sockaddr_in addr;
    int s_len;

    int i;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    //******************************************************************
    // get a free port
    //listener = socket(PF_INET, SOCK_STREAM, 0);

    //if(listener == 0)
    //{
    //    cerr << "ERROR listen" << endl;
    //    exit(-1);
    //}

    //addr.sin_family = AF_INET;
    //addr.sin_port = 0;
    //addr.sin_addr.s_addr = INADDR_ANY;
    //s_len = sizeof(addr);

    //if(bind(listener, (struct sockaddr *)&addr, sizeof(addr))<0)
    //{
    //    cerr << "ERROR bind" << endl;
    //    exit(-1);
    //}

    //if(listen(listener, MAX_CLIENTS))  
    //{
    //    cerr << "ERROR listen: too many client coneection requests" << endl;
    //    exit(-1);
    //}

    //if(getsockname(listener, (struct sockaddr*)&addr, (socklen_t*)&s_len ) == -1)
    //{
    //    cerr << "ERROR getsockname" << endl;
    //    exit(-1);
    //}

    // add the clientSocket to the master set
    FD_SET(clientSocket, &master);

    FD_SET(binderSocket, &master);    // TO_DO: can i just add the binderSocket to the set and then listen on it?

    // keep track of the biggest file descriptor
    fdmax = max(clientSocket, binderSocket); // so far, it's this one

    // main loop
    std::list<pthread_t> thread_list; 

    cout << "In rpcExecute(), clientSocket = " << clientSocket << endl;
    cout << "In rpcExecute(), binderSocket = " << binderSocket << endl;

    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            cerr << "ERROR in select in rpcExecute()" << endl;
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == clientSocket && terminate_flag == 0) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(clientSocket, (struct sockaddr *)&remoteaddr, &addrlen);

                    if (newfd == -1) {
                        cerr << "ERROR in accept in rpcExecute() of rpc.cpp" << endl;
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        cout << "server: new connection on socket " << newfd << endl;
                    }
                } else if (terminate_flag == 0) {           
                    // handle data from a client
                    char buf[8];    // buffer for client data
                    cout << "In rpcExecute(), waiting to receive data from clients" << endl;
                    if ((nbytes = recv(i, buf, sizeof (buf), 0)) <= 0) {        // TO_DO:  should I put "=" here?
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
                        cout << "TESTING: in rpcExecute() of rpc.cpp" << endl;
                        char rcv_len[4];
                        char rcv_type[4]; 
                        memcpy(rcv_len, buf, 4); 
                        memcpy(rcv_type, buf+4, 4); 

                        int len = atoi(rcv_len);  
                        int type = atoi(rcv_type);
                        char rcvMsg[len];

                        if (type == TERMINATE) {
                            // Need to verify Sender's ID (Binder)
                            // TO_DO: add verification code here

                            // Go to shut-down routine 
                            terminate_flag = 1; 

                            // break the loop of running through the existing connections looking for data to read
                            // i.e. stop reading new data
                            break;  
                        }


                        if (recv(i, rcvMsg, len, 0) < 0) {
                            cerr << "ERROR in receiving msg from client" << endl;
                        }

                        char * name = new char[100]; 
                        memcpy(name, rcvMsg, 100);    // TO-DO: sth wrong here  
                        char* newRcvMsg = rcvMsg + 100; 

                        int* new_argTypes;
                        void** new_args; 

                        // pack() will put info of newRcvMsg into argTypes & args respectively
                        pack(newRcvMsg, &new_argTypes, &new_args);  // TO_DO: & OR *
                        
                        struct arg_struct args;
                        args.sockfd = i;
                        args.name = name;
                        args.argTypes = new_argTypes; 
                        args.args = new_args; 

                        // TODO: still need the definition of search_skel()
                        pthread_t newThread; 
                        thread_list.push_back(newThread); 
                        if (pthread_create(&newThread, NULL, execute, (void*) &args)) {
                            cerr << "ERROR in creating new thread" << endl;
                        }

                        
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors

        if (terminate_flag == 1) {
            break; // break for(;;)
        }

    } // END for(;;)--and you thought it would never end!
    
    for (std::list<pthread_t>::iterator it = thread_list.begin(); it != thread_list.end(); it++) {
        pthread_join(*it, NULL);    // TO_DO: use NULL or some other status?
    }

    return 0;
}

// when received request from clients, do the execution here
void* execute(void* arguments) {
    struct arg_struct *args = (struct arg_struct *)arguments;

    skeleton skel_func = serverDatabase.SearchSkeleton(args->name, args->argTypes);    // search in server local DB
    int exeResult = skel_func(args->argTypes, args->args);

    int messageLen; 
    char * ready_buffer;

    if (exeResult == EXECUTE_SUCCESS) {
        messageLen = 100 + getTypeLength(args->argTypes) + getArgsLength(args->argTypes);  // name, argTypes, args

        char buffer[8 + messageLen];
        memcpy(buffer, (char *) &messageLen, 4);
        memcpy(buffer+4, (char *) &exeResult, 4);
        memcpy(buffer+8, args->name, 100); 
        memcpy(buffer+108, args->argTypes, getTypeLength(args->argTypes)); 
        memcpy(buffer+108+getTypeLength(args->argTypes), args->args, getArgsLength(args->argTypes));
    
        ready_buffer = buffer;
    } else {
        // EXECUTE_FAILURE
        reasonCode = -2;    // TODO: is this a good reason code?
        messageLen = 4; 

        char buffer [12];
        memcpy(buffer, (char *) &messageLen, 4);
        memcpy(buffer+4, (char *) &exeResult, 4);
        memcpy(buffer+8, (char *) &reasonCode, 4); 

        ready_buffer = buffer;
    }

    if (FD_ISSET(args->sockfd, &master)) {
        if (send(args->sockfd, ready_buffer, 8+messageLen, 0) == -1) {
            cerr << "send" << endl;
        }                      
   }
   pthread_exit(NULL);
}



// Clients call rpcTerminate()
int rpcTerminate(void) {
    int sockfd; 

    string Binder_id = getenv("BINDER_ADDRESS");
    string Binder_port = getenv("BINDER_PORT"); 
    // re-connect to Binder
    if (connectServer(Binder_id.c_str(), Binder_port.c_str(), &sockfd) < 0) {
        cout << "ERROR in connecting to Binder" << endl;
        return -1;      // TO_DO:  need a better meaningful negative number
    }

    // send LOC_REQUEST message to Binder
    int msgLen = 0;     // No following message after type
    char buffer[8];
    unsigned int requestType = TERMINATE;

    memcpy(buffer, (char *) &msgLen, 4);                 // first 4 bytes stores length of msg
    memcpy(buffer+4, (char *) &requestType, 4);          // next 4 bytes stores types info

    // send LOC_REQUEST msg to Binder
    if (send(sockfd, buffer, msgLen+8, 0) == -1) {
        cerr << "ERROR in sending TERMINATE to Binder" << endl;
        return TERMINATE_FAILURE;
    } 
    close(sockfd); 
    return TERMINATE_SUCCESS;
}







