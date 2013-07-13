#include "serverDB.h"
#include "helper.h"     // by tim for testing

using namespace std; 

void ServerDB::Add(char* name, int* argTypes, skeleton location)
{
    cout<<"add function"<<endl;
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

    cout << "In serverDB.cpp: name = " << (string) name << endl;
    cout << "db size:" << database.size() << endl; 
    for(list<ProSer>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        cout << "sth" << endl;
        cout << "fn: " << it->first.name << endl;
        if(function == it->first)
        {
            return it; 
        }
    }
    return database.end(); 
}


bool ServerDB::SearchSkeleton(char* name, int* argTypes, skeleton *skel_result)
{
    list<ProSer>::iterator it = SearchHelper(name, argTypes); 
    if(it == database.end())
    {
        return false;
    }
    else
    {      
        *skel_result = it->second;  
        cout << "(In serverDB.cpp) OK, here is the skeleton" << endl; 
        return true; 
    }
}
