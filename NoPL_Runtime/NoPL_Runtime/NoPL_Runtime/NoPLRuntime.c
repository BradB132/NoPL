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

//TODO: test every feature of this language to make sure it works

typedef struct
{
	char* stringValue;
	int isAllocated;
}NoPL_String;

typedef struct
{
	const NoPL_Callbacks* callbacks;
	const NoPL_Instruction* scriptBuffer;
	unsigned int bufferLength;
	unsigned int evaluationPosition;
	NoPL_Index objectTableSize;
	NoPL_Index numberTableSize;
	NoPL_Index booleanTableSize;
	NoPL_Index stringTableSize;
	void** objectTable;
	float* numberTable;
	int* booleanTable;
	NoPL_String* stringTable;
}NoPL_Evaluation;

#define NoPL_String() ((NoPL_String){0,0})

//internal functions
void freeNoPL_String(NoPL_String* string);
int evaluateStatement(NoPL_Evaluation* eval);
float evaluateNumber(NoPL_Evaluation* eval);
void evaluateString(NoPL_Evaluation* eval, NoPL_String* outStr);
int evaluateBoolean(NoPL_Evaluation* eval);
void* evaluatePointer(NoPL_Evaluation* eval);
void evaluateFunction(NoPL_Evaluation* eval, NoPL_FunctionValue* returnVal);
void boolToString(int boolVal, NoPL_String* outStr);
void numberToString(float number, NoPL_String* outStr);

#pragma mark -
#pragma mark String memory management

void freeNoPL_String(NoPL_String* string)
{
	if(!string->isAllocated)
		return;
	string->isAllocated = 0;
	if(string->stringValue)
	{
		free(string->stringValue);
		string->stringValue = NULL;
	}
}

#pragma mark -
#pragma mark String casting

void boolToString(int boolVal, NoPL_String* outStr)
{
	if(boolVal)
	{
		outStr->stringValue = malloc(5);
		strcpy(outStr->stringValue, "true");
	}
	else
	{
		outStr->stringValue = malloc(6);
		strcpy(outStr->stringValue, "false");
	}
	outStr->isAllocated = 1;
}

void numberToString(float number, NoPL_String* outStr)
{
	outStr->stringValue = malloc(16);
	snprintf(outStr->stringValue, 16, "%g", number);
	outStr->isAllocated = 1;
}

#pragma mark -
#pragma mark Script evaluation

int evaluateStatement(NoPL_Evaluation* eval)
{
	//get the instruction type
	NoPL_Instruction instr = eval->scriptBuffer[eval->evaluationPosition];
	
	//skip over the instruction byte to the expression's actual value
	eval->evaluationPosition += sizeof(NoPL_Instruction);
	
	//do whatever operation we need for this statement
	switch(instr)
	{
		case NoPL_BYTE_NUMERIC_INCREMENT:
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//increment the variable at that index
			eval->numberTable[*varIndex]++;
		}
			break;
		case NoPL_BYTE_NUMERIC_DECREMENT:
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//increment the variable at that index
			eval->numberTable[*varIndex]--;
		}
			break;
		case NoPL_BYTE_NUMERIC_ASSIGN:
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//get the numeric value
			float numVal = evaluateNumber(eval);
			
			//assign the variable at that index
			eval->numberTable[*varIndex] = numVal;
		}
			break;
		case NoPL_BYTE_NUMERIC_ADD_ASSIGN:
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//get the numeric value
			float numVal = evaluateNumber(eval);
			
			//assign the variable at that index
			eval->numberTable[*varIndex] += numVal;
		}
			break;
		case NoPL_BYTE_NUMERIC_SUBTRACT_ASSIGN:
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//get the numeric value
			float numVal = evaluateNumber(eval);
			
			//assign the variable at that index
			eval->numberTable[*varIndex] -= numVal;
		}
			break;
		case NoPL_BYTE_NUMERIC_DIVIDE_ASSIGN:
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//get the numeric value
			float numVal = evaluateNumber(eval);
			
			//assign the variable at that index
			eval->numberTable[*varIndex] /= numVal;
		}
			break;
		case NoPL_BYTE_NUMERIC_MULTIPLY_ASSIGN:
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//get the numeric value
			float numVal = evaluateNumber(eval);
			
			//assign the variable at that index
			eval->numberTable[*varIndex] *= numVal;
		}
			break;
		case NoPL_BYTE_NUMERIC_EXPONENT_ASSIGN:
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//get the numeric value
			float numVal = evaluateNumber(eval);
			
			//assign the variable at that index
			eval->numberTable[*varIndex] = powf(eval->numberTable[*varIndex], numVal);
		}
			break;
		case NoPL_BYTE_NUMERIC_MODULO_ASSIGN:
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//get the numeric value
			float numVal = evaluateNumber(eval);
			
			//assign the variable at that index
			eval->numberTable[*varIndex] = fmodf(eval->numberTable[*varIndex], numVal);
		}
			break;
		case NoPL_BYTE_BOOLEAN_ASSIGN:
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//get the boolean value
			int boolVal = evaluateBoolean(eval);
			
			//assign the variable at that index
			eval->booleanTable[*varIndex] = boolVal;
		}
			break;
		case NoPL_BYTE_STRING_ASSIGN:
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//get the string value
			NoPL_String strVal = NoPL_String();
			evaluateString(eval, &strVal);
			
			//free the string that was occupying this index in the table
			freeNoPL_String(&eval->stringTable[*varIndex]);
			
			//assign the new string to the index
			eval->stringTable[*varIndex] = strVal;
		}
			break;
		case NoPL_BYTE_STRING_CONCAT_ASSIGN:
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//get the new string value
			NoPL_String newStr = NoPL_String();
			evaluateString(eval, &newStr);
			
			//get the old string value
			NoPL_String oldStr = eval->stringTable[*varIndex];
			
			//create the new value by concatenating the strings
			NoPL_String concatStr = NoPL_String();
			concatStr.stringValue = malloc(strlen(oldStr.stringValue)+strlen(newStr.stringValue)+1);
			strcpy(concatStr.stringValue, oldStr.stringValue);
			strcat(concatStr.stringValue, newStr.stringValue);
			
			//free the string that was occupying this index in the table
			freeNoPL_String(&oldStr);
			//free the copy of the string that we added on
			freeNoPL_String(&newStr);
			
			//assign the new string to the index
			eval->stringTable[*varIndex] = concatStr;
		}
			break;
		case NoPL_BYTE_STRING_PRINT:
		{
			//get the string value
			NoPL_String strObj = NoPL_String();
			evaluateString(eval, &strObj);
			
			//print to callback function
			if(eval->callbacks->stringFeedback)
				eval->callbacks->stringFeedback(strObj.stringValue, NoPL_StringFeedbackType_PrintStatement);
			
			freeNoPL_String(&strObj);
		}
			break;
		case NoPL_BYTE_OBJECT_ASSIGN:
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//get the pointer value
			void* ptrVal = evaluatePointer(eval);
			
			//assign the variable at that index
			eval->objectTable[*varIndex] = ptrVal;
		}
			break;
		case NoPL_BYTE_FUNCTION_CALL:
		case NoPL_BYTE_FUNCTION_INDEX:
			
			//back up to read the operator again
			eval->evaluationPosition -= sizeof(NoPL_Instruction);
			
			//evaluate the function
			NoPL_FunctionValue val = NoPL_FunctionValue();
			evaluateFunction(eval, &val);
			
			break;
		case NoPL_BYTE_CONDITIONAL:
		{
			//evaluate the conditional
			int condition = evaluateBoolean(eval);
			
			//get the buffer move if the conditional is not true
			NoPL_Index* buffMove = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			if(!condition)
				eval->evaluationPosition += *buffMove;
		}
			break;
		case NoPL_BYTE_BUFFER_MOVE_FORWARD:
		{
			//get the buffer move amount
			NoPL_Index* buffMove = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//adjust position forward by move amount
			eval->evaluationPosition += *buffMove;
		}
			break;
		case NoPL_BYTE_BUFFER_MOVE_BACKWARD:
		{
			//get the buffer move amount
			NoPL_Index* buffMove = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//adjust position backward by move amount
			eval->evaluationPosition -= *buffMove;
		}
			break;
		case NoPL_BYTE_PROGRAM_EXIT:
			return 0;
		case NoPL_BYTE_DEBUG_LINE:
		{
			//get the line number
			NoPL_Index* lineNum = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//format a string to send to the callback function
			char str[32];
			sprintf(str, "Line: %d", (int)(*lineNum));
			
			//call the callback function
			if(eval->callbacks->stringFeedback)
				eval->callbacks->stringFeedback(str, NoPL_StringFeedbackType_DebugInfo);
		}
			break;
		default:
			printf("Statement Error: instruction #%d doesn't make sense here\n", (int)instr);
			break;
	}
	
	return 1;
}

float evaluateNumber(NoPL_Evaluation* eval)
{
	//get the instruction type
	NoPL_Instruction instr = eval->scriptBuffer[eval->evaluationPosition];
	
	//skip over the instruction byte to the expression's actual value
	eval->evaluationPosition += sizeof(NoPL_Instruction);
	
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
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//return that index from the variable table
			return eval->numberTable[*varIndex];
		}
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
			NoPL_FunctionValue val = NoPL_FunctionValue();
			evaluateFunction(eval, &val);
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
		{
			NoPL_String strObj = NoPL_String();
			evaluateString(eval, &strObj);
			float returnVal = atoi(strObj.stringValue);
			freeNoPL_String(&strObj);
			return returnVal;
		}
		default:
			printf("Number Error: instruction #%d doesn't make sense here\n", (int)instr);
			break;
	}
	
	return 0.0f;
}

void evaluateString(NoPL_Evaluation* eval, NoPL_String* outStr)
{
	//get the instruction type
	NoPL_Instruction instr = eval->scriptBuffer[eval->evaluationPosition];
	
	//skip over the instruction byte to the expression's actual value
	eval->evaluationPosition += sizeof(NoPL_Instruction);
	
	//do whatever operation we need to get this string
	switch(instr)
	{
		case NoPL_BYTE_LITERAL_STRING:
		{
			//create a string struct from the string contained in the script's buffer
			outStr->stringValue = (char*)(eval->scriptBuffer+eval->evaluationPosition);
			outStr->isAllocated = 0;
			eval->evaluationPosition += strlen(outStr->stringValue)+1;
			return;
		}
		case NoPL_BYTE_VARIABLE_STRING:
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//return that index from the variable table
			outStr->stringValue = eval->stringTable[*varIndex].stringValue;
			outStr->isAllocated = 0;
			return;
		}
		case NoPL_BYTE_STRING_CONCAT:
		{
			//get the two strings
			NoPL_String strObj1 = NoPL_String();
			evaluateString(eval, &strObj1);
			NoPL_String strObj2 = NoPL_String();
			evaluateString(eval, &strObj2);
			
			//create a new string by combining the other two
			outStr->stringValue = malloc(strlen(strObj1.stringValue)+strlen(strObj2.stringValue)+1);
			outStr->isAllocated = 1;
			strcpy(outStr->stringValue, strObj1.stringValue);
			strcat(outStr->stringValue, strObj2.stringValue);
			
			//free the original strings
			freeNoPL_String(&strObj1);
			freeNoPL_String(&strObj2);
			
			return;
		}
		case NoPL_BYTE_RESOLVE_RESULT_TO_STRING:
		{
			//attempt to resolve this function result to a string value
			NoPL_FunctionValue val = NoPL_FunctionValue();
			evaluateFunction(eval, &val);
			if(val.type == NoPL_DataType_String)
			{
				outStr->stringValue = val.stringValue;
				outStr->isAllocated = 1;
			}
			else if(val.type == NoPL_DataType_Number)
			{
				numberToString(val.numberValue, outStr);
			}
			else if(val.type == NoPL_DataType_Boolean)
			{
				boolToString(val.booleanValue, outStr);
			}
			else
			{
				outStr->stringValue = NULL;
				outStr->isAllocated = 0;
			}
			return;
		}
		case NoPL_BYTE_CAST_NUMBER_TO_STRING:
			numberToString(evaluateNumber(eval), outStr);
			return;
		case NoPL_BYTE_CAST_BOOLEAN_TO_STRING:
			boolToString(evaluateBoolean(eval), outStr);
			return;
		default:
			printf("String Error: instruction #%d doesn't make sense here\n", (int)instr);
			break;
	}
}

int evaluateBoolean(NoPL_Evaluation* eval)
{
	//get the instruction type
	NoPL_Instruction instr = eval->scriptBuffer[eval->evaluationPosition];
	
	//skip over the instruction byte to the expression's actual value
	eval->evaluationPosition += sizeof(NoPL_Instruction);
	
	//do whatever operation we need to get this boolean
	switch(instr)
	{
		case NoPL_BYTE_LITERAL_BOOLEAN_TRUE:
			return 1;
		case NoPL_BYTE_LITERAL_BOOLEAN_FALSE:
			return 0;
		case NoPL_BYTE_VARIABLE_BOOLEAN:
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//return that index from the variable table
			return eval->booleanTable[*varIndex];
		}
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
		{
			//get the two strings
			NoPL_String strObj1 = NoPL_String();
			evaluateString(eval, &strObj1);
			NoPL_String strObj2 = NoPL_String();
			evaluateString(eval, &strObj2);
			
			//compare the strings
			int boolResult = !strcmp(strObj1.stringValue, strObj2.stringValue);
			
			//free the strings
			freeNoPL_String(&strObj1);
			freeNoPL_String(&strObj2);
			
			return boolResult;
		}
		case NoPL_BYTE_STRING_LOGICAL_INEQUALITY:
		{
			//get the two strings
			NoPL_String strObj1 = NoPL_String();
			evaluateString(eval, &strObj1);
			NoPL_String strObj2 = NoPL_String();
			evaluateString(eval, &strObj2);
			
			//compare the strings
			int boolResult = strcmp(strObj1.stringValue, strObj2.stringValue);
			
			//free the strings
			freeNoPL_String(&strObj1);
			freeNoPL_String(&strObj2);
			
			return boolResult;
		}
		case NoPL_BYTE_OBJECT_LOGICAL_EQUALITY:
			return (evaluatePointer(eval) == evaluatePointer(eval));
		case NoPL_BYTE_OBJECT_LOGICAL_INEQUALITY:
			return (evaluatePointer(eval) != evaluatePointer(eval));
		case NoPL_BYTE_RESOLVE_RESULT_TO_BOOLEAN:
		{
			//attempt to resolve this function result to a boolean value
			NoPL_FunctionValue val = NoPL_FunctionValue();
			evaluateFunction(eval, &val);
			if(val.type == NoPL_DataType_Boolean)
				return val.booleanValue;
			else if(val.type == NoPL_DataType_Number)
				return (val.numberValue != 0.0f);
			else if(val.type == NoPL_DataType_String)
				return (val.stringValue && strcmp(val.stringValue, ""));
			else
				return 0;
		}
		case NoPL_BYTE_CAST_NUMBER_TO_BOOLEAN:
			return (evaluateNumber(eval) != 0.0f);
		case NoPL_BYTE_CAST_STRING_TO_BOOLEAN:
		{
			//get the string from script buffer
			NoPL_String strObj = NoPL_String();
			evaluateString(eval, &strObj);
			
			//check if this is a string with a value
			int boolResult = (strObj.stringValue && strcmp(strObj.stringValue, ""));
			
			//we're done with the string
			freeNoPL_String(&strObj);
			
			return boolResult;
		}
		default:
			printf("Boolean Error: instruction #%d doesn't make sense here\n", (int)instr);
			break;
	}
	
	return 0;
}

void* evaluatePointer(NoPL_Evaluation* eval)
{
	//get the instruction type
	NoPL_Instruction instr = eval->scriptBuffer[eval->evaluationPosition];
	
	//skip over the instruction byte to the expression's actual value
	eval->evaluationPosition += sizeof(NoPL_Instruction);
	
	//do whatever operation we need to get this pointer
	switch(instr)
	{
		case NoPL_BYTE_LITERAL_NULL:
			return NULL;
		case NoPL_BYTE_VARIABLE_OBJECT:
		{
			//get the index
			NoPL_Index* varIndex = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//return that index from the variable table
			return eval->objectTable[*varIndex];
		}
		case NoPL_BYTE_RESOLVE_RESULT_TO_OBJECT:
		{
			//attempt to resolve this function result to an object value
			NoPL_FunctionValue val = NoPL_FunctionValue();
			evaluateFunction(eval, &val);
			if(val.type == NoPL_DataType_Pointer)
				return val.pointerValue;
			else
				return NULL;
		}
		default:
			printf("Pointer Error: instruction #%d doesn't make sense here\n", (int)instr);
			break;
	}
	
	return NULL;
}

void evaluateFunction(NoPL_Evaluation* eval, NoPL_FunctionValue* returnVal)
{
	//get the instruction type
	NoPL_Instruction instr = eval->scriptBuffer[eval->evaluationPosition];
	
	//skip over the instruction byte to the expression's actual value
	eval->evaluationPosition += sizeof(NoPL_Instruction);
	
	//get the object that the function is being called on
	void* ptr = evaluatePointer(eval);
	
	//do whatever operation we need to get this function result
	switch(instr)
	{
		case NoPL_BYTE_FUNCTION_CALL:
		{
			//read a string from the buffer
			char* funcName = (char*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += strlen(funcName)+1;
			
			//how many arguments are there?
			NoPL_Index* argCount = (NoPL_Index*)(eval->scriptBuffer+eval->evaluationPosition);
			eval->evaluationPosition += sizeof(NoPL_Index);
			
			//set up an array for the args
			NoPL_FunctionValue argv[*argCount];
			memset(argv, 0, sizeof(NoPL_FunctionValue)*(*argCount));
			
			//set up a list of string args (better way to do this?)
			NoPL_String releaseStrings[*argCount];
			memset(releaseStrings, 0, sizeof(NoPL_String)*(*argCount));
			int releaseStringCount = 0;
			
			//populate the args with their values from the script
			for(int i = 0; i < *argCount; i++)
			{
				//get the arg type
				NoPL_Instruction argType = eval->scriptBuffer[eval->evaluationPosition];
				
				//skip over the instruction byte to the expression's actual value
				eval->evaluationPosition += sizeof(NoPL_Instruction);
				
				switch (argType)
				{
					case NoPL_BYTE_ARG_BOOLEAN:
						argv[i].type = NoPL_DataType_Boolean;
						argv[i].booleanValue = evaluateBoolean(eval);
						break;
					case NoPL_BYTE_ARG_NUMBER:
						argv[i].type = NoPL_DataType_Number;
						argv[i].numberValue = evaluateNumber(eval);
						break;
					case NoPL_BYTE_ARG_OBJECT:
						argv[i].type = NoPL_DataType_Pointer;
						argv[i].pointerValue = evaluatePointer(eval);
						break;
					case NoPL_BYTE_ARG_STRING:
					{
						//get the string value
						NoPL_String strObj = NoPL_String();
						evaluateString(eval, &strObj);
						
						argv[i].type = NoPL_DataType_String;
						argv[i].stringValue = strObj.stringValue;
						
						//put this in a list for deleting later if it was allocated
						if(strObj.isAllocated)
						{
							releaseStrings[releaseStringCount] = strObj;
							releaseStringCount++;
						}
					}
						break;
					default:
						printf("Function Error: instruction #%d doesn't make sense here\n", (int)argType);
						break;
				}
			}
			
			//call the function
			*returnVal = eval->callbacks->evaluateFunction(ptr, funcName, argv, (unsigned int)(*argCount));
			
			//release any strings that were allocated
			for(int i = 0; i < releaseStringCount; i++)
				freeNoPL_String(&releaseStrings[i]);
		}
			break;
		case NoPL_BYTE_FUNCTION_INDEX:
		{
			//get the type of the subscript
			NoPL_FunctionValue index = NoPL_FunctionValue();
			NoPL_Instruction argType = eval->scriptBuffer[eval->evaluationPosition];
			eval->evaluationPosition += sizeof(NoPL_Instruction);
			if(argType == NoPL_BYTE_ARG_NUMBER)
			{
				//set a numeric value for the index
				index.type = NoPL_DataType_Number;
				index.numberValue = evaluateNumber(eval);
				
				//call the function
				*returnVal = eval->callbacks->subscript(ptr, index);
			}
			else if (argType == NoPL_BYTE_ARG_STRING)
			{
				//set a string value for the index
				index.type = NoPL_DataType_String;
				NoPL_String strObj = NoPL_String();
				evaluateString(eval, &strObj);
				index.stringValue = strObj.stringValue;
				
				//call the function
				*returnVal = eval->callbacks->subscript(ptr, index);
				
				//clean up the string
				freeNoPL_String(&strObj);
			}
			else
				printf("Function Error: instruction #%d doesn't make sense here\n", (int)argType);
		}
			break;
		default:
			printf("Function Error: instruction #%d doesn't make sense here\n", (int)instr);
			break;
	}
}

void runScript(const NoPL_Instruction* scriptBuffer, unsigned int bufferLength, const NoPL_Callbacks* callbacks)
{
	//set up an evaluation struct
	NoPL_Evaluation eval;
	eval.callbacks = callbacks;
	eval.scriptBuffer = scriptBuffer;
	eval.bufferLength = bufferLength;
	eval.evaluationPosition = 0;
	
	//get the sizes of the variable tables
	eval.objectTableSize = 0;
	eval.numberTableSize = 0;
	eval.booleanTableSize = 0;
	eval.stringTableSize = 0;
	for(int i = 0; i < 4; i++)
	{
		NoPL_Instruction instr = eval.scriptBuffer[eval.evaluationPosition];
		if(instr == NoPL_BYTE_BOOLEAN_TABLE_SIZE)
		{
			eval.evaluationPosition += sizeof(NoPL_Instruction);
			NoPL_Index* sizePtr = (NoPL_Index*)(eval.scriptBuffer+eval.evaluationPosition);
			eval.booleanTableSize = *sizePtr;
		}
		else if(instr == NoPL_BYTE_NUMERIC_TABLE_SIZE)
		{
			eval.evaluationPosition += sizeof(NoPL_Instruction);
			NoPL_Index* sizePtr = (NoPL_Index*)(eval.scriptBuffer+eval.evaluationPosition);
			eval.numberTableSize = *sizePtr;
		}
		else if(instr == NoPL_BYTE_OBJECT_TABLE_SIZE)
		{
			eval.evaluationPosition += sizeof(NoPL_Instruction);
			NoPL_Index* sizePtr = (NoPL_Index*)(eval.scriptBuffer+eval.evaluationPosition);
			eval.objectTableSize = *sizePtr;
		}
		else if(instr == NoPL_BYTE_STRING_TABLE_SIZE)
		{
			eval.evaluationPosition += sizeof(NoPL_Instruction);
			NoPL_Index* sizePtr = (NoPL_Index*)(eval.scriptBuffer+eval.evaluationPosition);
			eval.stringTableSize = *sizePtr;
		}
		else
			break;
		
		//advance to the next position in the buffer
		eval.evaluationPosition += sizeof(NoPL_Index);
	}
	
	//create tables for variables
	int booleanTable[eval.booleanTableSize];
	float numberTable[eval.numberTableSize];
	void* objectTable[eval.objectTableSize];
	NoPL_String stringTable[eval.stringTableSize];
	eval.booleanTable = booleanTable;
	eval.numberTable = numberTable;
	eval.objectTable = objectTable;
	eval.stringTable = stringTable;
	memset(stringTable, 0, sizeof(NoPL_String)*eval.stringTableSize);
	
	//evaluate all statements in the script
	int evaluateOK = 1;
	while(evaluateOK && eval.evaluationPosition < eval.bufferLength)
		evaluateOK = evaluateStatement(&eval);
	
	//free any strings that were allocated
	for(int i = 0; i < eval.stringTableSize; i++)
		if(stringTable[i].isAllocated && stringTable[i].stringValue)
			free(stringTable[i].stringValue);
}