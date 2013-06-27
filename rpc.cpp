// Please put the rcp functions here

int rpcInit();
int rpcCall(char* name, int* argTypes, void** args);
int rpcCacheCall(char* name, int* argTypes, void** args);
int rpcRegister(char* name, int* argTypes, skeleton f);
int rpcExecute();
int rpcTerminate();
