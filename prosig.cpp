#include <string>
#include <iostream> 

#include "prosig.h"
#include "const.h"
#include "helper.h"

using namespace std;  

Prosig::Prosig()
{
    
}

/*
Prosig::Prosig(const Prosig &other)
{
    this->name = other.name; 
    this->argNum = other.argNum;
    int size = getTypeLength(other.argTypes); 
    char* temp = new char[size]; 
    memcpy(temp, other.argTypes, size);
    this->argTypes = (int*) temp;
}

const Prosig& Prosig::operator=( const Prosig& other )
{
    if(this == &other)
    {
        return *this; 
    }

    this->name = other.name; 
    this->argNum = other.argNum;
    int size = getTypeLength(other.argTypes); 
    char* temp = new char[size]; 
    memcpy(temp, other.argTypes, size);
    this->argTypes = (int*) temp;

    return *this; 
}
*/

Prosig::Prosig(std::string name, int argNum, int* argTypes):name(name),argNum(argNum), argTypes(argTypes)
{
/*
    int size = getTypeLength(argTypes);
    char* buff  = new char[size];
    memcpy(buff, argTypes, size); 
    argTypes = (int*) buff; 
    */
}

Prosig::~Prosig()
{
    //TODO: I can't delete it, says it's not allocated, I don't know why
    //std::cout<<"delete"<<std::endl;
    //delete [] argTypes;
}

bool Prosig::operator==(const Prosig &other) const 
{
    if(this->name != other.name)    // TO_DO: string comparison not proper?
        return false;
    if(this->argNum != other.argNum)
        return false;
    
    for(int i=0; i<this->argNum; ++i)
    {
        int type1 = (this->argTypes[i])>>16; 
        int type2= (other.argTypes[i])>>16;

        if(type1 != type2)
            return false;
        else
        {
            int len1, len2; 

            len1 = (this->argTypes[i])&array_size_mask; 
            len2 = (other.argTypes[i])&array_size_mask; 
                           
            if( (len1 == 0 && len2 != 0) ||
                (len2 != 0 && len2 ==0 ))
            {
                return false; 
            }
        }
    }
    return true; 
}
