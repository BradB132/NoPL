//
//  NoPLRuntime.h
//  NoPL_Runtime
//
//  Created by Brad Bambara on 7/25/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#ifndef NoPL_Runtime_NoPLRuntime_h
#define NoPL_Runtime_NoPLRuntime_h

#include "NoPLValues.h"

#pragma mark -
#pragma mark Enums / structs for script interface

typedef enum
{
	NoPL_DataType_Void,
	NoPL_DataType_Number,
	NoPL_DataType_String,
	NoPL_DataType_Boolean,
	NoPL_DataType_Pointer,
} NoPL_DataType;

typedef struct
{
	union
	{
		float numberValue;
		char* stringValue;
		int booleanValue;
		void* pointerValue;
	};
	NoPL_DataType type;
} NoPL_FunctionValue;

typedef enum
{
	NoPL_StringFeedbackType_PrintStatement,
	NoPL_StringFeedbackType_RuntimeError,
	NoPL_StringFeedbackType_DebugInfo,
} NoPL_StringFeedbackType;

typedef struct
{
	NoPL_FunctionValue (*evaluateFunction)(void* calledOnObject, const char* functionName, const NoPL_FunctionValue* argv, unsigned int argc);
	NoPL_FunctionValue (*subscript)(void* calledOnObject, NoPL_FunctionValue index);
	void (*stringFeedback)(const char* string, NoPL_StringFeedbackType type);
} NoPL_Callbacks;

#pragma mark -
#pragma mark Struct helpers

#define NoPL_FunctionValue()				(NoPL_FunctionValue){0,NoPL_DataType_Void}
#define NoPL_Callbacks()					(NoPL_Callbacks){0,0,0}
#define NoPL_assignString(charStr, funcVal)	funcVal.stringValue = malloc(strlen(charStr)+1);strcpy(funcVal.stringValue, charStr);

#pragma mark -
#pragma mark Script API

void runScript(const NoPL_Instruction* scriptBuffer, unsigned int bufferLength, const NoPL_Callbacks* callbacks);

#endif