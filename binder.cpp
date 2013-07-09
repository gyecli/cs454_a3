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

#include "rpc.h"
#include "my_rpc.h"
#include "binder.h"

#define MAX_CLIENTS 20

//TODO: to be moved to other places 
#define REGISTER 1          // Type of requests from servers
#define LOC_REQUEST 2       // Type of requests from clients

using namespace std;

BinderDB db;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
int binderInit(void)
{
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    char hostName[128];   // host name of local machine

    int nbytes;

    struct sockaddr_in addr;
    int s_len;

    //char remoteIP[INET6_ADDRSTRLEN];

    int i;

    //struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    //******************************************************************
    // get a free port
    listener = socket(PF_INET, SOCK_STREAM, 0);

    if(listener == 0)
    {
        cerr << "ERROR listen" << endl;
        exit(-1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = INADDR_ANY;
    s_len = sizeof(addr);

    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr))<0)
    {
        cerr << "ERROR bind" << endl;
        exit(-1);
    }
    if(listen(listener, MAX_CLIENTS))  // max 5 conns
    {
        cerr << "ERROR listen: too many client coneection requests" << endl;
        exit(-1);
    }

    if(getsockname(listener, (struct sockaddr*)&addr, (socklen_t*)&s_len ) == -1)
    {
        cerr << "ERROR getsockname" << endl;
        exit(-1);
    }

    gethostname(hostName, sizeof(hostName));

    cout << "BINDER_ADDRESS " << hostName << endl;
    cout << "BINDER_PORT " << ntohs(addr.sin_port) << endl;

    //*********************************************************************************
    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            cerr << "ERROR select" << endl;
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

                    if (newfd == -1) {
                        cerr << "ERROR accept" << endl;
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                    }
                } else {
                    // handle data from a client
                    char buf[8];    // buffer for first 8 bytes

                    if ((nbytes = recv(i, buf, sizeof buf, 0)) < 0) {
                        // got error or connection closed by client
                        cerr << "ERROR recv()" << endl;
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        // we got some data from a client
                        
                        char rcv_len[4];
                        char rcv_type[4]; 
                        memcpy(rcv_len, buf, 4); 
                        memcpy(rcv_type, buf+4, 4); 

                        int length = atoi (rcv_len);
                        int type = atoi (rcv_type);

                        if (type == REGISTER) {
                            // It's a register request from server
                            // register to the DB

                            // TODO: not sure yet.
                            char message[length];
                            if (recv(i, message, length, 0) < 0) {
                                cout << "ERROR receiving message from server" << endl;
                                exit(1);
                            }

                            binderRegister(message, length); 

                        } else if (type == LOC_REQUEST) {
                            // It's a location request from client
                            // DB lookup
                        }
                        if (FD_ISSET(i, &master) && i != listener) {
                            // except the listener
                            if (send(i, buf, nbytes, 0) == -1) {
                                cerr << "ERROR send" << endl;
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!

    return 0;
}

int binderRegister(char* received, int size)
{
    char server_id[SIZE_IDENTIFIER]; 
    char portno[SIZE_PORTNO]; 
    char name[SIZE_NAME];
    int* argTypes;
    memcpy(server_id, received + 8, SIZE_IDENTIFIER); 
    memcpy(portno, received + 8 + SIZE_IDENTIFIER, SIZE_PORTNO); 
    memcpy(name, received + 8 + SIZE_IDENTIFIER + SIZE_PORTNO, SIZE_NAME); 

    int used_size = SIZE_IDENTIFIER + SIZE_PORTNO + SIZE_NAME; 
    char* buff = new char[size - used_size]; 
    memcpy(buff, received + 8 + used_size, size - used_size);
    argTypes = (int*)buff; 

    Prosig pro = Prosig(string(name), getTypeLength(argTypes), argTypes);
    Server ser = Server(server_id, portno);
    db.Register(pro, ser); 

    return 0;   // TODO: havn't figured out return type 
}

int main()
{
    binderInit();
}
