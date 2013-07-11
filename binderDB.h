#ifndef BINDERDB_H
#define BINDERDB_H

#include <list>
#include <utility> 

class Prosig;
class ServerLoc; 

typedef std::pair<Prosig, ServerLoc> ProLoc;

class BinderDB
{
private:
    std::list<ProLoc>::iterator SearchHelper(Prosig function, ServerLoc ser); 

public: 
    std::list<ProLoc> database; 

    int Register(Prosig function, ServerLoc ser);
    int SearchServer(Prosig function, ServerLoc *ser);
};

#endif
