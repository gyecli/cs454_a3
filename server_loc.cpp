#include <string.h>
#include "server_loc.h"
#include "const.h"

//TODO: delete these
#include <iostream>
using namespace std;

ServerLoc::ServerLoc()
{
        //TODO: not sure if i should initialize them
        //identifier = new char[SIZE_NAME]; 
        //portno = new char[portno]; 
}

ServerLoc::ServerLoc(char* identifier, char* portno)
{
    strcpy(this->identifier, identifier); 
    strcpy(this->portno, portno); 
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
