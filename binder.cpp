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

//#include "rpc.h"
//#include "my_rpc.h"
#include "binder.h"


#define MAX_CLIENTS 10

using namespace std;

BinderDB db;

int binderRegister(char* received, int size)
{
    char server_id[SIZE_IDENTIFIER]; 
    char portno[SIZE_PORTNO]; 
    char name[SIZE_NAME];
    int* argTypes;

    //TODO: add 8 or not, it to be determined, 
    //dependent on how binder read data from socket 
    //should not add 8 normally
    memcpy(server_id, received, SIZE_IDENTIFIER); 
    memcpy(portno, received + SIZE_IDENTIFIER, SIZE_PORTNO); 
    memcpy(name, received + SIZE_IDENTIFIER + SIZE_PORTNO, SIZE_NAME); 

    int used_size = SIZE_IDENTIFIER + SIZE_PORTNO + SIZE_NAME; 
    char* buff = new char[size - used_size]; 
    memcpy(buff, received + used_size, size - used_size);
    argTypes = (int*)buff; 

    Prosig pro = Prosig(string(name), getTypeLength(argTypes), argTypes);
    Server ser = Server(server_id, portno);
    db.Register(pro, ser); 

    return 0;   // TODO: havn't figured out return type 
}

int loc_Request(char* received, int size, Server *ser)
{
    //char server_id[SIZE_IDENTIFIER]; 
    //char portno[SIZE_PORTNO]; 
    char name[SIZE_NAME];
    char* argTypes = new char[size - SIZE_IDENTIFIER];

    memcpy(name, received, SIZE_IDENTIFIER);
    memcpy(argTypes, received, size - SIZE_IDENTIFIER);

    Prosig function = MakePro(name, (int*)argTypes);
    int result = db.SearchServer(function, ser);
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
                    getpeername(sd, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
                    close(sd);
                    sockets[i]=0; 
                }
                else
                {
                    uint32_t size = char42int(size_buff);
                    valread = read(sd, type_buff, 4);
                    uint32_t type = char42int(type_buff);  

                    buff = new char[size+10]; 
                    valread = read(sd, buff, size+10);

                    if(type == REGISTER)
                    {
                        int result = binderRegister(buff, size); 
                        if(result == REGISTER_SUCCESS)
                        {                            
                            uint32_t length = 0; 
                            char* sendChar = new char[8 + length];
                            char lengthChar[4]; 
                            char resultChar[4]; 
                            int2char4(length, lengthChar);
                            int2char4(result, resultChar); 
                            memcpy(sendChar, lengthChar, 4); 
                            memcpy(sendChar, resultChar, 4);
                            send(sd, sendChar, 8, 0); 
                        }
                        else if(result == REGISTER_FAILURE)
                        {
                            uint32_t length = 4; 
                            char* sendChar = new char[8 + length];
                            char lengthChar[4]; 
                            char resultChar[4]; 
                            int2char4(length, lengthChar);
                            int2char4(result, resultChar); 
                            memcpy(sendChar, lengthChar, 4); 
                            memcpy(sendChar, resultChar, 4);
                            //TODO: 
                            //send(sd, sendChar, length + 8, 0); 
                        }
                    }
                    else if(type == LOC_REQUEST)
                    {
                        Server ser; 
                        int result = loc_Request(buff, size, &ser); 
                        if(result == LOC_SUCCESS)
                        {
                            char* sendChar = new char[4 + SIZE_IDENTIFIER + SIZE_PORTNO];
                            char resultChar[4]; 
                            int2char4(result, resultChar);
                            memcpy(sendChar, resultChar, 4); 

                            //send();
                        }
                        else if(result == LOC_FAILURE)
                        {

                        }
                    }
                    delete [] buff;
                }
            }
        }
    }

    close(master_socket);
    return 0;
}

uint32_t char42int(char* input)
{
    uint32_t result;
    result = (uint32_t)input[3];
    result += ((uint32_t)input[2])<<8; 
    result += ((uint32_t)input[1])<<16;
    result += ((uint32_t)input[0])<<24;

    return result;
}

void int2char4(uint32_t n, char* result)
{
    result[0] = (n >> 24) & 0xFF;
    result[1] = (n >> 16) & 0xFF;
    result[2] = (n >> 8) & 0xFF;
    result[3] = n & 0xFF;
}

static void error(string reason)
{
    #ifdef _DEBUG
        cout<<reason<<endl; 
    #endif
}


Prosig MakePro(char* name, int* argTypes)
{
    Prosig function(string(name), getTypeLength(argTypes), argTypes); 
    return function; 
}

int getTypeLength(int* argTypes) 
{
    int size = 0;
    int* it = argTypes;
    while (*it != 0) {
        size += 4;
        it = it+1;
    }
    return (size +4);
}