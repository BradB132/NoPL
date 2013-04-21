//
//  main.c
//  NoPLc
//
//  Created by Brad Bambara on 4/15/12.
//  Copyright (c) 2012 Small Planet Digital. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NoPLc.h"

int main(int argc, const char * argv[])
{
	//check to make sure we have 2 paths specified
	if(argc < 2)
	{
		printf("NoPLc usage: [input source path] [optional: compiled output path]");
		return -1;
	}
	
	//create a compile context
	NoPL_CompileContext compileCtx = newNoPL_CompileContext();
	
	//set up the compile options
	NoPL_CompileOptions compileOpt = NoPL_CompileOptions();
	
	//compile the context
	compileContextWithFilePath(argv[1], &compileOpt, &compileCtx);
	
	//check for errors
	if(compileCtx.errDescriptions)
	{
		//display errors if there are any
		printf("NoPL Compile Errors:\n%s", compileCtx.errDescriptions);
	}
	else
	{
		//print token ranges if we have them
		for(int i = 0; i < NoPL_TokenRangeType_count; i++)
		{
			if(compileCtx.tokenRanges->counts[i] > 0)
			{
				printf("Type: %d: ", i);
				
				for(int j = 0; j < compileCtx.tokenRanges->counts[i]; j++)
				{
					printf("%ld-%ld, ", compileCtx.tokenRanges->ranges[i][j].startIndex, compileCtx.tokenRanges->ranges[i][j].endIndex);
				}
				
				printf("\n");
			}
		}
		
		//get a copy of the output path that we want
		char* outputPath = NULL;
		if(argc > 2)
		{
			outputPath = (char*)malloc(strlen(argv[2])+1);
			strcpy(outputPath, argv[2]);
		}
		else
		{
			//this path should have an extension of 'nopl', we want to convert that to 'noplb'
			long arg1Length = strlen(argv[1]);
			outputPath = (char*)malloc(arg1Length+2);
			strcpy((char*)outputPath, argv[1]);
			outputPath[arg1Length] = 'b';
			outputPath[arg1Length+1] = '\0';
		}
		
		//no errors, save the output to file
		FILE* file = fopen(outputPath, "wb");
		fwrite(compileCtx.compiledData, 1, compileCtx.dataLength, file);
		fclose(file);
		
		//clean up the string copy we did earlier
		free(outputPath);
	}
	
	//free the context
	freeNoPL_CompileContext(&compileCtx);
	
	return 0;
}

