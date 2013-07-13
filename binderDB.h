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
    Prosig first;
    ServerLoc second;
    int third; 
    Tuple();
    Tuple(Prosig first, ServerLoc second, int thrid);
};

class BinderDB
{
private:
    std::list<Tuple>::iterator SearchHelper(Prosig function, ServerLoc ser); 

public: 
    std::list<Tuple> database; 

    int Register(Prosig function, ServerLoc ser, int sockfd);
    int SearchServer(Prosig function, ServerLoc *ser);
    void Cleanup(int sockfd);
};

#endif
