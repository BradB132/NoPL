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
	NoPL_CompileOptions compileOpt;
	compileOpt.optimizeForRuntime = 1;
	compileOpt.debugSymbols = 0;
	
	//compile the context
	compileContextWithFilePath(argv[1], &compileOpt, &compileCtx);
	
	//save the output to file
	FILE* file = fopen(argv[2], "wb");
	fwrite(compileCtx.compiledData, 1, compileCtx.dataLength, file);
	fclose(file);
	
	//free the context
	freeNoPL_CompileContext(&compileCtx);
	
	return 0;
}

