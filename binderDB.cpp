#include <iostream>
#include "binderDB.h"
#include "server_loc.h"


#include "prosig.h"
#include "const.h"


using namespace std; 

Tuple::Tuple()
{
    this->third = list<Prosig>(); 
}
Tuple::Tuple(int first, ServerLoc second, Prosig function):first(first), second(second)
{
    cout << " tupple constructor push back" << endl; 
    sleep(5);
    this->third = list<Prosig>(); 
    this->third.push_back(function);
}

//TODO: is there any other type of errors for register?
int BinderDB::Register(Prosig function, ServerLoc ser, int sockfd)
{
    cout << "inside register function" << endl; 
    for(list<Tuple>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        //this server has already registered at least one function 
        if(sockfd == it->first && ser == it->second)
        {
            for(list<Prosig>::iterator it2 = it->third.begin(); it2 != it->third.end(); ++it2)
            {
                if( function == *it2)
                {
                    return REGISTER_DUPLICATE;
                }
            }
            //register the new function
            it->third.push_back(function);
            return REGISTER_SUCCESS;
        }
    }
    //the first time to see this server
    database.push_back(Tuple(sockfd, ser, function));
    return REGISTER_SUCCESS;
}

//search for server given the function prototype 
int BinderDB::SearchServer(Prosig function, ServerLoc *ser)
{
    for(list<Tuple>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        for(list<Prosig>::iterator it2 = it->third.begin(); it2 != it->third.end(); ++it2)
        {
            if(function == *it2)
            {
                *ser = it->second;
                
                //move it to the back of the list 
                Tuple selected = *it; 
                database.erase(it);
                database.push_back(selected);
                return LOC_SUCCESS; 
            }
        }
    }
    return LOC_FAILURE; 
}

//when a connection is closed, 
//clean up the database that with that sock#
void BinderDB::Cleanup(int sockfd)
{
    for(list<Tuple>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        if(sockfd == it->first)
        {
            database.erase(it);
            return; 
        }
    }
}
