#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>
#include <string>
#include <iostream>
#include "prosig.h"
#include "const.h"	// added by Tim
#include "rpc.h"	// added by Tim


//uint32_t char42int(char* input);
//void int2char4(uint32_t n, char* result);
int getTypeLength(int* argTypes);
int getArgNum(int* argTypes);
int getArgsLength(int* argTypes);		// added by Tim
Prosig* MakePro(char* name, int* argTypes);

#endif

