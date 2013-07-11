#include "binderDB.h"
#include "server_loc.h"
#include "prosig.h"
#include "const.h"
#include <iostream>

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

using namespace std; 

//to find the position in the list
//where we have the specific function & server info
list<ProLoc>::iterator BinderDB::SearchHelper(Prosig function, ServerLoc ser)
{
    int i=0; 
    cout<<"TESTing: in binder.cpp, Binder size:"<<database.size()<<endl; 
    for(list<ProLoc>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        cout<<i<<endl;
        ++i;

        cout<<"function:"<<function.name<<" "<<function.argNum<<endl;
        cout<<"current:"<<it->first.name<<" "<<it->first.argNum<<endl;

        if(function == it->first && ser == it->second)
        {
            std::cout<<"found"<<std::endl; 
            return it; 
        }
    }
    std::cout<<"not found"<<std::endl<<endl; 
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