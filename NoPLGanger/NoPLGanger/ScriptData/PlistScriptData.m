//
//  PlistScriptData.m
//  NoPLGanger
//
//  Created by Brad Bambara on 10/18/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import "PlistScriptData.h"
#import "NSDictionary+NSDictionary_NoPL.h"

@implementation PlistScriptData

-(id)initWithPath:(NSString*)path
{
	if([[path pathExtension] compare:@"plist" options:NSCaseInsensitiveSearch] != NSOrderedSame)
		return nil;
	
	self = [super init];
	if(self)
	{
		originalPath = path;
		plistData = [NSDictionary dictionaryWithContentsOfFile:path];
	}
	return self;
}

-(NoPL_FunctionValue)callFunction:(void*)calledOnObject functionName:(NSString*)name args:(const NoPL_FunctionValue*)args argCount:(unsigned int)count
{
	//forward this call 
	if(!calledOnObject || calledOnObject == (__bridge void *)(plistData))
		return [plistData callFunction:calledOnObject functionName:name args:args argCount:count];
	return NoPL_FunctionValue();
}

-(NoPL_FunctionValue)getSubscript:(void*)calledOnObject index:(NoPL_FunctionValue)index
{
	//forward this call
	if(!calledOnObject || calledOnObject == (__bridge void *)(plistData))
		return [plistData getSubscript:calledOnObject index:index];
	return NoPL_FunctionValue();
}

@end
