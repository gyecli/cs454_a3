#ifndef BINDERDB_H
#define BINDERDB_H

#include <list>
#include <utility> 
#include "prosig.h"
#include "server_loc.h"
// class Prosig;
// class ServerLoc; 

//typedef std::pair<Prosig, ServerLoc> ProLoc;

class Tuple
{
public:

    int first;
    ServerLoc second;
    std::list<Prosig> third; 

    Tuple();
    Tuple(int first, ServerLoc second, Prosig function);
};

class BinderDB
{
public: 
    std::list<Tuple> database; 

    int Register(Prosig function, ServerLoc ser, int sockfd);
    int SearchServer(Prosig function, ServerLoc *ser);
    void Cleanup(int sockfd);
};

#endif