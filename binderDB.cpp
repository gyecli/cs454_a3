#include "binderDB.h"
#include "server_loc.h"
#include "prosig.h"
#include "const.h"

using namespace std;

int BinderDB::Register(Prosig function, ServerLoc ser)
{
    list<ProLoc>::iterator it = SearchHelper(function, ser); 
    if(it == database.end())
    {
        //first time for this server to register this function 
        database.push_back(ProLoc(function, ser));
        return REGISTER_SUCCESS; 

    }
    else
    {  
        //server already registed this function before
        return REGISTER_DUPLICATE; 
    }
}

//to find the position in the list
//where we have the specific function & server info
list<ProLoc>::iterator BinderDB::SearchHelper(Prosig function, ServerLoc ser)
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

//search for server given the function prototype 
int BinderDB::SearchServer(Prosig function, ServerLoc *ser)
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