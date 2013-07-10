#include <iostream>
#include <time.h>
#include <string>
#include <cstring>
#include <map>
#include <list>
#include <utility> 

using namespace std;

//type
#define REGISTER 1
#define LOC_REQUEST 2   

//success 
#define REGISTER_SUCCESS 0 
#define LOC_SUCCESS 0 

//failure 
#define LOC_FAILURE -1 
#define REGISTER_FAILURE -2  

//size
#define SIZE_IDENTIFIER 128
#define SIZE_PORTNO 4
#define SIZE_NAME 100

#define array_size_mask ((1<<17)-1)  



//class Prosig; 
class Server; 
class BinderDB; 
//procedure location 


//*******************************************************************************************************
//Procedure Signiture 
//stores everything we need to compare 2 signitures
//the argNum field is just for easy comparison
class Prosig{
public:
    // TODO: private
	//might not be a good idea to make all fields public
    //will make it like this for now, for the sake of simplicity
    std::string name;
    int argNum; 
    int* argTypes;

    Prosig(std::string name, int argNum, int* argTypes);
    ~Prosig();
    bool operator==(const Prosig &other) const;
};

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

typedef std::pair<Prosig, Server> ProLoc;

//////////////////
//implementation 

Prosig::Prosig(string name, int argNum, int* argTypes):name(name),argNum(argNum), argTypes(argTypes)
{
}

Prosig::~Prosig()
{
    delete [] argTypes;
}

bool Prosig::operator==(const Prosig &other) const 
{
    if(this->name != other.name)    // TO_DO: string comparison not proper?
        return false;
    if(this->argNum != other.argNum)
        return false;

    for(int i=0; i<this->argNum; ++i)
    {
        if(this->argTypes[i] != other.argTypes[i])
            return false; 
        if(this->name != other.name)
            return false;
        if(this->argNum != other.argNum)
            return false;

        for(int i=0; i<this->argNum; ++i)
        {
            if( (this->argTypes[i]>>16) != (other.argTypes[i]>>16))
                return false;
            else
            {
                //TODO: not sure if this is correct
                int len1, len2; 
                len1 = (this->argTypes[i])&array_size_mask; 
                len2 = (other.argTypes[i])&array_size_mask; 
                
                if( (len1 == 1 && len2 != 1) ||
                    (len2 != 1 && len2 ==1 ))
                    return false; 
            }
        }
        return true; 
    }
    return true; 
}

Server::Server()
{
        //TODO: not sure if i should initialize them
        //identifier = new char[SIZE_NAME]; 
        //portno = new char[portno]; 
}

Server::Server(char* identifier, char* portno): identifier(identifier),portno(portno)
{
}

bool Server::operator == (const Server &other) const
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

Server::~Server()
{
    delete [] identifier;
    delete [] portno; 
} 


//*******************************************************************************************************
//binder database
class BinderDB
{
private:
    std::list<ProLoc>::iterator SearchHelper(Prosig function, Server ser); 

public: 
    std::list<ProLoc> database; 

    int Register(Prosig function, Server ser);
    int SearchServer(Prosig function, Server *ser);

};

//to find the position in the list
//where we have the specific function & server info
list<ProLoc>::iterator BinderDB::SearchHelper(Prosig function, Server ser)
{
    for(list<ProLoc>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        if(function == it->first && ser == it->second)
        {
            return it; 
        }
    }
    return database.end(); 
}

int BinderDB::Register(Prosig function, Server ser)
{
    list<ProLoc>::iterator it = SearchHelper(function, ser); 
    if(it == database.end())
    {
        //first time for this server to register this function 
        database.push_back(ProLoc(function, ser));
    }
    //else
        //server already registed this function before
        //no need to do else 

    return REGISTER_SUCCESS; 
}

//search for server given the function prototype 
int BinderDB::SearchServer(Prosig function, Server *ser)
{
    for(list<ProLoc>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        if(function == it->first)
        {
            *ser = it->second;
            return LOC_SUCCESS; 
        }
    }
    return LOC_FAILURE; 
}

//helper function
uint32_t char42int(char* input);
void int2char4(uint32_t n, char* result);
int getTypeLength(int* argTypes);
static void error(string reason);
Prosig MakePro(char* name, int* argTypes);


