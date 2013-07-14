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
break;
}
it++;
}
return total_len; 
}

char* pack(int* argTypes, void** args) {

int argsLength = getArgsLength(argTypes);
char *packedBuff = new char[argsLength];
memset(packedBuff, 0, (argsLength*sizeof(char)));
char* it = packedBuff;

int unit_len;
for(int i = 0; argTypes[i] != 0; i++)
{
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
if (num == 0) // scalar
{
memcpy(it, args[i], unit_len);
it += unit_len;
}
else
{
memcpy(it, args[i], unit_len * num); // TO_DO: is args[i] is a consecutive memeory block
it += (unit_len*num);
}
}
cout << "pack end" << endl;
return packedBuff;
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
