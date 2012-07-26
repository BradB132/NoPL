//
//  NoPLRuntime.c
//  NoPL_Runtime
//
//  Created by Brad Bambara on 7/25/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#include <stdio.h>
#include "NoPLRuntime.h"

typedef struct
{
	const NoPL_Callbacks* callbacks;
	const NoPL_Instruction* scriptBuffer;
	unsigned int bufferLength;
	unsigned int evaluationPosition;
}NoPL_Evaluation;

//internal functions
void evaluateStatement(NoPL_Evaluation* eval);
float evaluateNumber(NoPL_Evaluation* eval);
char* evaluateString(NoPL_Evaluation* eval);
int evaluateBoolean(NoPL_Evaluation* eval);
void* evaluatePointer(NoPL_Evaluation* eval);
NoPL_FunctionValue evaluateFunction(NoPL_Evaluation* eval);

void evaluateStatement(NoPL_Evaluation* eval)
{
	switch(eval->scriptBuffer[eval->evaluationPosition])
	{
		case NoPL_BYTE_NUMERIC_INCREMENT:
			break;
		case NoPL_BYTE_NUMERIC_DECREMENT:
			break;
		case NoPL_BYTE_NUMERIC_ASSIGN:
			break;
		case NoPL_BYTE_NUMERIC_ADD_ASSIGN:
			break;
		case NoPL_BYTE_NUMERIC_SUBTRACT_ASSIGN:
			break;
		case NoPL_BYTE_NUMERIC_DIVIDE_ASSIGN:
			break;
		case NoPL_BYTE_NUMERIC_MULTIPLY_ASSIGN:
			break;
		case NoPL_BYTE_NUMERIC_EXPONENT_ASSIGN:
			break;
		case NoPL_BYTE_NUMERIC_MODULO_ASSIGN:
			break;
		case NoPL_BYTE_BOOLEAN_ASSIGN:
			break;
		case NoPL_BYTE_STRING_ASSIGN:
			break;
		case NoPL_BYTE_STRING_CONCAT_ASSIGN:
			break;
		case NoPL_BYTE_STRING_PRINT:
			break;
		case NoPL_BYTE_OBJECT_ASSIGN:
			break;
		case NoPL_BYTE_FUNCTION_CALL:
			//global function call
			break;
		case NoPL_BYTE_FUNCTION_INDEX:
			//TODO: this doesn't really make sense, but can happen
			break;
		case NoPL_BYTE_CONDITIONAL:
			break;
		case NoPL_BYTE_BUFFER_MOVE:
			break;
		case NoPL_BYTE_PROGRAM_EXIT:
			break;
		case NoPL_BYTE_DEBUG_LINE:
			break;
		default:
			printf("Statement Error: instruction #%d doesn't make sense here", (int)eval->scriptBuffer[eval->evaluationPosition]);
			break;
	}
}

float evaluateNumber(NoPL_Evaluation* eval)
{
	switch(eval->scriptBuffer[eval->evaluationPosition])
	{
		case NoPL_BYTE_LITERAL_NUMBER:
			break;
		case NoPL_BYTE_VARIABLE_NUMBER:
			break;
		case NoPL_BYTE_NUMERIC_ADD:
			break;
		case NoPL_BYTE_NUMERIC_SUBTRACT:
			break;
		case NoPL_BYTE_NUMERIC_DIVIDE:
			break;
		case NoPL_BYTE_NUMERIC_MULTIPLY:
			break;
		case NoPL_BYTE_NUMERIC_EXPONENT:
			break;
		case NoPL_BYTE_NUMERIC_MODULO:
			break;
		case NoPL_BYTE_NUMERIC_ABS_VALUE:
			break;
		case NoPL_BYTE_NUMERIC_NEGATION:
			break;
		case NoPL_BYTE_RESOLVE_RESULT_TO_NUMBER:
			break;
		case NoPL_BYTE_CAST_BOOLEAN_TO_NUMBER:
			break;
		case NoPL_BYTE_CAST_STRING_TO_NUMBER:
			break;
		default:
			printf("Number Error: instruction #%d doesn't make sense here", (int)eval->scriptBuffer[eval->evaluationPosition]);
			break;
	}
	
	return 0;
}

char* evaluateString(NoPL_Evaluation* eval)
{
	switch(eval->scriptBuffer[eval->evaluationPosition])
	{
		case NoPL_BYTE_LITERAL_STRING:
			break;
		case NoPL_BYTE_VARIABLE_STRING:
			break;
		case NoPL_BYTE_STRING_CONCAT:
			break;
		case NoPL_BYTE_RESOLVE_RESULT_TO_STRING:
			break;
		case NoPL_BYTE_CAST_NUMBER_TO_STRING:
			break;
		case NoPL_BYTE_CAST_BOOLEAN_TO_STRING:
			break;
		default:
			printf("String Error: instruction #%d doesn't make sense here", (int)eval->scriptBuffer[eval->evaluationPosition]);
			break;
	}
	
	return NULL;
}

int evaluateBoolean(NoPL_Evaluation* eval)
{
	switch(eval->scriptBuffer[eval->evaluationPosition])
	{
		case NoPL_BYTE_LITERAL_BOOLEAN_TRUE:
			return 1;
		case NoPL_BYTE_LITERAL_BOOLEAN_FALSE:
			return 0;
		case NoPL_BYTE_VARIABLE_BOOLEAN:
			break;
		case NoPL_BYTE_NUMERIC_LOGICAL_EQUALITY:
			break;
		case NoPL_BYTE_NUMERIC_LOGICAL_INEQUALITY:
			break;
		case NoPL_BYTE_BOOLEAN_LOGICAL_EQUALITY:
			break;
		case NoPL_BYTE_BOOLEAN_LOGICAL_INEQUALITY:
			break;
		case NoPL_BYTE_BOOLEAN_AND:
			break;
		case NoPL_BYTE_BOOLEAN_OR:
			break;
		case NoPL_BYTE_BOOLEAN_LESS_THAN:
			break;
		case NoPL_BYTE_BOOLEAN_LESS_THAN_EQUAL:
			break;
		case NoPL_BYTE_BOOLEAN_GREATER_THAN:
			break;
		case NoPL_BYTE_BOOLEAN_GREATER_THAN_EQUAL:
			break;
		case NoPL_BYTE_BOOLEAN_NEGATION:
			break;
		case NoPL_BYTE_STRING_LOGICAL_EQUALITY:
			break;
		case NoPL_BYTE_STRING_LOGICAL_INEQUALITY:
			break;
		case NoPL_BYTE_OBJECT_LOGICAL_EQUALITY:
			break;
		case NoPL_BYTE_OBJECT_LOGICAL_INEQUALITY:
			break;
		case NoPL_BYTE_RESOLVE_RESULT_TO_BOOLEAN:
			break;
		case NoPL_BYTE_CAST_NUMBER_TO_BOOLEAN:
			break;
		case NoPL_BYTE_CAST_STRING_TO_BOOLEAN:
			break;
		default:
			printf("Boolean Error: instruction #%d doesn't make sense here", (int)eval->scriptBuffer[eval->evaluationPosition]);
			break;
	}
	
	return 0;
}

void* evaluatePointer(NoPL_Evaluation* eval)
{
	switch(eval->scriptBuffer[eval->evaluationPosition])
	{
		case NoPL_BYTE_LITERAL_NULL:
			return NULL;
		case NoPL_BYTE_VARIABLE_OBJECT:
			break;
		case NoPL_BYTE_RESOLVE_RESULT_TO_OBJECT:
			break;
		default:
			printf("Pointer Error: instruction #%d doesn't make sense here", (int)eval->scriptBuffer[eval->evaluationPosition]);
			break;
	}
	
	return NULL;
}

NoPL_FunctionValue evaluateFunction(NoPL_Evaluation* eval)
{
	//TODO: fancy function call
}

void runScript(const NoPL_Instruction* scriptBuffer, unsigned int bufferLength, const NoPL_Callbacks* callbacks)
{
	//set up an evaluation struct
	NoPL_Evaluation eval;
	eval.callbacks = callbacks;
	
	//TODO: variable tables
	
	evaluateStatement(&eval);
}