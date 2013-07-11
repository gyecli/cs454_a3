
//type
#define REGISTER 1
#define LOC_REQUEST 2 
#define EXECUTE 4  

//success 
#define REGISTER_SUCCESS 0 
#define LOC_SUCCESS 0 
#define RPCCALL_SUCCESS 0

//failure 
#define LOC_FAILURE -1 
#define REGISTER_FAILURE -2
#define REGISTER_DUPLICATE -3
#define EXECUTE_FAILURE -4
#define RPCCALL_FAILURE -5

//size
#define SIZE_IDENTIFIER 128
#define SIZE_PORTNO 4
#define SIZE_NAME 100

//masks for extracting parameters 
#define IO_mask (3 << 30)
#define Input_mask (1 << 31)
#define Output_mask (1 << 30)
#define Type_mask (255 << 16)
#define array_size_mask ((1<<17)-1)	