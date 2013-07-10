#include "my_rpc.h"

using namespace std; 

//////////////////////////////////
//class definition
/////////////////////
void ServerDB::Add(char* name, int* argTypes, skeleton location)
{
    Prosig function = MakePro(name, argTypes);

    list<ProSer>::iterator it = SearchHelper(name, argTypes);
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

list<ProSer>::iterator ServerDB::SearchHelper(char* name, int* argTypes)
{
    Prosig function = MakePro(name, argTypes);

    for(list<ProSer>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        if(function == it->first)
        {
            return it; 
        }
    }
    return database.end(); 
}

skeleton ServerDB::SearchSkeleton(char* name, int* argTypes)
{
    list<ProSer>::iterator it = SearchHelper(name, argTypes); 
    return it->second; 
}

////////////////////
//heper functoin 

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

Prosig MakePro(char* name, int* argTypes)
{
    Prosig function(string(name), getTypeLength(argTypes), argTypes); 
    return function; 
}