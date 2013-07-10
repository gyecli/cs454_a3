#include <iostream>
#include <time.h>
#include <string>
#include <cstring>
#include <map>
#include <list>
#include <utility> 

using namespace std;

class Prosig; 
class Server; 
class BinderDB; 
//procedure location 
typedef std::pair<Prosig, Server> ProLoc;


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


//*******************************************************************************************************
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
public: 
    std::list<ProLoc> database; 

    int Register(Prosig function, Server ser);
    std::list<ProLoc>::iterator SearchAll(Prosig function, Server ser); 
    bool Search(Prosig function, Server *ser);
};


int BinderDB::Register(Prosig function, Server ser)
{
    list<ProLoc>::iterator it = SearchAll(function, ser); 
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

list<ProLoc>::iterator BinderDB::SearchAll(Prosig function, Server ser)
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

bool BinderDB::Search(Prosig function, Server *ser)
{
    for(list<ProLoc>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        if(function == it->first)
        {
            *ser = it->second;
            return true; 
        }
    }
    return false; 
}
