//
//  main.c
//  NoPLc
//
//  Created by Brad Bambara on 4/15/12.
//  Copyright (c) 2012 Small Planet Digital. All rights reserved.
//

#include <stdio.h>
#include "NoPLc.h"

int main(int argc, const char * argv[])
{
	//check to make sure we have 2 paths specified
	if(argc < 3)
	{
		printf("NoPLc requires 2 arguments: input source path and compiled output path");
		return -1;
	}
	
	//create a compile context
	NoPL_CompileContext compileCtx = newNoPL_CompileContext();
	
	//set up the compile options
	NoPL_CompileOptions compileOpt = NoPL_CompileOptions();
	compileOpt.createTokenRanges = 1;
	
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
		
		//no errors, save the output to file
		FILE* file = fopen(argv[2], "wb");
		fwrite(compileCtx.compiledData, 1, compileCtx.dataLength, file);
		fclose(file);
	}
	
	//free the context
	freeNoPL_CompileContext(&compileCtx);
	
	return 0;
}

