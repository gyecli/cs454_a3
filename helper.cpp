#include "helper.h"
#include <cstdlib>
#include <cstring>

#define LEN_ARG(t) ((t) & 0xFFFF)

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
                cout<<"what? type is wrong!!!"<<endl;
                break;
        }
        it++;
    }
    return total_len; 
}

char* pack(int* argTypes, void** args) {

    int argsLength = getArgsLength(argTypes);
    char *packedBuff = new char[argsLength+1];
    memset(packedBuff, 0, argsLength+1);
    char* it = packedBuff;

    cout << "argsLength " << argsLength << endl; 

    int unit_len = 0;
    for(int i = 0; argTypes[i] != 0; i++)
    {
        cout<<"i: "<<i << endl; 
        unsigned int current_type = ((argTypes[i]) & Type_mask) >> 16;
        unsigned int num = ((argTypes[i]) & array_size_mask); // # of current arg of current_type

        switch(current_type)
        {
            case ARG_CHAR:
                // type: char
                unit_len = sizeof(char);
                break;
            case ARG_SHORT:
                // type: short
                unit_len = sizeof(short);
                break;
            case ARG_INT:
                // type: int
                unit_len = sizeof(int);
                break;
            case ARG_LONG:
                // type: long
                unit_len = sizeof(long);
                break;
            case ARG_DOUBLE:
                // type: double
                unit_len = sizeof(double);
                break;
            case ARG_FLOAT:
                // type: float
                unit_len = sizeof(float);
                break;
            default:
                break;
        }

        cout<<"type: " <<current_type << " num:" << num <<endl; 

        if (num == 0) // scalar
        {
            memcpy(it, args[i], unit_len);
            it += unit_len;
        }
        else
        {
            cout << "char:" << endl; 

            cout <<endl<< "unit length" << unit_len;
            cout << "before copy " << endl ;
             
            sleep(5);
            memcpy(it, args[i], unit_len * num); // TO_DO: is args[i] is a consecutive memeory block
            cout << "after copy" << endl; 
            sleep(1);
            it += (unit_len*num);
        }
    }

    cout<<"******************"<<endl<<"it:" << (it-packedBuff) << " calculate" << argsLength << endl; 
    cout << "pack end" << endl;
    return packedBuff;
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
            cerr << "should never come here" << endl;
        }
    }
 
    return pp_res;
}


void** unpack(int* argTypes, char* memBlock)
{
    cout << "begin unpack" << endl;
    void** args;
    int argNum = getArgNum(argTypes);
    char* it = memBlock;
    int unit_len;

    int total_size = 0;
    args = new void*[argNum];

    for (int i = 0; argTypes[i] != 0; i++)
    {
        unsigned int current_type = ((argTypes[i]) & Type_mask) >> 16;
        unsigned int num = ((argTypes[i]) & array_size_mask); // # of current arg of current_type
        switch(current_type)
        {
            case ARG_CHAR:
                // type: char
                unit_len = sizeof(char);
                if (num == 0) //scalar
                {
                    char* temp = new char[1];
                    memcpy(temp, it, unit_len);
                    args[i] = (void *)(temp);

                    it += unit_len;
                }
                else
                {
                    char* temp = new char[unit_len*num];
                    memcpy(temp, it, unit_len*num);
                    args[i] = (void *) temp;

                    it += (unit_len*num);
                }
                break;
            case ARG_SHORT:
                // type: short
                unit_len = sizeof(short);
                if (num == 0) //scalar
                {
                    short* temp = new short[1];
                    memcpy(temp, it, unit_len);
                    args[i] = (void *)(temp);

                    it += unit_len;
                }
                else
                {
                    short* temp = new short[unit_len*num];
                    memcpy(temp, it, unit_len*num);
                    args[i] = (void *) temp;

                    it += (unit_len*num);
                }
                break;
            case ARG_INT:
                // type: int
                unit_len = sizeof(int);
                if (num == 0) //scalar
                {
                    int* temp = new int[1];
                    memcpy(temp, it, unit_len);
                    args[i] = (void *)(temp);

                    it += unit_len;
                }
                else
                {
                    int* temp = new int[unit_len*num];
                    memcpy(temp, it, unit_len*num);
                    args[i] = (void *) temp;

                    it += (unit_len*num);
                }
                break;
            case ARG_LONG:
                // type: long
                unit_len = sizeof(long);
                if (num == 0) //scalar
                {
                    long* temp = new long[1];
                    memcpy(temp, it, unit_len);
                    args[i] = (void *)(temp);

                    it += unit_len;
                }
                else
                {
                    long* temp = new long[unit_len*num];
                    memcpy(temp, it, unit_len*num);
                    args[i] = (void *) temp;

                    it += (unit_len*num);
                }
                break;
            case ARG_DOUBLE:
                unit_len = sizeof(double);
                // type: double
                if (num == 0) //scalar
                {
                    double* temp = new double[1];
                    memcpy(temp, it, unit_len);
                    args[i] = (void *)(temp);

                    it += unit_len;
                }
                else
                {
                    double* temp = new double[unit_len*num];
                    memcpy(temp, it, unit_len*num);
                    args[i] = (void *) temp;

                    it += (unit_len*num);
                }
                break;
            case ARG_FLOAT:
                unit_len = sizeof(float);
                // type: float
                if (num == 0) //scalar
                {
                    float* temp = new float[1];
                    memcpy(temp, it, unit_len);
                    args[i] = (void *)(temp);

                    it += unit_len;
                }
                else
                {
                    float* temp = new float[unit_len*num];
                    memcpy(temp, it, unit_len*num);
                    args[i] = (void *) temp;

                    it += (unit_len*num);
                }
                break;
            default:
                break;
        }
        if (num == 0) {
            total_size = total_size + unit_len;
        } else {
            total_size += unit_len*num;
        }
    }
    return args;
}

 
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
