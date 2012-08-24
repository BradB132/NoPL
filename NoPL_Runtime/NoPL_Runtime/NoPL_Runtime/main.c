//
//  main.c
//  NoPL_Runtime
//
//  Created by Brad Bambara on 7/25/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NoPLRuntime.h"
#include "NoPLStandardFunctions.h"

char* mallocBufferFromFilePath(const char* filePath, unsigned long* outLength)
{
	FILE* file;
	char* buffer;
	unsigned long fileLen;
	
	//Open file
	file = fopen(filePath, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", filePath);
		return 0;
	}
	
	//Get file length
	fseek(file, 0, SEEK_END);
	fileLen = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	//Allocate memory
	buffer = (char*)malloc(fileLen+1);
	if(!buffer)
	{
		fprintf(stderr, "Memory error!");
		fclose(file);
		return 0;
	}
	
	//Read file contents into buffer
	fread(buffer, 1, fileLen, file);
	fclose(file);
	
	*outLength = fileLen;
	return buffer;
}

NoPL_FunctionValue testingEvalFunc(void* calledOnObject, const char* functionName, const NoPL_FunctionValue* argv, unsigned int argc)
{
	NoPL_FunctionValue returnValue = NoPL_FunctionValue();
	
	//check standard functions for a value
	returnValue = nopl_standardFunctions(calledOnObject, functionName, argv, argc);
	if(returnValue.type != NoPL_DataType_Void)
		return returnValue;
	
	return returnValue;
}

NoPL_FunctionValue testingSubscript(void* calledOnObject, NoPL_FunctionValue index)
{
	return NoPL_FunctionValue();
}

void testStrings(const char* string, NoPL_StringFeedbackType type)
{
	switch (type) {
		case NoPL_StringFeedbackType_PrintStatement:
			printf("NoPL Print: %s\n", string);
			return;
		case NoPL_StringFeedbackType_DebugInfo:
			printf("NoPL Debug: %s\n", string);
			return;
		case NoPL_StringFeedbackType_RuntimeError:
			printf("NoPL Error: %s\n", string);
			return;
	}
}

int main(int argc, const char * argv[])
{
	//check to make sure we have a path specified
	if(argc < 2)
	{
		printf("You must supply a path for the script");
		return -1;
	}
	
	//load the script data from file specified from command line
	unsigned long fileLength;
	char* scriptBuffer = mallocBufferFromFilePath(argv[1], &fileLength);
	
	//set up callback function pointers
	NoPL_Callbacks callbacks = NoPL_Callbacks();
	callbacks.evaluateFunction = &testingEvalFunc;
	callbacks.subscript = &testingSubscript;
	callbacks.stringFeedback = &testStrings;
	
	//run the script
	runScript((NoPL_Instruction*)scriptBuffer, (unsigned int)fileLength, &callbacks);
	
	//delete the used script data
	free(scriptBuffer);
	
    return 0;
}

