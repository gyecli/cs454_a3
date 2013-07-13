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


//#include "/my_test/my_server_function_skels.h"

#define MAX_CLIENTS 20
using namespace std;

//yiyao
int binderSocket; 
int clientSocket; 

char serverID[SIZE_IDENTIFIER];
char serverPort[SIZE_PORTNO];
ServerDB serverDatabase; 

//tim
int specialSock; 
char rcv_name[SIZE_NAME];
int reasonCode; 
fd_set master;    // master file descriptor list (used in rpcExecute())
int terminate_flag = 0;     // 1 means receive terminate request

//



// This is for pthread arguments passing (see rpcExecute() ) 
struct arg_struct {
    int sockfd;         // This is for send()
    char* name;
    int* argTypes;
    void** args;         //TO_DO: type?
};

static void* execute(void* arguments);  // Prototype

//////////////////////////////////////////////////////////////////////////////////////////
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
        exit(-1);
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
        exit(-1);
    }

    cout<<"before connection to server"<<endl;
    cout<<hostAddr<<endl;
    unsigned short *p = (unsigned short*)portno;
    cout<<*p<<endl;

    if(connect(*socketnum, (struct sockaddr *)&addr, sizeof(addr))<0)
    {
        perror("ERROR connect in connectServer()");
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

    cout << "TESTING: in ConnectBinder() " << endl;

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

    cout<<"server id:"<<serverID<<endl;
    unsigned short* p = (unsigned short*)serverPort; 
    cout<<"port no:"<<*p<<endl;
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

    //cout<<"before register, my current porno is:"<<endl;
    //unsigned short *pp = (unsigned short*)serverPort; 
    //cout<<*pp<<endl; 
    memcpy(send_buff, (char*)&totalSize, 4); 
    int t = REGISTER;
    memcpy(send_buff+4, (char*)&t , 4);
    memcpy(send_buff+8, serverID, SIZE_IDENTIFIER);
    memcpy(send_buff+8+SIZE_IDENTIFIER, serverPort, SIZE_PORTNO); 
    memcpy(send_buff+8+SIZE_IDENTIFIER+SIZE_PORTNO, name, SIZE_NAME); 
    memcpy(send_buff+8+SIZE_IDENTIFIER+SIZE_PORTNO+SIZE_NAME, argTypes, argSize);
    write(binderSocket, (void*)send_buff, totalSize+8);

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

        // by tim
        // TODO: There should be a integer follwing message type to indicate warnings or errors (page 6)
        // So we should read 4 more bytes
        char indicator[4]; 
        valread = read(binderSocket, indicator, 4);
        uint32_t *i = (uint32_t*)indicator; 

        if(*type == REGISTER_SUCCESS)
        {
            char warning_buff[4]; 
            valread = read(binderSocket, warning_buff, 4);
            int* warning = (int*) warning_buff; 

            if(*warning > 0)
                cout<<"Warning: "<< *warning<<endl; 
        }  
        else
        {
            char error_buff[4]; 
            valread = read(binderSocket, error_buff, 4);
            int* error_code = (int*) error_buff; 
            cout<<"error:"<<*error_code<<endl;
            return REGISTER_FAILURE; 
        }
    }
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
    valread = read(sockfd, size_buff, 4);

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
        valread = read(sockfd, type_buff, 4);
        uint32_t *type = (uint32_t*)type_buff;

        if (*type == LOC_SUCCESS) 
        {                 
            // now extract server name (128 bytes) and server port (2 bytes)
            cout << "LOC_SUCCESS in rpcCall()" << endl;
            buff = new char[*size]; 
            valread = read(sockfd, buff, *size);
            if(valread < 0)
            {
                error("ERROR read from socket, probably due to connection failure");
                return LOC_FAILURE; 
            }

            char server_id[SIZE_IDENTIFIER + 1] = {0};
            char server_port[SIZE_PORTNO + 1] = {0}; 
            memcpy(server_id, buff, SIZE_IDENTIFIER); 
            memcpy(server_port, buff+SIZE_IDENTIFIER, SIZE_PORTNO); 

            //cout<<"id:"<<string(server_id)<<endl;
            //cout<<"port:" << string(server_port) <<endl;

            close(sockfd);    // close socket between client and binder

            cout<<"before connect to server:"<<endl;
            unsigned short *p = (unsigned short*) server_port;
            cout<<server_id<<endl<<*p<<endl;

            // Now connect to target server
            if (connectServer(server_id, server_port, &sockfd) < 0) {
                cout << "ERROR in connecting to server" << endl;
                return -1; 
            }

            cout<<"connect to server success"<<endl;

            int messageLen = msgLen + getArgsLength(argTypes);  // name, argTypes, args
            requestType = EXECUTE;

            char buffer[8 + messageLen];
            memcpy(buffer, (char *) &messageLen, 4);
            memcpy(buffer+4, (char *) &requestType, 4);
            memcpy(buffer+8, name, SIZE_NAME); 
            memcpy(buffer+8+SIZE_NAME, argTypes, getTypeLength(argTypes)); 
            memcpy(buffer+8+SIZE_NAME+getTypeLength(argTypes), args, getArgsLength(argTypes));

            // send EXECUTE request to server
            if (send(sockfd, buffer, messageLen+8, 0) == -1) 
            {
                cout << "ERROR in sending LOC_REQUEST to Binder" << endl;
                return -1; 
            } 
            cout << "Sent EXECUTE request to server" << endl;

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


int rpcExecute(void) 
{
    fd_set read_fds;  // temp file descriptor list for select()
    int max_sd, sd, new_socket, valread;        // maximum file descriptor number
    socklen_t addrlen;
    struct sockaddr_in addr;
    char size_buff[4];
    char type_buff[4];
    char* buff;
    int clients_sockets[MAX_CLIENTS] = {0};

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // add the clientSocket to the master set
    FD_SET(clientSocket, &master);
    FD_SET(binderSocket, &master);    // TO_DO: can i just add the binderSocket to the set and then listen on it?

    // keep track of the biggest file descriptor
    max_sd = max(clientSocket, binderSocket); // so far, it's this one

    cout << "In rpcExecute(), clientSocket = " << clientSocket << endl;
    cout << "In rpcExecute(), binderSocket = " << binderSocket << endl;

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

        //read_fds = master; // copy it

        if (select(max_sd + 1, &read_fds, NULL, NULL, NULL) == -1) 
        {
            cerr << "ERROR in select in rpcExecute()" << endl;
            exit(4);
        }
        
        //incoming connection
        if(FD_ISSET(clientSocket, &read_fds))
        {
            if((new_socket  = accept(clientSocket, (struct sockaddr*)&addr, (socklen_t*)&addrlen))<0)
            {
                error("ERROR accept new socket");
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
                    getpeername(sd, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
                    close(sd);
                    clients_sockets[j]=0; 
                }
                else
                {
                    cout<<"received something from rpcExecute"<<endl;
                    uint32_t *size = (uint32_t*)size_buff; 
                    valread = read(sd, type_buff, 4);
                    uint32_t *type = (uint32_t*)type_buff;

                    if(*type == EXECUTE)
                    {
                        cout<<"received EXECUTE"<<endl;
                        //buff = new char[size - 8];
                    }
                } 
            } 
        } 

        if (terminate_flag == 1) {
            break; // break for(;;)
        }

    } // END for(;;)--and you thought it would never end!
    return 0; 
}

//////////////////////////////////////////////////////////////////////////////////////////

static void print (void** args) {
    cout << "arg 1: " << *(int *)args[1] << endl;
    cout << "arg 2: " << *(int *)args[2] << endl;
}

//////////////////////////////////////////////////////////////////////////////////////////
// when received request from clients, do the execution here

static void* execute(void* arguments) {
    cout << "\nEntering execute()..." << endl;
    //struct arg_struct *args = (struct arg_struct *)arguments;
    //cout << "name " << (string)args->name << endl;
    //cout << "argTypes_len: " << getTypeLength(args->argTypes) << endl;
    //cout << "args_len: " << getArgsLength(args->argTypes) << endl;
    char* buf = (char *) arguments; 

    char * name = buf; 
    int* argTypes = (int*)(buf+ SIZE_NAME);
    void** args = (void**)(buf+SIZE_NAME+getTypeLength(argTypes)); 

    skeleton skel_func;
    int exeResult = EXECUTE_FAILURE;    

    cout << "name: " << string(name) << endl;
    cout << "argTypes len " << getTypeLength(argTypes) << endl;

    if (serverDatabase.SearchSkeleton(name, argTypes, &skel_func) == false) {    // search in server local DB
        cerr << "No such skel_func" << endl;
    } else {
        cout << "Got desired results" << endl;
        exeResult = skel_func(argTypes, args);
    }
    
    //print(args);
    cout << "aaa" << endl;    
    
    int messageLen; 
    char * ready_buffer;

    if (exeResult == EXECUTE_SUCCESS) {
        messageLen = 100 + getTypeLength(argTypes) + getArgsLength(argTypes);  // name, argTypes, args

        char buffer[8 + messageLen];
        memcpy(buffer, (char *) &messageLen, 4);
        memcpy(buffer+4, (char *) &exeResult, 4);
        memcpy(buffer+8, name, 100); 
        memcpy(buffer+108, argTypes, getTypeLength(argTypes)); 
        memcpy(buffer+108+getTypeLength(argTypes), args, getArgsLength(argTypes));
    
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

    if (FD_ISSET(specialSock, &master)) {
        if (send(specialSock, ready_buffer, 8+messageLen, 0) == -1) {
            cerr << "send" << endl;
        }                      
   }
   pthread_exit(NULL);
}

/*
// no thread version 
void execute(char* name, int *argTypes, void** args, int sock) {
    cout << "\nEntering execute()..." << endl;

    skeleton skel_func;
    int exeResult = EXECUTE_FAILURE;     // 
    if (serverDatabase.SearchSkeleton(name, argTypes, &skel_func) == false) {    // search in server local DB
        cerr << "No such skel_func" << endl;
    } else {
        cout << "Got desired results" << endl;
        exeResult = skel_func(argTypes, args);
    }
    
    cout << "aaa" << endl;    
    
    int messageLen; 
    char * ready_buffer;

    if (exeResult == EXECUTE_SUCCESS) {
        messageLen = 100 + getTypeLength(argTypes) + getArgsLength(argTypes);  // name, argTypes, args

        char buffer[8 + messageLen];
        memcpy(buffer, (char *) &messageLen, 4);
        memcpy(buffer+4, (char *) &exeResult, 4);
        memcpy(buffer+8, name, 100); 
        memcpy(buffer+108, argTypes, getTypeLength(argTypes)); 
        memcpy(buffer+108+getTypeLength(argTypes), args, getArgsLength(argTypes));
    
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

    if (FD_ISSET(sock, &master)) {
        if (send(sock, ready_buffer, 8+messageLen, 0) == -1) {
            cerr << "send" << endl;
        }                      
   }
}
*/


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







