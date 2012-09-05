//
//  NoPLc.h
//  NoPLc
//
//  Created by Brad Bambara on 4/15/12.
//  Copyright (c) 2012 Small Planet Digital. All rights reserved.
//

#ifndef NoPLc_NoPLc_h
#define NoPLc_NoPLc_h

#include "NoPLValues.h"

typedef struct
{
	NoPL_Instruction* compiledData;
	NoPL_Index dataLength;
	char* errDescriptions;
	void* privateAttributes;
} NoPL_CompileContext;

typedef struct
{
	int optimizeForRuntime;
	int debugSymbols;
} NoPL_CompileOptions;

void compileContextWithFilePath(const char* path, const NoPL_CompileOptions* options, NoPL_CompileContext* context);
void compileContextWithString(const char* scriptString, const NoPL_CompileOptions* options, NoPL_CompileContext* context);

NoPL_CompileContext newNoPL_CompileContext();
void freeNoPL_CompileContext(NoPL_CompileContext* context);

#endif
