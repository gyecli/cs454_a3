#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>  // ip
#include <unistd.h>  // write
#include <arpa/inet.h>  // inet_addr
#include <string>
#include <pthread.h>
#include <queue>
#include <netdb.h>
#include <stdlib.h>

#include "rpc.h"
#include "my_rpc.h"

#define MAX_CLIENTS 5

using namespace std; 
//class serverDB; 
//class Prosig; 

int sockfd;
char serverID[SIZE_IDENTIFIER];
char serverPort[SIZE_PORTNO];
ServerDB serverDatabase; 

//////////////////////////////////
//class definition
/////////////////////
void ServerDB::Add(Prosig function, skeleton location)
{
    list<ProSer>::iterator it = Search(function);
    if(it == database.end())
    {
        //no current result found
        //insert a new element into db
        database.push_back(ProSer(function,location));
    }
    else
    {
        //already an entry for it
        //update the skeleton
        it->second = location; 
    }
}

list<ProSer>::iterator ServerDB::Search(Prosig function)
{
    skeleton result = NULL; 
    for(list<ProSer>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        if(function == it->first)
        {
            return it; 
        }
    }
    return database.end(); 

}

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

void ConnectBinder()
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
    sockfd = socket(PF_INET, SOCK_STREAM, 0);

    if(sockfd == 0)
    {
        perror("ERROR socket");
        exit(-1);
    }

    if(connect(sockfd, (struct sockaddr *)&addr, sizeof(addr))<0)
    {
        perror("ERROR connect");
        exit(-1);
    }
}

//Determine the identifier & portno 
//that will be used for clients 
void GetSelfID()
{
    int server_socket;
    struct sockaddr_in addr;
    int addrlen, valread; 
    char hostname[SIZE_IDENTIFIER];
    fd_set readfds; 
    int clients_sockets[MAX_CLIENTS] = {0};
    int max_sd, sd, activity; 

    //server socket, which will be connected by clients later
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(server_socket == 0)
    {
        perror("ERROR socket");
        exit(-1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = INADDR_ANY;
    addrlen = sizeof(addr); 

    if(bind(server_socket, (struct sockaddr *)&addr, sizeof(addr))<0)
    {
        perror("ERROR bind"); 
        exit(-1);
    }

    //make this socket a passive one
    if(listen(server_socket, MAX_CLIENTS))  // max 5 conns
    {
        perror("ERROR listen");
        exit(-1);
    }

    if( getsockname(
        server_socket, 
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
    ConnectBinder(); 
    
    cout<<"connection!"<<endl;
}


int rpcRegister(char* name, int *argTypes, skeleton f)
{
    //firstly send to binder 
    char* send; 
    int argSize = getTypeLength(argTypes);
    int totalSize = SIZE_IDENTIFIER + SIZE_PORTNO + SIZE_NAME + argSize; 
    send = new char[totalSize + 8];

    //marshall everything into the stream to binder 
    memcpy(send, (char*)&totalSize, 4); 
    memcpy(send+4, (char*)REGISTER , 4);
    memcpy(send+8, serverID, SIZE_IDENTIFIER);
    memcpy(send+8+SIZE_IDENTIFIER, serverPort, SIZE_PORTNO); 
    memcpy(send+8+SIZE_IDENTIFIER+SIZE_PORTNO, name, SIZE_NAME); 
    memcpy(send+8+SIZE_IDENTIFIER+SIZE_PORTNO+SIZE_NAME, argTypes, argSize);

    write(sockfd, (void*)send, totalSize+8);

    //store to local DB
    serverDatabase.Add(Prosig((string)name, getTypeLength(argTypes), argTypes), f);
}
