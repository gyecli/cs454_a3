#include "binderDB.h"
#include "server_loc.h"
#include "prosig.h"
#include "const.h"
using namespace std; 

Tuple::Tuple(){}
Tuple::Tuple(Prosig first, ServerLoc second, int third):first(first), second(second), third(third){}


//TODO: is there any other type of errors for register?
int BinderDB::Register(Prosig function, ServerLoc ser, int sockfd)
{
    list<Tuple>::iterator it = SearchHelper(function, ser); 
    if(it == database.end())
    {
        //first time for this server to register this function 
        database.push_back(Tuple(function, ser, sockfd));
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
list<Tuple>::iterator BinderDB::SearchHelper(Prosig function, ServerLoc ser)
{
    for(list<Tuple>::iterator it=database.begin(); it!=database.end(); ++it)
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
    for(list<Tuple>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        if(function == it->first)
        {
            *ser = it->second;
            //move it to the back of the list 
            Tuple selected = *it; 
            database.erase(it);
            database.push_back(selected);
            return LOC_SUCCESS; 
        }
        // TODO: move the ServerLoc to the end of the list
        // for round-rabin behavior
    }
    return LOC_FAILURE; 
}

void BinderDB::Cleanup(int sockfd)
{
    for(list<Tuple>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        if(sockfd == it->third)
        {
            database.erase(it);
        }
    }
}