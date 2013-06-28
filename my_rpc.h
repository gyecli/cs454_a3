//



struct REGISTER 
{
	int length;
	int type;
	char* server_id;
	char* server_port; 
	char* name; 
	int* argTypes; 
};

struct LOC_REQUEST
{
	int length; 
	int types;
	char* name; 
	int* argTypes; 
};

struct LOC_SUCCESS
{
	int length; 
	int type; 
	char* server_id;
	char* server_port;
};


struct EXECUTE
{
	int length; 
	int type; 
	int* argTypes; 
	void **args; 
};

struct EXECUTE_SUCCESS
{
	int length; 
	int type; 
	char* name; 
	int* argTypes; 
	void** args; 
};

struct FAILURE
{
	int length; 
	int type; 
	int reasonCode; 
};