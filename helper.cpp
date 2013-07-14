#include "helper.h"
#include <cstdlib>
#include <cstring>

#define LEN_ARG(t) ((t) & 0xFFFF)

using namespace std; 

Prosig* MakePro(char* name, int* argTypes)
{
    Prosig *function = new Prosig(string(name), getArgNum(argTypes), argTypes); 
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
    int total_len = 0; // # of bytes
    while(*it != 0) { // last element of argTypes is always ZERO
        // Type_mask = (255 << 16)
        unsigned int current_type = ((*it) & Type_mask) >> 16; 
        unsigned int num = ((*it) & array_size_mask); // # of current arg of current_type

        if (num == 0) {
            num = 1; 
        }
        switch(current_type) {
            case ARG_CHAR:
                // type: char
                total_len += sizeof(char) * num; 
                break; 
            case ARG_SHORT:
                // type: short
                total_len += sizeof(short) * num; 
                break;
            case ARG_INT:
                // type: int
                total_len += sizeof(int) * num; 
                break;
            case ARG_LONG:
                // type: long
                total_len += sizeof(long) * num; 
                break;
            case ARG_DOUBLE:
                // type: double
                total_len += sizeof(double) * num; 
                break; 
            case ARG_FLOAT:
                // type: float
                total_len += sizeof(float) * num; 
                break;
            default:
                cerr<<"ERROR: Unknown Type!"<<endl;
                break;
        }
        it++;
    }
    return total_len; 
}

// char* pack(i 

static int UNIT_LEN(int t) {
  t = t >> 16;
    t &= 0xFF;
 
    switch (t) {
    case ARG_CHAR:
        return sizeof(char);
    case ARG_SHORT:
        return sizeof(short);
    case ARG_INT:
        return sizeof(int);
    case ARG_LONG:
        return sizeof(long);
    case ARG_FLOAT:
        return sizeof(float);
    case ARG_DOUBLE:
        return sizeof(double);
    default:
        cerr << "should never reach here" << endl;
        return 0;
    }
} 

void** unpickle(int *arg_types, char* mem_block) {
    void **pp_res = NULL;
    char *p_read = mem_block;
    int *p_type = NULL;
 
    int i, len_arg, unit_len;
    int num_args = 0;
 
    if (*arg_types == 0 || mem_block == NULL) {
        return NULL;
    }
 
    for (p_type = arg_types; *p_type != 0; p_type++) {
        num_args++;
    }
    pp_res = new void *[num_args];
 
    for (i = 0; i < num_args; i++) {
        len_arg = LEN_ARG(arg_types[i]);
        unit_len = UNIT_LEN(arg_types[i]);
        if (len_arg == 0) {
            pp_res[i] = (void *)(new char[unit_len]);
            memcpy(pp_res[i], p_read, unit_len);
            p_read += unit_len;
        } else if (len_arg > 0) {
            pp_res[i] = (void *)(new char[unit_len * len_arg]);
            memcpy(pp_res[i], p_read, unit_len * len_arg);
            p_read += unit_len * len_arg;
        } else {
            cerr << "should never reach here" << endl;
        }
    }

    return pp_res;
}
 
char* pickle(int* arg_types, void** arg_array) {
    char *p_res = NULL, *p_write = NULL;
    int *p_type = NULL;
    int i, len_arg, unit_len;
    int len_res = 0, num_args = 0;
 
    if (*arg_types == 0 || arg_array == NULL) {
        return NULL;
    }
 
    for (p_type = arg_types; *p_type != 0; p_type++) {
        len_arg = LEN_ARG(*p_type);
        unit_len = UNIT_LEN(*p_type);
        if (len_arg == 0) {
            len_res += unit_len;
        } else if (len_arg > 0) {
            len_res += len_arg * unit_len;
        }
        num_args++;
    }
    p_res = new char[len_res + 10];
    p_write = p_res;
 
    for (i = 0; i < num_args; i++) {
        len_arg = LEN_ARG(arg_types[i]);
        unit_len = UNIT_LEN(arg_types[i]);
        if (len_arg == 0) {
            memcpy(p_write, arg_array[i], unit_len);
            p_write += unit_len;
        } else if (len_arg > 0) {
            memcpy(p_write, arg_array[i], unit_len * len_arg);
            p_write += unit_len * len_arg;
        } else {
            cerr << "should never reach here" << endl;
        }
    }

    return p_res;
}
