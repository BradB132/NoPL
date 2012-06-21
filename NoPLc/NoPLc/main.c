//
//  main.c
//  NoPLc
//
//  Created by Brad Bambara on 4/15/12.
//  Copyright (c) 2012 Small Planet Digital. All rights reserved.
//

#include <stdio.h>

int main(int argc, const char * argv[])
{
	//check to make sure we have 2 paths specified
	if(argc < 3)
	{
		printf("NoPLc requires 2 arguments: input source path and compiled output path");
		return -1;
	}
	
//	pANTLR3_INPUT_STREAM inputStream = antlr3AsciiFileStreamNew((pANTLR3_UINT8)argv[1]);
//	if(!inputStream)
//	{
//		printf("NoPLc was not able to read input file at path: '%s'", argv[1]);
//		return -1;
//	}
	//antlr3readAscii(pANTLR3_INPUT_STREAM input, pANTLR3_UINT8 fileName);
	
}

