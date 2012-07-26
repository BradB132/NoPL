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

typedef enum
{
	NoPL_DataType_Number,
	NoPL_DataType_String,
	NoPL_DataType_Boolean,
	NoPL_DataType_Pointer,
	NoPL_DataType_Void,
}NoPL_DataType;

typedef struct
{
	float numberValue;
	char* stringValue;
	int booleanValue;
	void* pointerValue;
	NoPL_DataType type;
}NoPL_FunctionValue;

typedef enum
{
	NoPL_StringFeedbackType_PrintStatement,
	NoPL_StringFeedbackType_RuntimeError,
	NoPL_StringFeedbackType_DebugInfo,
}NoPL_StringFeedbackType;

typedef struct
{
	NoPL_FunctionValue (*evaluateFunction)(void* calledOnObject, char* functionName, NoPL_FunctionValue* argv, unsigned int argc);
	NoPL_FunctionValue (*subscript)(void* calledOnObject, NoPL_FunctionValue index);
	void (*stringFeedback)(char* string, NoPL_StringFeedbackType type);
}NoPL_Callbacks;

void runScript(const NoPL_Instruction* scriptBuffer, unsigned int bufferLength, const NoPL_Callbacks* callbacks);

#endif
