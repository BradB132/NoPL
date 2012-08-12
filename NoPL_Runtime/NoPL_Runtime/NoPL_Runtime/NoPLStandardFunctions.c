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

#pragma mark -
#pragma mark fast hash for looking up function names

unsigned long hash_djb2ToInt(unsigned char* str)
{
	unsigned long hash = 5381;
	int c;
	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;
	return (int)(hash % 0xFFFFFF);
}

#pragma mark -
#pragma mark standard functions

NoPL_FunctionValue nopl_standardFunctions(void* calledOnObject, char* functionName, NoPL_FunctionValue* argv, unsigned int argc)
{
	//create a blank function result
	NoPL_FunctionValue returnVal = NoPL_FunctionValue();
	
	//bail if this call isn't on a global function
	if(calledOnObject)
		return returnVal;
	
	//use hashed names to quickly look up the correct function
	switch (hash_djb2ToInt((unsigned char*)functionName))
	{
		case 15501227://format
		{
			//make sure the args start with a string
			if(argc < 1 || argv[0].type != NoPL_DataType_String)
				break;
			
			//TODO: figure out how to do this
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
			returnVal.numberValue = (float)hash_djb2ToInt((unsigned char*)argv[0].stringValue);
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
			//bail if we don't have the correct args
			if(argc < 2 || argc > 3 || argv[0].type != NoPL_DataType_String || argv[1].type != NoPL_DataType_Number || (argc == 3 && argv[2].type != NoPL_DataType_Number))
				break;
			
			//get lengths and indices
			int strLength = (int)strlen(argv[0].stringValue);
			int startIndex = (int)(argv[1].numberValue);
			int substrLength;
			if(startIndex < 0)
				startIndex = 0;
			if(startIndex >= strLength)
				substrLength = 0;
			else if(argc == 2)
			{
				//arg count is 2, subtract to include the rest of the string
				substrLength = strLength-startIndex;
			}
			else
			{
				//arg count is 3, so we have a length for the string
				substrLength = (int)(argv[2].numberValue);
				
				//make sure we're not going past the end of the string
				if(startIndex+substrLength > strLength)
					substrLength = strLength-startIndex;
			}
			
			//copy the string
			returnVal.type = NoPL_DataType_String;
			returnVal.stringValue = malloc(substrLength+1);
			strncpy(returnVal.stringValue, argv[0].stringValue+startIndex, substrLength);
			returnVal.stringValue[substrLength] = 0;
		}
			break;
		case 1421073://indexOf
		{
			//bail if we don't have two string arguments
			if(argc != 2 || argv[0].type != NoPL_DataType_String || argv[1].type != NoPL_DataType_String)
				break;
			
			//search the string
			char* found = strstr(argv[0].stringValue, argv[1].stringValue);
			
			returnVal.type = NoPL_DataType_Number;
			if(!found)
				returnVal.numberValue = -1;
			else
				returnVal.numberValue = found-argv[0].stringValue;
		}
			break;
		case 1452132://replaceAll
		{
			if(argc != 2 || argv[0].type != NoPL_DataType_String || argv[1].type != NoPL_DataType_String)
				break;
			
			//TODO: implement replace
		}
			break;
		case 11849711://replaceFirst
		{
			if(argc != 2 || argv[0].type != NoPL_DataType_String || argv[1].type != NoPL_DataType_String)
				break;
			
			//TODO: implement replaceFirst
		}
			break;
		case 13832097://replaceLast
		{
			if(argc != 2 || argv[0].type != NoPL_DataType_String || argv[1].type != NoPL_DataType_String)
				break;
			
			//TODO: implement replaceLast
		}
			break;
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
	
	return returnVal;
}
