
//DODO how should we define all these ints 
#define REGISTER 1
#define LOC_REQUEST 2
#define LOC_SUCCESS 3
#define EXECUTE 4
#define EXECUTE_SUCCESS 5
#define FAILURE 0

#define RPCCALL_SUCCESS 0
#define RPCCALL_FAILURE -1
#define LOC_FAILURE 6

#define IO_mask (3 << 30)
#define Input_mask (1 << 31)
#define Output_mask (1 << 30)
#define Type_mask (255 << 16)
#define Length_mask 65535			// OR 65535 (NOT SURE, becasue it's last 2 bytes)

//define reasonCode here	

//added by Yiyao 
#define REGISTER_SUCCESS 0; 
