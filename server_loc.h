#ifndef SERVER_LOC_H
#define SERVER_LOC_H

#include "const.h"

class ServerLoc
{
public:
    char identifier[SIZE_IDENTIFIER]; 
    char portno[SIZE_PORTNO]; 

    //TODO: not sure if I need a copy constructor / assignment function 
    ServerLoc();
    ServerLoc(char* identifier, char* portno);
    bool operator == (const ServerLoc &other) const;
    ~ServerLoc();
};

#endif
