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

#include "const.h"
#include "prosig.h"
#include "server_loc.h"      // TODO: comment out for u, cuz no such header file
#include "binderDB.h"
#include "helper.h"

#define MAX_CLIENTS 20

using namespace std;

BinderDB binder_database;

int binderRegister(char* received, int size, int sockfd)
{
    char server_id[SIZE_IDENTIFIER]; 
    char portno[SIZE_PORTNO]; 
    char name[SIZE_NAME];
    int* argTypes;

    memcpy(server_id, received, SIZE_IDENTIFIER); 
    memcpy(portno, received + SIZE_IDENTIFIER, SIZE_PORTNO); 
    memcpy(name, received + SIZE_IDENTIFIER + SIZE_PORTNO, SIZE_NAME); 

    int used_size = SIZE_IDENTIFIER + SIZE_PORTNO + SIZE_NAME; 
    char* buff = new char[size - used_size]; 
    memcpy(buff, received + used_size, size - used_size);
    argTypes = (int*)buff; 

    Prosig pro = MakePro(name, argTypes);
    ServerLoc ser = ServerLoc(server_id, portno);
    int result = binder_database.Register(pro, ser, sockfd); 

    return result; 
}

int Loc_Request(char* received, int size, ServerLoc *ser)
{
    char name[SIZE_NAME];
    char* argTypes = new char[size - SIZE_NAME];
    memcpy(name, received, SIZE_NAME);
    memcpy(argTypes, received + SIZE_NAME, size - SIZE_NAME);
    Prosig function = MakePro(name, (int*)argTypes);
    int result = binder_database.SearchServer(function, ser);

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
    char *buff;
    fd_set readfds; 
    int sockets[MAX_CLIENTS] = {0};
    int max_sd, sd, activity; 

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
        activity = select(max_sd+1, &readfds, NULL, NULL, NULL); 

        //incoming connection
        if(FD_ISSET(master_socket, &readfds))
        {
            if((new_socket  = accept(master_socket, (struct sockaddr*)&addr, (socklen_t*)&addrlen))<0)
            {
                error("ERROR accept new socket");
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

                if(valread == 0)
                {
                    cout<<"closed server"<<endl;
                    binder_database.Cleanup(sd);
                    close(sd);
                    sockets[i]=0; 
                    //clear this server in database
                }
                else
                {
                    uint32_t *size = (uint32_t*)size_buff;
                    //cout<<"size:"<< *size <<endl;
                    valread = read(sd, type_buff, 4);
                    uint32_t *type = (uint32_t*)type_buff;  

                    buff = new char[*size+10]; 
                    valread = read(sd, buff, *size);
                    buff[*size] = '\0';

                    if(*type == REGISTER)
                    {
                        //cout<<"received register"<<endl;
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
                    }
                    else if(*type == LOC_REQUEST)
                    {
                        ServerLoc ser; 
                        int result = Loc_Request(buff, *size, &ser);

                        if(result == LOC_SUCCESS)
                        {
                            int length = SIZE_IDENTIFIER + SIZE_PORTNO;
                            char* sendChar = new char[8 + length];

                            cout<<"before sending to client"<<endl;
                            cout<<ser.identifier<<endl;
                            unsigned short *p = (unsigned short*)ser.portno;
                            cout<<*p<<endl;

                            memcpy(sendChar, (char*)&length, 4); 
                            memcpy(sendChar + 4, (char*)&result, 4);
                            memcpy(sendChar + 8, ser.identifier, SIZE_IDENTIFIER);
                            memcpy(sendChar + 8 + SIZE_IDENTIFIER, ser.portno, SIZE_PORTNO); 

                            send(sd, sendChar, length + 8, 0);
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
                        }
                    }
                    else
                    {
                        cout<<"===================received what?:"<<*type<<endl;
                    }
                    delete [] buff;
                }
            }
        }
    }
    close(master_socket);
    return 0;
}
