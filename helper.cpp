#include "helper.h"
#include "Prosig.h"

using namespace std; 

/*
uint32_t char42int(char* input)
{
    uint32_t result;
    result = (uint32_t)input[3];
    result += ((uint32_t)input[2])<<8; 
    result += ((uint32_t)input[1])<<16;
    result += ((uint32_t)input[0])<<24;

    return result;
}

void int2char4(uint32_t n, char* result)
{
    result[0] = (n >> 24) & 0xFF;
    result[1] = (n >> 16) & 0xFF;
    result[2] = (n >> 8) & 0xFF;
    result[3] = n & 0xFF;
}
*/


Prosig* MakePro(char* name, int* argTypes)
{
    Prosig *function = new Prosig(string(name), getArgNum(argTypes), argTypes); 
    //cout << "in make pro" << function->name << endl; 
    //sleep(5);
    return function; 
}

//returns how many bytes argType should have
int getTypeLength(int* argTypes) 
{
    int size = 0;
    int* it = argTypes;
    while (*it != 0) {
        size += 4;
        it = it+1;
    }
    return (size +4);
}

int getArgNum(int* argTypes)
{
    int num = 0;
    int* it = argTypes;
    while (*it != 0) {
        ++num;
        ++it;
    }
    return num;
}


// Added by Tim (moved from rpc.cpp)
// get the total length of args in bytes
int getArgsLength(int* argTypes) {
    int* it = argTypes; 
    int total_len = 0;         // # of bytes
    while(*it != 0) {          // last element of argTypes is always ZERO
        // Type_mask = (255 << 16)
        unsigned int current_type = ((*it) & Type_mask) >> 16; 

        unsigned int num = ((*it) & array_size_mask);  // # of current arg of current_type
        if (num == 0) {
            num = 1; 
        }

        switch(current_type) {
            case ARG_CHAR:
                // type: char
                total_len += 1 * num; 
                break; 
            case ARG_SHORT:
                // type: short
                total_len += 2 * num; 
                break;
            case ARG_INT:
                // type: int
                total_len +=  4 * num; 
                break;
            case ARG_LONG:
                // type: long
                total_len += 4 * num; 
                break;
            case ARG_DOUBLE:
                // type: double
                total_len += 8 * num; 
                break; 
            case ARG_FLOAT:
                // type: float
                total_len += 4 * num; 
                break;
            default:
                break;
        }
        it++;
    }
    return total_len; 
}