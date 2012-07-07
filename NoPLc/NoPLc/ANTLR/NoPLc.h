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
#include "NoPLLexer.h"//TODO: move this back to .c file
#include "NoPLParser.h"

typedef struct
{
	NoPL_Instruction* compiledData;
	NoPL_Index dataLength;
	NoPL_Index arrayLength;
	pANTLR3_STACK objectStack;
	pANTLR3_STACK numberStack;
	pANTLR3_STACK booleanStack;
	pANTLR3_STACK stringStack;
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
