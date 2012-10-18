//
//  XMLScriptData.m
//  NoPLGanger
//
//  Created by Brad Bambara on 10/18/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import "XMLScriptData.h"

@implementation XMLScriptData

-(id)initWithPath:(NSString*)path
{
	if([[path pathExtension] compare:@"xml" options:NSCaseInsensitiveSearch] != NSOrderedSame)
		return nil;
	
	self = [super init];
	if(self)
	{
		originalPath = path;
	}
	return self;
}


-(NoPL_FunctionValue)callFunction:(void*)calledOnObject functionName:(NSString*)name args:(const NoPL_FunctionValue*)args argCount:(unsigned int)count
{
	NoPL_FunctionValue returnVal = NoPL_FunctionValue();
	
	return returnVal;
}

-(NoPL_FunctionValue)getSubscript:(void*)calledOnObject index:(NoPL_FunctionValue)index
{
	NoPL_FunctionValue returnVal = NoPL_FunctionValue();
	
	return returnVal;
}

@end
