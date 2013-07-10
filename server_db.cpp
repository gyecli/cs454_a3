#include "rpc.h"
#include "my_rpc.h"
#include "binder.h"

#include <list>

using namespace std; 

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
	skeleton result = NULL; 
	for(list<ProSer>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        if(function == it->first)
        {
        	return it; 
        }
    }
    return database.end(); 

}

