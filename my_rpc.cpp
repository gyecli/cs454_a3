#include "my_rpc.h"

using namespace std; 

//////////////////////////////////
//class definition
/////////////////////
void ServerDB::Add(Prosig function, skeleton location)
{
    list<ProSer>::iterator it = Search(function);
    if(it == database.end())
    {
        //no current result found
        //insert a new element into db
        database.push_back(ProSer(function,location));
    }
    else
    {
        //already an entry for it
        //update the skeleton
        it->second = location; 
    }
}

list<ProSer>::iterator ServerDB::Search(Prosig function)
{
    for(list<ProSer>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        if(function == it->first)
        {
            return it; 
        }
    }
    return database.end(); 

}

//////////////////////////////////////////////////////////////////////////////////////////
// figure out the size (in bytes）of argTypes array, including the "0" at the end; 
int getTypeLength(int* argTypes) {
    int size = 0;
    int* it = argTypes;
    while (*it != 0) {
        size += 4;
        it = it+1;
    }
    return (size +4);
}