#include "helper.h"

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
void error(string reason)
{
    //#ifdef _DEBUG
        cout<<reason<<endl; 
    //#endif
}


Prosig MakePro(char* name, int* argTypes)
{
    Prosig function(string(name), getArgNum(argTypes), argTypes); 
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
        it = it+1;
    }
    return num;
}