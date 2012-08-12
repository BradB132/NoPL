//
//  NoPLStandardFunctions.h
//  NoPL_Runtime
//
//  Created by Brad Bambara on 8/11/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#ifndef NoPL_Runtime_NoPLStandardFunctions_h
#define NoPL_Runtime_NoPLStandardFunctions_h

#include "NoPLRuntime.h"

//function that handles all standard function calls
NoPL_FunctionValue nopl_standardFunctions(void* calledOnObject, char* functionName, NoPL_FunctionValue* argv, unsigned int argc);

//pointers to objects that could be used to extend the existing classes
void* nopl_standardMath();
void* nopl_standardString();

#endif
