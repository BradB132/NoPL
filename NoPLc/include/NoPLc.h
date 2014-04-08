//
//  NoPLc.h
//  NoPLc
//
//  Created by Brad Bambara on 4/15/12.
//  Copyright (c) 2012. All rights reserved.
//

#ifndef NoPLc_NoPLc_h
#define NoPLc_NoPLc_h

#ifdef __cplusplus
extern "C" {
#endif

#include "NoPLValues.h"

#pragma mark - Ranges

typedef struct
{
	unsigned long startIndex;
	unsigned long endIndex;
} NoPL_TokenRange;

typedef enum
{
	NoPL_TokenRangeType_numericLiterals = 0,
	NoPL_TokenRangeType_stringLiterals = 1,
	NoPL_TokenRangeType_booleanLiterals = 2,
	NoPL_TokenRangeType_pointerLiterals = 3,
	NoPL_TokenRangeType_controlFlowKeywords = 4,
	NoPL_TokenRangeType_typeKeywords = 5,
	NoPL_TokenRangeType_operators = 6,
	NoPL_TokenRangeType_variables = 7,
	NoPL_TokenRangeType_functions = 8,
	NoPL_TokenRangeType_syntax = 9,
	NoPL_TokenRangeType_comments = 10,
	NoPL_TokenRangeType_metadata = 11,
	NoPL_TokenRangeType_precompile = 12,
	NoPL_TokenRangeType_count = 13,
} NoPL_TokenRangeType;

typedef struct
{
	NoPL_TokenRange* ranges[NoPL_TokenRangeType_count];
	unsigned int counts[NoPL_TokenRangeType_count];
} NoPL_TokenRanges;

#pragma mark - NoPL_CompileContext

typedef struct
{
	NoPL_Instruction* compiledData;
	NoPL_Index dataLength;
	char* errDescriptions;
	NoPL_TokenRanges* tokenRanges;
	void* privateAttributes;
} NoPL_CompileContext;

NoPL_CompileContext newNoPL_CompileContext();
void freeNoPL_CompileContext(NoPL_CompileContext* context);

#pragma mark - NoPL_CompileOptions

typedef struct
{
	int optimizeForRuntime;
	int debugSymbols;
	int createTokenRanges;
} NoPL_CompileOptions;

#define NoPL_CompileOptions() {1,0,0}

#pragma mark - Compiling APIs

void compileContextWithFilePath(const char* path, const NoPL_CompileOptions* options, NoPL_CompileContext* context);
void compileContextWithString(const char* scriptString, const NoPL_CompileOptions* options, NoPL_CompileContext* context, const char* apiPath);

#ifdef __cplusplus
}
#endif

#endif
