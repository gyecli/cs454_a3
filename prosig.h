#ifndef PROSIG_H
#define PROSIG_H

#include <string>

//Procedure Signiture 
//stores everything we need to compare 2 signitures
//the argNum field is just for easy comparison

class Prosig{
public:
    // TODO: private
	//might not be a good idea to make all fields public
    //will make it like this for now, for the sake of simplicity
    std::string name;
    int argNum; 
    int* argTypes;

    Prosig();
    Prosig(const Prosig &other);
    Prosig(std::string name, int argNum, int* argTypes);
    ~Prosig();
    bool operator==(const Prosig &other) const;
    const Prosig& operator=( const Prosig& rhs );

};

#endif