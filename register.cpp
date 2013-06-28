#include <iostream>
#include <time.h>

//Procedure Signiture 
//stores everything we need to compare 2 signitures
//the argNum field is just for easy comparison
class Prosig{
    string name;
    int argNum; 
    int* argTypes;
public:
    Prosig(string name, int argNum):name(name),argNum(argNum)
    {
        argTypes = new int[argNum]; 
    }

    ~prosig()
    {
        delete [] argTypes;
    }

    bool Prosig::operator==(const Prosig &other) const {
        if(this->name != other.name)
            return false;
        if(this->argNum != other.argNum)
            return false;

        for(int i=0; i<this0>argNum; ++i)
        {
            if(this->argTypes[i] != other.argTypes[i])
                return false; 
        }
        return true; 
    }
};

class Servers
{
    map<, t_clock> serverList; 
public:
};

