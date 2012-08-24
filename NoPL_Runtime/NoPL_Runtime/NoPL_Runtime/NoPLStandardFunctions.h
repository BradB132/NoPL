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
NoPL_FunctionValue nopl_standardFunctions(const void* calledOnObject, const char* functionName, const NoPL_FunctionValue* argv, unsigned int argc);

#endif
