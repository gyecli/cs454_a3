
//type
#define REGISTER 1
#define LOC_REQUEST 2 
#define EXECUTE 4  
#define TERMINATE 5		// added by Tim

//success 
#define REGISTER_SUCCESS 0 
#define LOC_SUCCESS 0 
#define RPCCALL_SUCCESS 0
#define EXECUTE_SUCCESS 0
#define TERMINATE_SUCCESS 0		// may not be necessary

//failure 
#define LOC_FAILURE -1 
#define REGISTER_FAILURE -2
#define EXECUTE_FAILURE -4
#define RPCCALL_FAILURE -5
#define TERMINATE_FAILURE -6	// added by Tim
#define UNKNOW_REQUEST -7
#define NO_RPC -8 // no such RPC is registered 
#define NOT_REGISTER -9 // server didn't call rpcRegister, but call rpcExecute
#define CANT_CONNECT_BINDER -10 

//warning
#define REGISTER_DUPLICATE 100
#define SERVER_UNAVAILABLE 101
//size
#define SIZE_IDENTIFIER 128
#define SIZE_PORTNO 2
#define SIZE_NAME 100
#define SIZE_SOCK (sizeof(int))

//masks for extracting parameters 
#define IO_mask (3 << 30)
#define Input_mask (1 << 31)
#define Output_mask (1 << 30)
#define Type_mask (255 << 16)
#define array_size_mask ((1<<16)-1)	