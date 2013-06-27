#include <iostream>
#include <string>
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

#define MAX_CLIENTS 10

using namespace std;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    char hostName[128];   // host name of local machine

    char buf[256];    // buffer for client data
    int nbytes;

    struct sockaddr_in addr;
    int s_len;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
          ////////////
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
                    newfd = accept(listener,
                                  (struct sockaddr *)&remoteaddr,
                                  &addrlen);

                    if (newfd == -1) {
                        cerr << "ERROR accept" << endl;
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
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            cout << "selectserver: socket %d hung up\n" << i << endl;
                        } else {
                            cerr << "ERROR recv()" << endl;
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        // we got some data from a client
                        for(j = 0; j <= fdmax; j++) {
                            // send to everyone!
                            if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if (j != listener && j != i) {
                                    if (send(j, buf, nbytes, 0) == -1) {
                                        cerr << "ERROR send" << endl;
                                    }
                                }
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!

    return 0;
}
