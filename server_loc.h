#ifndef SERVER_LOC_H
#define SERVER_LOC_H

class ServerLoc
{
public:
    char* identifier; 
    char* portno; 

    //TODO: not sure if I need a copy constructor / assignment function 
    ServerLoc();
    ServerLoc(char* identifier, char* portno);
    bool operator == (const ServerLoc &other) const;
    ~ServerLoc();
};

#endif
