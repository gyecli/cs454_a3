#include "binder.h"
#include "my_rpc.h"

// added by Tim
#include "rpc.h"

#define array_size_mask ((1<<17)-1)     // TO_DO: This is already in "my_rpc.h" (commented by Tim)

using namespace std;  

Prosig::Prosig(string name, int argNum, int* argTypes):name(name),argNum(argNum), argTypes(argTypes)
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

Server::Server()
{
        //TODO: not sure if i should initialize them
        //identifier = new char[SIZE_NAME]; 
        //portno = new char[portno]; 
}

Server::Server(char* identifier, char* portno): identifier(identifier),portno(portno)
{
}

bool Server::operator == (const Server &other) const
{
    int n; 
    //compare the memory pointed 
    n = memcmp(this->portno, other.portno, SIZE_PORTNO);
    if(n != 0)
    return false; 
    n = memcmp(this->identifier, other.identifier, SIZE_IDENTIFIER);
    if(n != 0)
        return false;
    return true; 
    }

Server::~Server()
{
    delete [] identifier;
    delete [] portno; 
} 

//binder database

int BinderDB::Register(Prosig function, Server ser)
{
    list<ProLoc>::iterator it = SearchAll(function, ser); 
    if(it == database.end())
    {
        //first time for this server to register this function 
        database.push_back(ProLoc(function, ser));
    }
    //else
        //server already registed this function before
        //no need to do else 

    return REGISTER_SUCCESS; 
}

list<ProLoc>::iterator BinderDB::SearchAll(Prosig function, Server ser)
{
    for(list<ProLoc>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        if(function == it->first && ser == it->second)
        {
            return it; 
        }
    }
    return database.end(); 
}

bool BinderDB::Search(Prosig function, Server *ser)
{
    for(list<ProLoc>::iterator it=database.begin(); it!=database.end(); ++it)
    {
        if(function == it->first)
        {
            *ser = it->second;
            return true; 
        }
    }
    return false; 
}

//TODO: repetitive with Tim's code
//should put it in some kind of library file 
int getTypeLength(int* argTypes) {
    int size = 0;
    int* it = argTypes;
    while (*it != 0) {
        size += 4;
        it = it+1;
    }
    return (size +4);
}

void package(int type, char* msg)
{

}

int BinderRegister()
{
    return 0; 
}

int rpcRegister(char* name, int *argTypes, skeleton f)
{
    return 0; 
}

/*
int main()
{
    BinderDB db;
    char size_buff[4];
    char type_buff[4];    
    char* received; 
    uint32_t type, size;

    //wrap

    //TODO: all memcpy will be replace by socket read
    //unwrap 
    memcpy(size_buff, received, 4);
    size = atoi(size_buff);
    memcpy(type_buff, received+4, 4);
    type = atoi(type_buff);
    

    switch(type)
    {
        case REGISTER:
        {
            char server_id[SIZE_IDENTIFIER]; 
            char portno[SIZE_PORTNO]; 
            char name[SIZE_NAME];
            memcpy(server_id, received + 8, SIZE_IDENTIFIER); 
            memcpy(portno, received + 8 + SIZE_IDENTIFIER, SIZE_PORTNO); 
            memcpy(name, received + 8 + SIZE_IDENTIFIER + SIZE_PORTNO, SIZE_NAME); 

            int used_size = SIZE_IDENTIFIER + SIZE_PORTNO + SIZE_NAME; 
            char* buff = new char[size - used_size]; 
            memcpy(buff, received + 8 + used_size, size - used_size);
            int* argTypes = (int*)buff; 

            Prosig pro = Prosig(string(name), getTypeLength(argTypes), argTypes);
            Server ser = Server(server_id, portno);
            db.Register(pro, ser); 
            break;
        }
        case LOC_REQUEST:
        {
            char name[SIZE_NAME];
            char* buff = new char[size - SIZE_NAME]; 
            int result; 

            memcpy(name, received + 8, SIZE_NAME);
            memcpy(buff, received + 8 + SIZE_NAME, size - SIZE_NAME); 
            int* argTypes = (int*)buff; 
            Server *ser = new Server(); 
            result = db.Search(Prosig(string(name), getTypeLength(argTypes), argTypes), ser);
            if(result == true)
            {
                //TODO: not right on passing variables
                //ser won't be copied correctly 
                package(LOC_SUCCESS, (char*)&ser);
            }
            else
            {
                package(LOC_FAILURE, NULL); 
            }
        }

    }
}
*/

