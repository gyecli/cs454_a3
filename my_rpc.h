#include <list>
#include <utility>
#include <string>
#include "rpc.h"
//TODO re-define all these ints 

//type constants 
#define REGISTER 1

#define LOC_REQUEST 2
#define LOC_SUCCESS 0
#define LOC_FAILURE -1

#define EXECUTE 4
#define EXECUTE_SUCCESS 0
#define EXECUTE_FAILURE -2

#define RPCCALL_SUCCESS 0
#define RPCCALL_FAILURE -3

//masks for extracting parameters 
#define IO_mask (3 << 30)
#define Input_mask (1 << 31)
#define Output_mask (1 << 30)
#define Type_mask (255 << 16)
#define array_size_mask ((1<<17)-1)		

//define reasonCode here	

//added by Yiyao 
#define REGISTER_SUCCESS 0 

#define SIZE_IDENTIFIER 128
#define SIZE_PORTNO 4
#define SIZE_NAME 100

//classes 
class Prosig{
public:
    // TODO: private
	//might not be a good idea to make all fields public
    //will make it like this for now, for the sake of simplicity
    std::string name;
    int argNum; 
    int* argTypes;

    Prosig(std::string name, int argNum, int* argTypes);
    ~Prosig();
    bool operator==(const Prosig &other) const;
};

typedef std::pair<Prosig, skeleton> ProSer; 

class ServerDB
{
public:
	std::list<ProSer> database; 

	void Add(Prosig function, skeleton location); 
	std::list<ProSer>::iterator Search(Prosig function); 
};

//helper functions
int getTypeLength(int* argTypes);


