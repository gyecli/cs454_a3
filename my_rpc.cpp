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

//////////////////////////
// figure out the size (in bytes）of argTypes array, 
//including the "0" at the end; 
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

void error(string reason)
{
    #ifdef _DEBUG
        cout<<reason<<endl; 
    #endif
}

uint32_t char42int(char* input)
{
    uint32_t result;
    result = (uint32_t)input[3];
    result += ((uint32_t)input[2])<<8; 
    result += ((uint32_t)input[1])<<16;
    result += ((uint32_t)input[0])<<24;

    return result;
}

void int2char4(uint32_t n, char* result)
{
    result[0] = (n >> 24) & 0xFF;
    result[1] = (n >> 16) & 0xFF;
    result[2] = (n >> 8) & 0xFF;
    result[3] = n & 0xFF;
}

