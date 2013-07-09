#include <iostream>
#include <time.h>
#include <string>
#include <map>
#include <list>
#include <utility> 


class Prosig; 
class Server; 
class BinderDB; 
//procedure location 
typedef std::pair<Prosig, Server> ProLoc;
int getTypeLength(int* argTypes); 

//Procedure Signiture 
//stores everything we need to compare 2 signitures
//the argNum field is just for easy comparison
class Prosig{
public:
	//might not be a good idea to make all fields public
    //will make it like this for now, for the sake of simplicity
    std::string name;
    int argNum; 
    int* argTypes;

    Prosig(std::string name, int argNum, int* argTypes);
    ~Prosig();
    bool operator==(const Prosig &other) const;
};

//server
class Server
{
public:
    char* identifier; 
    char* portno; 

    //TODO: not sure if I need a copy constructor / assignment function 
    Server();
    Server(char* identifier, char* portno);
    bool operator == (const Server &other) const;
    ~Server();
};


//binder database
class BinderDB
{
public: 
    std::list<ProLoc> database; 

    int Register(Prosig function, Server ser);
    std::list<ProLoc>::iterator SearchAll(Prosig function, Server ser); 
    bool Search(Prosig function, Server *ser);
};