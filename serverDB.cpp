#include "serverDB.h"

using namespace std; 

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

bool ServerDB::SearchSkeleton(char* name, int* argTypes, skeleton *skel_loc)
{
    list<ProSer>::iterator it = SearchHelper(name, argTypes); 
    if(it == database.end())
    {
        return false;
    }
    else
    {
        *skel_loc = it->second; 
        return false; 
    }
}
