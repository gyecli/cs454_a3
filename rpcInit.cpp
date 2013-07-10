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

using namespace std; 

int sockfd;

int rpcInit()
{
	struct sockaddr_in addr;
    char* hostAddr, *portno; 
    //pthread_t thread1, thread2, thread3; 
    //int iret1, iret2, iret3; 

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

    cout<<"connection!"<<endl;
}


int rpcRegister(char* name, int *argTypes, skeleton f)
{
    //firstly send to binder 
    char* send; 
    int argSize = getTypeLength(argTypes);
    int totalSize = 4 + SIZE_IDENTIFIER + SIZE_PORTNO + SIZE_NAME + argSize; 

    //marshall everything into the stream to binder 
    memcpy(send, (char*)&totalSize, 4); 
    memcpy(send+4, (char*)REGISTER , 4);
    memcpy(send+8, , SIZE_NAME);
    memcpy(send+8+SIZE_IDENTIFIER, , SIZE_PORTNO); 

    write(sockfd, (void*)send, n+4);

    //store to local DB

}

int main()
{
    rpcInit();
}