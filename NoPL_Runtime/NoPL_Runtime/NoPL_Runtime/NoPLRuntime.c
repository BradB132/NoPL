//
//  NoPLRuntime.c
//  NoPL_Runtime
//
//  Created by Brad Bambara on 7/25/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
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
	//get the instruction type
	NoPL_Instruction instr = eval->scriptBuffer[eval->evaluationPosition];
	
	//skip over the instruction byte to the number's actual value
	eval->evaluationPosition++;
	
	//do whatever operation we need to get this number
	switch(instr)
	{
		case NoPL_BYTE_LITERAL_NUMBER:
		{
			float* literalPtr = (float*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(float);
			return *literalPtr;
		}
		case NoPL_BYTE_VARIABLE_NUMBER:
			//TODO: symbol table
		case NoPL_BYTE_NUMERIC_ADD:
			return evaluateNumber(eval) + evaluateNumber(eval);
		case NoPL_BYTE_NUMERIC_SUBTRACT:
			return evaluateNumber(eval) - evaluateNumber(eval);
		case NoPL_BYTE_NUMERIC_DIVIDE:
			return evaluateNumber(eval) / evaluateNumber(eval);
		case NoPL_BYTE_NUMERIC_MULTIPLY:
			return evaluateNumber(eval) * evaluateNumber(eval);
		case NoPL_BYTE_NUMERIC_EXPONENT:
			return powf(evaluateNumber(eval), evaluateNumber(eval));
		case NoPL_BYTE_NUMERIC_MODULO:
			return fmodf(evaluateNumber(eval), evaluateNumber(eval));
		case NoPL_BYTE_NUMERIC_ABS_VALUE:
			return fabsf(evaluateNumber(eval));
		case NoPL_BYTE_NUMERIC_NEGATION:
			return -evaluateNumber(eval);
		case NoPL_BYTE_RESOLVE_RESULT_TO_NUMBER:
		{
			//attempt to resolve this function result to a numeric value
			NoPL_FunctionValue val = evaluateFunction(eval);
			if(val.type == NoPL_DataType_Number)
				return val.numberValue;
			else if(val.type == NoPL_DataType_Boolean)
				return (float)val.booleanValue;
			else if(val.type == NoPL_DataType_String)
				return atoi(val.stringValue);
			else
				return 0.0f;
		}
		case NoPL_BYTE_CAST_BOOLEAN_TO_NUMBER:
			return (float)evaluateBoolean(eval);
		case NoPL_BYTE_CAST_STRING_TO_NUMBER:
			return atoi(evaluateString(eval));
		default:
			printf("Number Error: instruction #%d doesn't make sense here", (int)eval->scriptBuffer[eval->evaluationPosition]);
			break;
	}
	
	return 0.0f;
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
	//get the instruction type
	NoPL_Instruction instr = eval->scriptBuffer[eval->evaluationPosition];
	
	//skip over the instruction byte to the number's actual value
	eval->evaluationPosition++;
	
	//do whatever operation we need to get this number
	switch(instr)
	{
		case NoPL_BYTE_LITERAL_BOOLEAN_TRUE:
			eval->evaluationPosition++;
			return 1;
		case NoPL_BYTE_LITERAL_BOOLEAN_FALSE:
			eval->evaluationPosition++;
			return 0;
		case NoPL_BYTE_VARIABLE_BOOLEAN:
			//TODO: symbol tables
		case NoPL_BYTE_NUMERIC_LOGICAL_EQUALITY:
			return (evaluateNumber(eval) == evaluateNumber(eval));
		case NoPL_BYTE_NUMERIC_LOGICAL_INEQUALITY:
			return (evaluateNumber(eval) != evaluateNumber(eval));
		case NoPL_BYTE_BOOLEAN_LOGICAL_EQUALITY:
			return (evaluateBoolean(eval) == evaluateBoolean(eval));
		case NoPL_BYTE_BOOLEAN_LOGICAL_INEQUALITY:
			return (evaluateBoolean(eval) != evaluateBoolean(eval));
		case NoPL_BYTE_BOOLEAN_AND:
			return (evaluateBoolean(eval) && evaluateBoolean(eval));
		case NoPL_BYTE_BOOLEAN_OR:
			return (evaluateBoolean(eval) || evaluateBoolean(eval));
		case NoPL_BYTE_BOOLEAN_LESS_THAN:
			return (evaluateNumber(eval) < evaluateNumber(eval));
		case NoPL_BYTE_BOOLEAN_LESS_THAN_EQUAL:
			return (evaluateNumber(eval) <= evaluateNumber(eval));
		case NoPL_BYTE_BOOLEAN_GREATER_THAN:
			return (evaluateNumber(eval) > evaluateNumber(eval));
		case NoPL_BYTE_BOOLEAN_GREATER_THAN_EQUAL:
			return (evaluateNumber(eval) >= evaluateNumber(eval));
		case NoPL_BYTE_BOOLEAN_NEGATION:
			return !evaluateBoolean(eval);
		case NoPL_BYTE_STRING_LOGICAL_EQUALITY:
			return !strcmp(evaluateString(eval), evaluateString(eval));
		case NoPL_BYTE_STRING_LOGICAL_INEQUALITY:
			return strcmp(evaluateString(eval), evaluateString(eval));
		case NoPL_BYTE_OBJECT_LOGICAL_EQUALITY:
			return (evaluatePointer(eval) == evaluatePointer(eval));
		case NoPL_BYTE_OBJECT_LOGICAL_INEQUALITY:
			return (evaluatePointer(eval) != evaluatePointer(eval));
		case NoPL_BYTE_RESOLVE_RESULT_TO_BOOLEAN:
		{
			//attempt to resolve this function result to a boolean value
			NoPL_FunctionValue val = evaluateFunction(eval);
			if(val.type == NoPL_DataType_Boolean)
				return val.booleanValue;
			else if(val.type == NoPL_DataType_Number)
				return (val.numberValue != 0.0f);
			else if(value.type == NoPL_DataType_String)
				return (val.stringValue && strcmp(val.stringValue, ""));
			else
				return 0;
		}
		case NoPL_BYTE_CAST_NUMBER_TO_BOOLEAN:
			return (evaluateNumber(eval) != 0.0f);
		case NoPL_BYTE_CAST_STRING_TO_BOOLEAN:
		{
			char* str = evaluateString(eval);
			return (str && strcmp(str, ""));
		}
		default:
			printf("Boolean Error: instruction #%d doesn't make sense here", (int)eval->scriptBuffer[eval->evaluationPosition]);
			break;
	}
	
	return 0;
}

void* evaluatePointer(NoPL_Evaluation* eval)
{
	//get the instruction type
	NoPL_Instruction instr = eval->scriptBuffer[eval->evaluationPosition];
	
	//skip over the instruction byte to the number's actual value
	eval->evaluationPosition++;
	
	//do whatever operation we need to get this number
	switch(instr)
	{
		case NoPL_BYTE_LITERAL_NULL:
			eval->evaluationPosition++;
			return NULL;
		case NoPL_BYTE_VARIABLE_OBJECT:
			//TODO: symbol table
		case NoPL_BYTE_RESOLVE_RESULT_TO_OBJECT:
		{
			//attempt to resolve this function result to an object value
			NoPL_FunctionValue val = evaluateFunction(eval);
			if(val.type == NoPL_DataType_Pointer)
				return val.pointerValue;
			else
				return NULL;
		}
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