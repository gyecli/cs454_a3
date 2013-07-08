

#define REGISTER 1
#define LOC_REQUEST 2
#define LOC_SUCCESS 3
#define EXECUTE 4
#define EXECUTE_SUCCESS 5
#define FAILURE 0


#define IO_mask (3 << 30)
#define Input_mask (1 << 31)
#define Output_mask (1 << 30)
#define Type_mask (255 << 16)
#define Length_mask 255			// OR 65535 (NOT SURE, becasue it's last 2 bytes)

//define reasonCode here	