#include <iostream>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <cassert>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <cstdio>
#include <pthread.h>

#include "const.h"
#include "prosig.h"
#include "server_loc.h"      // TODO: comment out for u, cuz no such header file
#include "binderDB.h"
#include "helper.h"

#define MAX_CLIENTS 20

using namespace std;

BinderDB binder_database;

int connectServer(const char* hostAddr, const char* portno, int *socketnum) 
{
    struct sockaddr_in addr;
    struct hostent *he;
    if ( (he = gethostbyname(hostAddr) ) == NULL ) 
    {
        perror("ERROR gethostbyname");
        return ERROR_CONNECCTION; 
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
        return ERROR_CONNECCTION; 
    }

    if(connect(*socketnum, (struct sockaddr *)&addr, sizeof(addr))<0)
    {
        perror("ERROR connect");
        return ERROR_CONNECCTION; 
    }

    return 0; 
}


int binderRegister(char* received, int size, int sockfd)
{
    char server_id[SIZE_IDENTIFIER] = {0}; 
    char portno[SIZE_PORTNO] = {0}; 
    char name[SIZE_NAME] = {0};
    int* argTypes;

    memcpy(server_id, received, SIZE_IDENTIFIER); 
    memcpy(portno, received + SIZE_IDENTIFIER, SIZE_PORTNO); 
    memcpy(name, received + SIZE_IDENTIFIER + SIZE_PORTNO, SIZE_NAME); 

    int used_size = SIZE_IDENTIFIER + SIZE_PORTNO + SIZE_NAME; 
    char* buff = new char[size - used_size]; 
    memcpy(buff, received + used_size, size - used_size);
    argTypes = (int*)buff; 
    Prosig *function = MakePro(name, argTypes);
    ServerLoc ser = ServerLoc(server_id, portno);
    int result = binder_database.Register(*function, ser, sockfd); 

    return result; 
}

int Loc_Request(char* received, int size, ServerLoc *ser)
{
    char name[SIZE_NAME];
    char* argTypes = new char[size - SIZE_NAME];
    memcpy(name, received, SIZE_NAME);
    memcpy(argTypes, received + SIZE_NAME, size - SIZE_NAME);
    Prosig *function = MakePro(name, (int*)argTypes);
    int result = binder_database.SearchServer(*function, ser);

    return result; 
}

int main()
{
    int master_socket, new_socket;
    struct sockaddr_in addr;
    int addrlen, valread; 
    char hostname[128];
    char size_buff[4];
    char type_buff[4];
    //char *buff;
    fd_set readfds; 
    int sockets[MAX_CLIENTS] = {0};
    int max_sd, sd; 

    //master socket
    master_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(master_socket == 0)
    {
        perror("ERROR socket");
        exit(-1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = INADDR_ANY;
    addrlen = sizeof(addr); 

    if(bind(master_socket, (struct sockaddr *)&addr, sizeof(addr))<0)
    {
        perror("ERROR bind"); 
        exit(-1);
    }

    if(listen(master_socket, MAX_CLIENTS))  // max 5 conns
    {
        perror("ERROR listen");
        exit(-1);
    }

    if( getsockname(
        master_socket, 
        (struct sockaddr*)&addr, 
        (socklen_t*)&addrlen )
         == -1)
    {
        perror("ERROR socketname");
        exit(-1);
    }
    gethostname(hostname, sizeof(hostname));

    cout<<"BINDER_ADDRESS "<<hostname<<endl;
    cout<<"BINDER_PORT "<<ntohs(addr.sin_port)<<endl;

    bool terminate = false; 

    while(true)
    {
        FD_ZERO(&readfds); 
        FD_SET(master_socket, &readfds); 
        max_sd = master_socket; 

        for(int i=0; i < MAX_CLIENTS; ++i)
        {
            sd = sockets[i]; 
            if(sd>0)
                FD_SET(sd, &readfds); 
            if(sd>max_sd)
                max_sd = sd; 
        }

        //selct blocks until there's an activity
        if( select(max_sd+1, &readfds, NULL, NULL, NULL) == -1 )
        {
            perror("ERROR select");
        }


        //incoming connection
        if(FD_ISSET(master_socket, &readfds))
        {
            if((new_socket  = accept(master_socket, (struct sockaddr*)&addr, (socklen_t*)&addrlen))<0)
            {
                perror("ERROR accept new socket");
            }
            
            for(int i=0; i<MAX_CLIENTS; ++i)
            {
                if(sockets[i] == 0)
                {
                    sockets[i] = new_socket;
                    break; 
                }
            }
        }

        for(int i=0; i<MAX_CLIENTS; ++i)
        {
            sd = sockets[i]; 
            if(FD_ISSET(sd, &readfds))
            {
                valread = read(sd, size_buff, 4);

                if(valread < 0)
                {
                    perror("ERROR read");
                }
                else if(valread == 0)
                {
                    //clear this server in database
                    binder_database.Cleanup(sd);
                    close(sd);
                    sockets[i]=0; 
                }
                else
                {
                    uint32_t *size = (uint32_t*)size_buff;
                    valread = read(sd, type_buff, 4);
                    uint32_t *type = (uint32_t*)type_buff;  

                    if(*type == TERMINATE)
                    {
                        terminate = true; 
                        unsigned int size = 4; 
                        unsigned int type = TERMINATE; 
                        unsigned int code = BINDER_CODE; 
                        char* sendChar = new char[8 + size];
                        memcpy(sendChar, (char*)&size, 4);
                        memcpy(sendChar + 4, (char*)&type, 4);
                        memcpy(sendChar + 8, (char*)&code, 4); 

                        for(list<Tuple>::iterator it = binder_database.database.begin(); it != binder_database.database.end(); ++it)
                        {
                            char id[SIZE_IDENTIFIER + 1] = {0};
                            char port[SIZE_PORTNO + 1] = {0}; 

                            memcpy(id, it->second.identifier, SIZE_IDENTIFIER);
                            memcpy(port, it->second.portno, SIZE_PORTNO);

                            int curr_sk;
                            connectServer(id, port, &curr_sk);
                            if(curr_sk > 0)
                            {
                                int r = send(curr_sk, sendChar, 8 + size, 0);
                                if(  r == -1)
                                {
                                    perror("error sending TERMINATE to server"); 
                                }
                                else if( r ==0)
                                {
                                    perror("Can't connect to this server, maybe it's already terminated"); 
                                }
                            }
                            else
                            {
                                perror("error in connecting to server");
                            }
                        }
                        delete [] sendChar; 
                    }

                    if(terminate == true)
                        break; 

                    char* buff = new char[*size];
                    memset(buff, 0, *size); 
                    valread = read(sd, buff, *size);
                    if(valread < 0)
                    {
                        perror("ERROR read");
                    }
                    else if(valread == 0)
                    {
                        //clear this server in database
                        binder_database.Cleanup(sd);
                        close(sd);
                        sockets[i]=0; 
                    }
                    else
                    {

                        if(*type == REGISTER)
                        {
                            int result = binderRegister(buff, *size, sd); 
                            uint32_t length = 4; 
                            char* sendChar = new char[8 + length];
                            int success; 

                            if(result == REGISTER_SUCCESS)
                            {
                                success = REGISTER_SUCCESS;
                                result = 0; 
                            }
                            else if(result == REGISTER_DUPLICATE)
                            {
                                success = REGISTER_SUCCESS;
                            }
                            else
                            {
                                success = REGISTER_FAILURE; 
                            }
                            memcpy(sendChar, (char*)&length, 4); 
                            memcpy(sendChar + 4, (char*)&success, 4);
                            memcpy(sendChar + 8, (char*)&result, 4); 
                            send(sd, sendChar, 8 + length, 0); 
                            delete [] sendChar; 
                        }
                        else if(*type == LOC_REQUEST)
                        {
                            ServerLoc ser; 
                            int result = Loc_Request(buff, *size, &ser);

                            if(result == LOC_SUCCESS)
                            {
                                int length = SIZE_IDENTIFIER + SIZE_PORTNO;
                                char* sendChar = new char[8 + length];

                                memcpy(sendChar, (char*)&length, 4); 
                                memcpy(sendChar + 4, (char*)&result, 4);
                                memcpy(sendChar + 8, ser.identifier, SIZE_IDENTIFIER);
                                memcpy(sendChar + 8 + SIZE_IDENTIFIER, ser.portno, SIZE_PORTNO); 
                                send(sd, sendChar, length + 8, 0);

                                delete [] sendChar; 
                            }
                            else if(result != LOC_SUCCESS)
                            {
                                int length = 4; 
                                char* sendChar = new char[8 + length];
                                memcpy(sendChar, (char*)&length, 4); 
                                int f = LOC_FAILURE;
                                memcpy(sendChar + 4, (char*)&f, 4);
                                memcpy(sendChar + 8, (char*)&result, 4);
                                send(sd, sendChar, length + 8, 0); 

                                delete [] sendChar; 
                            }
                        }
                        else
                        {
                            perror("ERROR can't recognize request type");
                            return UNKNOW_REQUEST; 
                        }
                        delete [] buff; 
                    }
                }
            }
            if(terminate == true)
                 break; 
        }
        if(terminate == true)
             break; 
    }
    close(master_socket);
    return 0;
}

