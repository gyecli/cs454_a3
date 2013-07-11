#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>
#include <string>
#include <iostream>
#include "prosig.h"


uint32_t char42int(char* input);
void int2char4(uint32_t n, char* result);
int getTypeLength(int* argTypes);
void error(std::string reason);
Prosig MakePro(char* name, int* argTypes);

#endif

