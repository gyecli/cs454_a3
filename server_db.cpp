#include "rpc.h"
#include "my_rpc.h"
#include "binder.h"

#include <list>


typedef std::pair<Prosig, skeleton> ProSer; 

class ServerDB
{
public:
	std::list<ProSer> database; 

	void Add(Prosig function); 
	skeleton Search(Prosig function); 
};

void ServerDB::Add(Prosig function)
{

}

skeleton ServerDB::Search(Prosig function)
{
	
}

