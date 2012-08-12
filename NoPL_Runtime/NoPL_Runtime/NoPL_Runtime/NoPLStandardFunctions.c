//
//  NoPLStandardFunctions.c
//  NoPL_Runtime
//
//  Created by Brad Bambara on 8/11/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "NoPLStandardFunctions.h"

char NoPL_StandardFunction_Identifier_String;
char NoPL_StandardFunction_Identifier_Math;

#pragma mark -
#pragma mark fast hash for looking up function names

unsigned long hash_djb2(unsigned char* str)
{
	unsigned long hash = 5381;
	int c;
	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;
	return hash;
}

#pragma mark -
#pragma mark object accessors

void* nopl_standardMath()
{
	return &NoPL_StandardFunction_Identifier_Math;
}

void* nopl_standardString()
{
	return &NoPL_StandardFunction_Identifier_String;
}

#pragma mark -
#pragma mark standard functions

NoPL_FunctionValue nopl_standardFunctions(void* calledOnObject, char* functionName, NoPL_FunctionValue* argv, unsigned int argc)
{
	//create a blank function result
	NoPL_FunctionValue returnVal = NoPL_FunctionValue();
	unsigned int hashedFunctionName = (unsigned int)(hash_djb2((unsigned char*)functionName) % 0xFFFFFF);
	
	//check if this is a global
	if(!calledOnObject)
	{
		//bail if we have arguments
		if(argc)
			return returnVal;
		
		//return pointers to classes based on name
		switch (hashedFunctionName)
		{
			case 8921961://Str
				returnVal.type = NoPL_DataType_Pointer;
				returnVal.pointerValue = &NoPL_StandardFunction_Identifier_String;
				break;
			case 8975915://Math
				returnVal.type = NoPL_DataType_Pointer;
				returnVal.pointerValue = &NoPL_StandardFunction_Identifier_Math;
				break;
		}
	}
	//not a global, are we calling the String class?
	else if(calledOnObject == &NoPL_StandardFunction_Identifier_String)
	{
		//check the name of the function
		switch (hashedFunctionName)
		{
			case 15501227://format
			{
				//TODO: implement format
			}
				break;
			case 3423698://length
			{
				//bail if we don't have one string argument
				if(argc != 1 || argv[0].type != NoPL_DataType_String)
					break;
				
				//return the length of the given string
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = (float)strlen(argv[0].stringValue);
			}
				break;
			case 4434095://fastHash
			{
				//bail if we don't have one string argument
				if(argc != 1 || argv[0].type != NoPL_DataType_String)
					break;
				
				//hash the string, return the number
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = (float)(hash_djb2((unsigned char*)argv[0].stringValue) % 0xFFFFFF);
			}
				break;
			case 8299935://toLower
			{
				//bail if we don't have one string argument
				if(argc != 1 || argv[0].type != NoPL_DataType_String)
					break;
				
				//copy the string
				returnVal.type = NoPL_DataType_String;
				NoPL_assignString(argv[0].stringValue, returnVal);
				
				//make the copied string lowercase
				int i;
				for(i = 0; returnVal.stringValue[i]; i++)
					returnVal.stringValue[i] = tolower(returnVal.stringValue[i]);
			}
				break;
			case 2224323://toUpper
			{
				//bail if we don't have one string argument
				if(argc != 1 || argv[0].type != NoPL_DataType_String)
					break;
				
				//copy the string
				returnVal.type = NoPL_DataType_String;
				NoPL_assignString(argv[0].stringValue, returnVal);
				
				//make the copied string lowercase
				int i;
				for(i = 0; returnVal.stringValue[i]; i++)
					returnVal.stringValue[i] = toupper(returnVal.stringValue[i]);
			}
				break;
			case 7515466://substring
			{
				//TODO: implement substring
			}
				break;
			case 1421073://indexOf
			{
				//TODO: implement indexOf
			}
				break;
			case 12584000://replace
			{
				//TODO: implement replace
			}
				break;
		}
	}
	//not global or string, are we calling the Math class?
	else if(calledOnObject == &NoPL_StandardFunction_Identifier_Math)
	{
		//check the name of the function
		switch (hashedFunctionName)
		{
			case 5862622://PI
			{
				//bail if we have args
				if(argc)
					break;
				
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = M_PI;
			}
				break;
			case 177674://e
			{
				//bail if we have args
				if(argc)
					break;
				
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = M_E;
			}
				break;
			case 15378630://DEGREE_TO_RADIAN
			{
				//bail if we don't have the correct args
				if(argc > 1 || (argc == 1 && argv[0].type != NoPL_DataType_Number))
					break;
				
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = M_PI/180.f;
				
				if(argc == 1)
				{
					//we actually are taking an arg and multipling it times the constant
					returnVal.numberValue *= argv[0].numberValue;
				}
			}
				break;
			case 911202://RADIAN_TO_DEGREE
			{
				//bail if we don't have the correct args
				if(argc > 1 || (argc == 1 && argv[0].type != NoPL_DataType_Number))
					break;
				
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = 180.0f/M_PI;
				
				if(argc == 1)
				{
					//we actually are taking an arg and multipling it times the constant
					returnVal.numberValue *= argv[0].numberValue;
				}
			}
				break;
			case 10358891://sqrt
			{
				//bail if we don't have the correct args
				if(argc != 1 || argv[0].type != NoPL_DataType_Number)
					break;
				
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = sqrtf(argv[0].numberValue);
			}
				break;
			case 8956442://sin
			{
				//bail if we don't have the correct args
				if(argc != 1 || argv[0].type != NoPL_DataType_Number)
					break;
				
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = sinf(argv[0].numberValue);
			}
				break;
			case 8939221://cos
			{
				//bail if we don't have the correct args
				if(argc != 1 || argv[0].type != NoPL_DataType_Number)
					break;
				
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = cosf(argv[0].numberValue);
			}
				break;
			case 8957267://tan
			{
				//bail if we don't have the correct args
				if(argc != 1 || argv[0].type != NoPL_DataType_Number)
					break;
				
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = tanf(argv[0].numberValue);
			}
				break;
			case 9713900://asin
			{
				//bail if we don't have the correct args
				if(argc != 1 || argv[0].type != NoPL_DataType_Number)
					break;
				
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = asinf(argv[0].numberValue);
			}
				break;
			case 9696679://acos
			{
				//bail if we don't have the correct args
				if(argc != 1 || argv[0].type != NoPL_DataType_Number)
					break;
				
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = acosf(argv[0].numberValue);
			}
				break;
			case 9714725://atan
			{
				//bail if we don't have the correct args
				if(argc != 1 || argv[0].type != NoPL_DataType_Number)
					break;
				
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = atanf(argv[0].numberValue);
			}
				break;
			case 1818890://atan2
			{
				//bail if we don't have the correct args
				if(argc != 2 || argv[0].type != NoPL_DataType_Number || argv[1].type != NoPL_DataType_Number)
					break;
				
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = atan2f(argv[0].numberValue, argv[1].numberValue);
			}
				break;
			case 15385150://random
			{
				//bail if we have more than 2 args, or if any non-numbers were passed in
				if(argc > 2 ||
				   (argc == 1 && argv[0].type != NoPL_DataType_Number) ||
				   (argc == 2 && (argv[0].type != NoPL_DataType_Number || argv[1].type != NoPL_DataType_Number)) )
					break;
				
				//seed the random function with time (only the first time)
				static int firstCall = 1;
				if(firstCall)
				{
					srand((unsigned int)time(NULL));
					firstCall = 0;
				}
				
				//calc a normalized random number
				int random = rand();
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = (float)random/(float)RAND_MAX;
				srand(random);
				
				//multiply the normalized if we have arguments
				if(argc == 1)
					returnVal.numberValue *= argv[0].numberValue;
				else if(argc == 2)
					returnVal.numberValue = argv[0].numberValue+returnVal.numberValue*(argv[1].numberValue-argv[0].numberValue);
			}
				break;
			case 8949010://log
			{
				//bail if we have the wrong number of args, or if any non-numbers were passed in
				if(argc < 1 || argc > 2 || argv[0].type != NoPL_DataType_Number || (argc == 2 && argv[1].type != NoPL_DataType_Number))
					break;
				
				//calc the log
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = logf(argv[0].numberValue);
				
				//divide to get a different base if we have a second value
				if(argc == 2)
					returnVal.numberValue /= logf(argv[1].numberValue);
			}
				break;
			case 5044477://round
			{
				//bail if we don't have the correct args
				if(argc != 1 || argv[0].type != NoPL_DataType_Number)
					break;
				
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = roundf(argv[0].numberValue);
			}
				break;
			case 9770526://ceil
			{
				//bail if we don't have the correct args
				if(argc != 1 || argv[0].type != NoPL_DataType_Number)
					break;
				
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = ceilf(argv[0].numberValue);
			}
				break;
			case 7476342://floor
			{
				//bail if we don't have the correct args
				if(argc != 1 || argv[0].type != NoPL_DataType_Number)
					break;
				
				returnVal.type = NoPL_DataType_Number;
				returnVal.numberValue = floorf(argv[0].numberValue);
			}
				break;
		}
	}
	
	return returnVal;
}
