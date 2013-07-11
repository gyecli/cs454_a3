#include "prosig.h"
#include "const.h"
#include <string>

Prosig::Prosig(std::string name, int argNum, int* argTypes):name(name),argNum(argNum), argTypes(argTypes)
{
}

Prosig::~Prosig()
{
    delete [] argTypes;
}

bool Prosig::operator==(const Prosig &other) const 
{
    if(this->name != other.name)    // TO_DO: string comparison not proper?
        return false;
    if(this->argNum != other.argNum)
        return false;

    for(int i=0; i<this->argNum; ++i)
    {
        if(this->argTypes[i] != other.argTypes[i])
            return false; 
        if(this->name != other.name)
            return false;
        if(this->argNum != other.argNum)
            return false;

        for(int i=0; i<this->argNum; ++i)
        {
            if( (this->argTypes[i]>>16) != (other.argTypes[i]>>16))
                return false;
            else
            {
                //TODO: not sure if this is correct
                int len1, len2; 
                len1 = (this->argTypes[i])&array_size_mask; 
                len2 = (other.argTypes[i])&array_size_mask; 
                
                if( (len1 == 1 && len2 != 1) ||
                    (len2 != 1 && len2 ==1 ))
                    return false; 
            }
        }
        return true; 
    }
    return true; 
}
