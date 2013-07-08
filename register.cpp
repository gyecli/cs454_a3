#include <iostream>
#include <time.h>
#include <string>
#include <map>
#include <list>
#include <utility> 

using namespace std; 
class Prosig; 
class BinderDB; 

//Procedure Signiture 
//stores everything we need to compare 2 signitures
//the argNum field is just for easy comparison
class Prosig{
public:
    string name;
    int argNum; 
    int* argTypes;
    //might not be a good idea to make all fields public
    //will make it like this for now, for the sake of simplicity 

    Prosig(string name, int argNum):name(name),argNum(argNum)
    {
        argTypes = new int[argNum]; 
    }

    ~Prosig()
    {
        delete [] argTypes;
    }

    bool operator <(const Prosig& rhs) const
    {
        return argNum < rhs.argNum;
    }

    bool operator==(const Prosig &other) const 
    {
        if(this->name != other.name)
            return false;
        if(this->argNum != other.argNum)
            return false;

        for(int i=0; i<this->argNum; ++i)
        {
            if(this->argTypes[i] != other.argTypes[i])
                return false; 
        }
        return true; 
    }
    
};

typedef pair<char*, char*> server;

class BinderDB
{
public: 
    map<class Prosig, list<server> > database; 
    void Add(Prosig function, char* server_id, char* portno)
    {
        server thisServer(server_id, portno); 
        //if(true)
        if(database.count(function))
        {
            //such funtion has already exist 
            //from at least one server
            list<server> currentList = database[function]; 
            list<server>::iterator findIter = find(currentList.begin(), currentList.end(), thisServer);
            if(findIter = currentList.end())
            {
                //the server is already in the list, do nothing but return
                return; 
            }
            else
            {
                currentList.push_front(thisServer);
            }
        }
        else
        {
            //first time to see such a function
            list<server> serverList; 
            serverList.push_front(thisServer);
            database.insert(make_pair(function, serverList));
        }
    }
};

/*
class Servers
{
    map<, t_clock> serverList; 
public:
};
*/

