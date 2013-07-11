#include <list>
#include "rpc.h"
#include "helper.h"
class Prosig;

typedef std::pair<Prosig, skeleton> ProSer; 

class ServerDB
{
private:
	std::list<ProSer>::iterator SearchHelper(char* name, int* argTypes); 

public:
	std::list<ProSer> database; 

	void Add(char* name, int* argTypes, skeleton location); 
	skeleton SearchSkeleton(char* name, int* argTypes);
};
