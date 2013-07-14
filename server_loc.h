#ifndef SERVER_LOC_H
#define SERVER_LOC_H

#include "const.h"

class ServerLoc
{
public:
    char identifier[SIZE_IDENTIFIER + 10]; 
    char portno[SIZE_PORTNO + 10]; 

    //TODO: not sure if I need a copy constructor / assignment function 
    ServerLoc();
    ServerLoc(char* identifier, char* portno);
    bool operator == (const ServerLoc &other) const;
    ~ServerLoc();
};

#endif
