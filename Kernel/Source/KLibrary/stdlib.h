#pragma once 
#include "stdint.h"

//void abort(void);
void* (*kmalloc)(size_t);	
int (*kfree)(void*);	

void* (*malloc)(size_t);	
int (*free)(void*);	