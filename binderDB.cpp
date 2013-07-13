#include "binderDB.h"
#include "server_loc.h"
#include "prosig.h"
#include "const.h"
#include <iostream>

using namespace std;

//TODO: is there any other type of errors for register?
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
    int i = 0; 
    for(list<ProLoc>::iterator it = database.begin(); it != database.end(); ++it)
    {
        cout<<i<<endl;
        ++i;
        cout<<"function:"<<function.name<<" "<<function.argNum<<endl;
        cout<<"current:"<<it->first.name<<" "<<it->first.argNum<<endl;

        if(function == it->first)
        {
            cout<<"first same"<<endl; 
        }
        if(!(ser == it->second))
        {
            cout<<"from server:"<<endl; 
            cout<<ser.identifier<<endl;
            unsigned short *p = (unsigned short*)ser.portno; 
            cout<<*p<<endl; 

            cout<<"in database:"<<endl;
            cout<<it->second.identifier<<endl;
            p = (unsigned short*)it->second.portno; 
            cout<<*p<<endl;
        }

        if(function == it->first && ser == it->second)
        {
            cout << "found" << endl; 
            return it; 
        }
    }
    cout << "not found" << endl << endl; 
    return database.end(); 
}

//search for server given the function prototype 
int BinderDB::SearchServer(Prosig function, ServerLoc *ser)
{
    for(list<ProLoc>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        if(function == it->first)
        {
            cout<<"function loc foound"<<endl;
            *ser = it->second;
            return LOC_SUCCESS; 
        }
        // TODO: move the ServerLoc to the end of the list
        // for round-rabin behavior
    }
    cout<<"not foound function loc"<<endl;

    return LOC_FAILURE; 
}