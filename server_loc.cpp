#include <string.h>
#include <iostream>
#include "server_loc.h"
#include "const.h"

//TODO: delete these

using namespace std;

ServerLoc::ServerLoc()
{
        //TODO: not sure if i should initialize them
        //identifier = new char[SIZE_NAME]; 
        //portno = new char[portno]; 
}

ServerLoc::ServerLoc(char* identifier, char* portno)
{
    memset(this->identifier, 0, SIZE_IDENTIFIER + 10);
    memset(this->portno, 0, SIZE_PORTNO + 10);
    memcpy(this->identifier, identifier, SIZE_IDENTIFIER); 
    memcpy(this->portno, portno, SIZE_PORTNO); 
}

bool ServerLoc::operator == (const ServerLoc &other) const
{
    unsigned char *portLeft = (unsigned char*) this->portno;
    unsigned char *portRight = (unsigned char*) other.portno;

    if(*portLeft != *portRight)
    {
        return false; 
    }
    if(strcmp(this->identifier, other.identifier) != 0)
    {
        return false; 
    }

    return true; 
}

ServerLoc::~ServerLoc()
{
    //delete [] identifier;
    //delete [] portno; 
} 
