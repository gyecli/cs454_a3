#include <string.h>
#include "server_loc.h"
#include "const.h"

ServerLoc::ServerLoc()
{
        //TODO: not sure if i should initialize them
        //identifier = new char[SIZE_NAME]; 
        //portno = new char[portno]; 
}

ServerLoc::ServerLoc(char* identifier, char* portno): identifier(identifier),portno(portno)
{
}

bool ServerLoc::operator == (const ServerLoc &other) const
{
    int n; 
    //compare the memory pointed 
    n = memcmp(this->portno, other.portno, SIZE_PORTNO);
    if(n != 0)
        return false; 
    n = memcmp(this->identifier, other.identifier, SIZE_IDENTIFIER);
    if(n != 0)
        return false;
    return true; 
    }

ServerLoc::~ServerLoc()
{
    delete [] identifier;
    delete [] portno; 
} 
