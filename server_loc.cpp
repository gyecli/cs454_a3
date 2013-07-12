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
    //delete [] identifier;
    //delete [] portno; 
} 
