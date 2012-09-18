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

#pragma mark -
#pragma mark NoPL_CompileContext

typedef struct
{
	NoPL_Instruction* compiledData;
	NoPL_Index dataLength;
	char* errDescriptions;
	void* privateAttributes;
} NoPL_CompileContext;

NoPL_CompileContext newNoPL_CompileContext();
void freeNoPL_CompileContext(NoPL_CompileContext* context);


#pragma mark -
#pragma mark NoPL_CompileOptions

typedef struct
{
	int optimizeForRuntime;
	int debugSymbols;
} NoPL_CompileOptions;

#define NoPL_CompileOptions() {1,0}


#pragma mark -
#pragma mark Compiling APIs

void compileContextWithFilePath(const char* path, const NoPL_CompileOptions* options, NoPL_CompileContext* context);
void compileContextWithString(const char* scriptString, const NoPL_CompileOptions* options, NoPL_CompileContext* context);

#endif
