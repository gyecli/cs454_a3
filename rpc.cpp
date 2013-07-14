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
char serverID[SIZE_IDENTIFIER];
char serverPort[SIZE_PORTNO];
ServerDB serverDatabase; 

//tim 
fd_set master; // master file descriptor list (used in rpcExecute())
int terminate_flag = 0; // 1 means receive terminate request

// Calculate the # of units in argTypes
int calculate_num(char* buffer)
{
    int num = 0;
    char *it = buffer; 
    while (true) {
        int *n = (int*) it; 
        if (*n == 0) {
            return num+1; 
        }
        num++; 
        it += 4; 
    }
}

static void* execute(void* arguments);  // Prototype

//////////////////////////////////////////////////////////////////////////////////////////
// Helper function to create a connection, to be used by client to connect to server 
// return 0 on success, negative number on failure
// Assuming "sockfd" is already assigned
int connectServer(const char* hostAddr, const char* portno, int *socketnum) 
{
    struct sockaddr_in addr;
    struct hostent *he;
    if ( (he = gethostbyname(hostAddr) ) == NULL ) 
    {
        perror("ERROR gethostbyname");
        return ERROR_CONNECTION; 
    }

    /* copy the network address to sockaddr_in structure */
    addr.sin_family = AF_INET;
    uint16_t port = htons(*(uint16_t*)portno);
    addr.sin_port = port;
    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    *socketnum = socket(PF_INET, SOCK_STREAM, 0);

    if(*socketnum == 0)
    {
        perror("ERROR socket");
        return ERROR_CONNECTION;
    }

    if(connect(*socketnum, (struct sockaddr *)&addr, sizeof(addr))<0)
    {
        perror("ERROR connect");
        return ERROR_CONNECTION;
    }

    return 0; //TO-DO change return value, to indicate error
}

// connect to Binder, and assign a value to (*sockfd)
int ConnectBinder(int* sockfd)
{
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // get BINDER_ADDRESS and BINDER_PORT
    char* server;
    char* server_port;
    if((server = getenv("BINDER_ADDRESS")) == 0)
    {
        cerr << "can't get env variable BINDER_ADDRESS" << endl; 
        return CANT_CONNECT_BINDER; 
    } 
    if((server_port = getenv("BINDER_PORT")) == 0)
    {
        cerr << "can't get env variable BINDER_PORT" << endl; 
        return CANT_CONNECT_BINDER;
    } 

    if ((rv = getaddrinfo(server, server_port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return ERROR_CONNECTION;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        if ((*sockfd = socket(p->ai_family, p->ai_socktype,
                        p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(*sockfd);
            perror("ERROR in binder connecting");
            continue;
        }
        break;
    }
    if (p == NULL) 
    {
        perror("ERROR: can't connect to binder");
        return CANT_CONNECT_BINDER;
    }
    freeaddrinfo(servinfo); // all done with this structure
    return 0; 
}

//Determine the identifier & portno 
//that will be used for clients 
int GetSelfID()
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
        return ERROR_CONNECTION; 
    }

    //make this socket a passive one
    if(listen(clientSocket, MAX_CLIENTS))  // max 5 conns
    {
        perror("ERROR listen");
        return ERROR_CONNECTION; 
    }

    if( getsockname(
        clientSocket, 
        (struct sockaddr*)&addr, 
        (socklen_t*)&addrlen )
         == -1)
    {
        perror("ERROR socketname");
        return ERROR_CONNECTION; 
    }
    gethostname(hostname, sizeof(hostname));

    memcpy(serverID, hostname, SIZE_IDENTIFIER); 
    uint16_t pno = ntohs(addr.sin_port); 
    memcpy(serverPort, (char*)(&pno), SIZE_PORTNO); 

    return 0; 
}

int rpcInit()
{
    // Opcen a client socket for incoming clients
    if ( GetSelfID() < 0)
    {
        return INIT_FAILURE; 
    }
                
    if ( ConnectBinder(&binderSocket) < 0 )
    {
        return INIT_FAILURE; 
    } 

    return INIT_SUCCESS; 
}

int rpcRegister(char* name, int *argTypes, skeleton f)
{    
    //firstly send to binder 
    char* send_buff; 
    int valread;
    int argSize = getTypeLength(argTypes);
    int totalSize = SIZE_IDENTIFIER + SIZE_PORTNO + SIZE_NAME + argSize; 
    send_buff = new char[totalSize + 8];

    memcpy(send_buff, (char*)&totalSize, 4); 
    int t = REGISTER;
    memcpy(send_buff+4, (char*)&t , 4);
    memcpy(send_buff+8, serverID, SIZE_IDENTIFIER);
    memcpy(send_buff+8+SIZE_IDENTIFIER, serverPort, SIZE_PORTNO); 
    memcpy(send_buff+8+SIZE_IDENTIFIER+SIZE_PORTNO, name, SIZE_NAME); 
    memcpy(send_buff+8+SIZE_IDENTIFIER+SIZE_PORTNO+SIZE_NAME, argTypes, argSize);
    write(binderSocket, (void*)send_buff, totalSize+8);

    char size_buff[4];
    valread = read(binderSocket, size_buff, 4);
    uint32_t *type; 
    if(valread < 0)
    {
        perror("ERROR read from Binder, probably due to connection failure");
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
        type = (uint32_t*)type_buff;

        char *buff = new char[*size]; 
        valread = read(binderSocket, buff, *size);
        delete [] buff; 

        if(*type > 0)
        {
            cout<<"Warning # "<< *type;
            if(*type == REGISTER_DUPLICATE) 
            {
                cout<<" : Duplicate Registration";
            }
            cout<<endl; 
        }
    }
    //store to local DB
    serverDatabase.Add(name, argTypes, f);
    return *type; 
}


//////////////////////////////////////////////////////////////////////////////////////////
// rpcCall 
int rpcCall(char* name, int* argTypes, void** args) 
{
    char* buff; 
    int sockfd; 
    int valread;
    char size_buff[4];
    char type_buff[4];
    ConnectBinder(&sockfd);

    // send LOC_REQUEST message to Binder
    int msgLen = (SIZE_NAME + getTypeLength(argTypes));  // name, argTypes

    char send_buff[msgLen + 8];
    unsigned int requestType = LOC_REQUEST;

    memcpy(send_buff, (char *) &msgLen, 4);                 // first 4 bytes stores length of msg
    memcpy(send_buff + 4, (char *) &requestType, 4);          // next 4 bytes stores types info
    memcpy(send_buff + 8, name, SIZE_NAME);                // and then msg = name + argTypes
    memcpy(send_buff + 8 + SIZE_NAME, argTypes, getTypeLength(argTypes)); 

    // send LOC_REQUEST msg to Binder
    if (send(sockfd, send_buff, msgLen + 8, 0) == -1) 
    {
        cerr << "ERROR in sending LOC_REQUEST to Binder" << endl;
    } 

    // wait for reply msg from Binder
    valread = read(sockfd, size_buff, 4);

    if(valread < 0)
    {
        perror("ERROR read from socket, probably due to connection failure");
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
        valread = read(sockfd, type_buff, 4);
        uint32_t *type = (uint32_t*)type_buff;

    	if (*type == LOC_SUCCESS) 
        {                 
    		// now extract server name (128 bytes) and server port (2 bytes)
            buff = new char[*size]; 
            valread = read(sockfd, buff, *size);
            if(valread < 0)
            {
                perror("ERROR read from socket in rpcCall");
                return LOC_FAILURE; 
            }

    		char server_id[SIZE_IDENTIFIER + 1] = {0};
    		char server_port[SIZE_PORTNO + 1] = {0}; 
    		memcpy(server_id, buff, SIZE_IDENTIFIER); 
    		memcpy(server_port, buff+SIZE_IDENTIFIER, SIZE_PORTNO); 

            delete [] buff; 
    		close(sockfd);    // close socket between client and binder

            // Now connect to target server
    		if (connectServer(server_id, server_port, &sockfd) < 0) 
            {
                return RPCCALL_FAILURE; 
            }

            //before yiyao
            int argLen = getArgsLength(argTypes); 
            int type_len = getTypeLength(argTypes); 
            int messageLen = SIZE_NAME + type_len + argLen; // name, argTypes, args
            int requestType = EXECUTE;

            char * buffer = new char[8+messageLen+1];

            memset(buffer, 0, (8+messageLen+3)*sizeof(char));

            memcpy(buffer, (char *) &messageLen, 4);
            memcpy(buffer + 4, (char *) &requestType, 4);
            memcpy(buffer + 8, name, SIZE_NAME); 
            memcpy(buffer + 8 + SIZE_NAME, argTypes, type_len);
            char* packedArgs = pickle(argTypes, args); 
            memcpy(buffer + 8 + SIZE_NAME + type_len, packedArgs, argLen);
            write(sockfd, (void*)buffer, 8 + messageLen); // send EXE request to server
            delete [] buffer;   

            // wait for reply msg from Server
            valread = read(sockfd, size_buff, 4); // get size
            uint32_t *rpy_size = (uint32_t*)size_buff;

            if(valread < 0)
            {
                perror("ERROR read from socket, probably due to connection failure");
                return RPCCALL_FAILURE; 
            }
            else if(valread == 0)
            {
                // Server(s) have terminated before client get result back
                return RPCCALL_FAILURE; 
            }
            else 
            {
                valread = read(sockfd, type_buff, 4); // get type
                int *rpy_type = (int *)type_buff;

                if (*rpy_type == EXECUTE_SUCCESS) 
                {
                    buff = new char[*rpy_size + 8]; 
                    memset(buff, 0, *rpy_size + 8);
                    valread = read(sockfd, buff, *rpy_size);    // name + argTypes + args

                    if(valread <= 0)
                    {
                        perror("ERROR read from socket, probably due to connection failure");
                        return RPCCALL_FAILURE; 
                    }

                    int len_type = getTypeLength(argTypes); // type in byte
                    int num_args = getArgNum(argTypes); // arg number 
                    int len_args = getArgsLength(argTypes); // arg in byte 

                    // Extract new_args from mem block, and override the old args with new_args
                    void **new_args = unpickle(argTypes, buff + SIZE_NAME + len_type);
                    memcpy(args, new_args, num_args*(sizeof(void*)));

                    // we are done with rpcCall(), clean up
                    delete [] buff; 
                    close(sockfd);

                    return RPCCALL_SUCCESS;
                }
                else if (*rpy_type == EXECUTE_FAILURE) 
                {
                    close(sockfd);
                    return RPCCALL_FAILURE;
                } 
                else 
                {
                    close(sockfd);
                    return UNKNOW_ERROR;
                } 
            }  // if(valread > 0)
        } 
        else 
        {
            cerr << "Loc_Request failed" << endl;
            return RPCCALL_FAILURE; 
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// server calls rpcExecute: wait for and receive request 
// forward them to skeletons, and send back teh results
int rpcExecute(void) 
{
    //check if the server called rpcRegister first
    if(serverDatabase.database.size() == 0)
    {
        cerr << "Server hasn't registered anything yet" << endl;
        return NOT_REGISTER;
    }

    fd_set read_fds;  // temp file descriptor list for select()
    int max_sd, sd, new_socket, valread;        // maximum file descriptor number
    socklen_t addrlen;
    struct sockaddr_in addr;
    char size_buff[4];
    char type_buff[4];
    char* buff;
    int clients_sockets[MAX_CLIENTS] = {0};
    std::list<pthread_t> thread_list;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    while(true)
    {
        FD_ZERO(&read_fds); 
        FD_SET(clientSocket, &read_fds); 
        max_sd = clientSocket; 

        for(int i=0; i < MAX_CLIENTS; ++i)
        {
            sd = clients_sockets[i]; 
            if(sd > 0)
                FD_SET(sd, &read_fds); 
            if(sd > max_sd)
                max_sd = sd; 
        }

        if (select(max_sd + 1, &read_fds, NULL, NULL, NULL) == -1) 
        {
            perror("ERROR in select in rpcExecute()");
            return ERROR_CONNECTION; 
        }

        //incoming connection
        if(FD_ISSET(clientSocket, &read_fds))
        {
            if((new_socket  = accept(clientSocket, (struct sockaddr*)&addr, (socklen_t*)&addrlen))<0)
            {
                perror("ERROR accept new socket");
                return ERROR_CONNECTION;
            }

            for(int i=0; i<MAX_CLIENTS; ++i)
            {
                if(clients_sockets[i] == 0)
                {
                    clients_sockets[i] = new_socket;
                    break; 
                }
            }
        }

        // run through the existing connections looking for data to read
        for(int j = 0; j < MAX_CLIENTS; ++j)
        {
            sd = clients_sockets[j]; 
            if ( sd > 0 && FD_ISSET(sd, &read_fds)) 
            {
                // we got one!
                valread = read(sd, size_buff, 4);
                if(valread == 0)
                {
                    //getpeername(sd, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
                    close(sd);
                    clients_sockets[j]=0; 
                }
                else
                {
                    //cout<<"received something from rpcExecute"<<endl;
                    uint32_t *size = (uint32_t*)size_buff; 
                    valread = read(sd, type_buff, 4);
                    uint32_t *type = (uint32_t*)type_buff;

                    if(*type == EXECUTE)
                    {
                        buff = new char[*size];
                        if (recv(sd, buff, *size, 0) < 0) 
                        { // rcvMsg = name + argTypes + args
                            perror("ERROR in receiving msg from client");
                        }
                        int n = calculate_num(buff+ SIZE_NAME);         // get # of argTypes
                        int args_len = *size - SIZE_NAME - n * 4; 
                        char * new_buf = new char[SIZE_SOCK + (*size)];
                        int temp_sock = sd;
                        clients_sockets[j]=0; 

                        memcpy(new_buf, &temp_sock, SIZE_SOCK);
                        memcpy(new_buf+SIZE_SOCK, buff, *size);

                        pthread_t newThread; 
                        thread_list.push_back(newThread); 
                        delete [] buff; 

                        // fork a new thead to handle the calculation by passing data to execute()
                        if (pthread_create(&newThread, NULL, execute, (void*)new_buf)) 
                        {
                            cerr << "ERROR in creating new thread" << endl;
                        }
                    }
                    if(*type == TERMINATE)
                    {
                        buff = new char[*size];
                        if (recv(sd, buff, *size, 0) < 0)
                        {
                            perror("ERROR in receiving msg from binder");
                        }
                        unsigned int *code = (unsigned int*) buff; 
                        if(*code == BINDER_CODE)
                        {
                            terminate_flag = 1;
                            break; 
                        }
                        else
                        {
                            perror("ERROR, client does not have the right to terminate");
                        }
                    }
                    else
                    {
                        valread = read(sd, buff, *size);
                    }
                } 
            } 
        } 
        if (terminate_flag == 1) {
            break; // break  outter loop
        }
    } // END while(true) 

    // wait for all existed threads to finish
    for (std::list<pthread_t>::iterator it = thread_list.begin(); it != thread_list.end(); it++) 
    {
        pthread_join(*it, NULL); // TO_DO: use NULL or some other status?
    }
    return EXECUTE_SUCCESS; 
}


// Passed to a thead, handle the requested calculation 
static void* execute(void* arguments) 
{
    char* buf = (char *) arguments;
    int specialSock;                        // Same as incoming socket, used to send back data 
    memcpy(&specialSock, buf, SIZE_SOCK);

    char name[SIZE_NAME] = {0}; 
    memcpy(name, buf + SIZE_SOCK, SIZE_NAME);
    int* it = (int*)(buf + SIZE_SOCK + SIZE_NAME);
    int type_len = getTypeLength(it);
    int args_len = getArgsLength(it); 
    int* argTypes = new int[type_len];
    memcpy(argTypes, buf+SIZE_SOCK+SIZE_NAME, type_len);

    char* argsBlock = new char[args_len];
    void** args = unpickle(argTypes, (buf+SIZE_SOCK+SIZE_NAME + getTypeLength(argTypes)));  // extract "args"
    
    skeleton skel_func;
    int exeResult = EXECUTE_FAILURE;            // default: EXECUTE_FAILURE

    if (serverDatabase.SearchSkeleton(name, argTypes, &skel_func) == false) 
    { // search in server local DB
        perror("No such skel_func");
    } 
    else {
        exeResult = skel_func(argTypes, args);
    }

    int messageLen; 
    char* buffer;
    if (exeResult == EXECUTE_SUCCESS) 
    {
        char* result_args = new char[args_len]; 
        result_args = pickle(argTypes, args);         // pack "args" into a consecutive mem block for sending
        messageLen = SIZE_NAME + type_len + args_len; // name, argTypes, args
        buffer = new char[8 + messageLen];
        memcpy(buffer, (char *) &messageLen, 4);
        memcpy(buffer+4, (char *) &exeResult, 4);
        memcpy(buffer+8, name, SIZE_NAME); 
        memcpy(buffer+108, argTypes, type_len); 
        memcpy(buffer+108+type_len, result_args, args_len);

    } 
    else {
        // EXECUTE_FAILURE
        exeResult = EXECUTE_FAILURE;
        int reasonCode = -99; // TODO: is this a good reason code?
        messageLen = 4;
        buffer = new char[32];
        memset(buffer, 0, 32);
        memcpy(buffer, (char *) &messageLen, 4);
        memcpy(buffer+4, (char *) &exeResult, 4);
        memcpy(buffer+8, (char *) &reasonCode, 4); 
    }
    if (send(specialSock, buffer, 8 + messageLen, 0) == -1) 
    {
        perror("send ERROR in execute()");
    }

    // Finish Execucation, clean up 
    delete [] argTypes;
    delete [] buffer;
    close(specialSock);
    pthread_exit(NULL);
}


// Clients call rpcTerminate()
int rpcTerminate() 
{
    int sockfd; 

    // re-connect to Binder
    if (ConnectBinder(&sockfd) < 0) {
        cout << "ERROR in connecting to Binder" << endl;
        return CANT_CONNECT_BINDER;      // TO_DO:  need a better meaningful negative number
    }

    // send TERMINATE message to Binder
    int msgLen = 0;     // No following message after type
    char buffer[8];
    unsigned int requestType = TERMINATE;

    memcpy(buffer, (char *) &msgLen, 4);                 // first 4 bytes stores length of msg
    memcpy(buffer+4, (char *) &requestType, 4);          // next 4 bytes stores types info

    // send LOC_REQUEST msg to Binder
    if (send(sockfd, buffer, msgLen+8, 0) == -1) {
        cerr << "ERROR in sending TERMINATE to Binder" << endl;
        return CANT_CONNECT_BINDER;
    } 
    close(sockfd); 
    return TERMINATE_SUCCESS;
}







