//
//  NoPLScriptData.m
//  NoPLGanger
//
//  Created by Brad Bambara on 10/24/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import "NoPLScriptData.h"

@implementation NoPLScriptData

-(id)initWithPath:(NSString *)path andHandle:(NoPL_DebugHandle)debugHandle
{
	self = [super initWithPath:path];
	if(self)
	{
		handle = debugHandle;
		numbers = [NSMutableDictionary dictionary];
		booleans = [NSMutableDictionary dictionary];
		pointers = [NSMutableDictionary dictionary];
		strings = [NSMutableDictionary dictionary];
	}
	return self;
}

-(void)addVariable:(NoPL_DataType)type name:(NSString*)varName index:(int)index
{
	NSMutableDictionary* dict = NULL;
	switch (type)
	{
		case NoPL_DataType_Boolean:
			dict = booleans;
			break;
		case NoPL_DataType_Number:
			dict = numbers;
			break;
		case NoPL_DataType_Pointer:
			dict = pointers;
			break;
		case NoPL_DataType_String:
			dict = strings;
			break;
		default:
			break;
	}
	
	if(dict)
		[dict setObject:[NSNumber numberWithInt:index] forKey:varName];
}

-(NoPL_FunctionValue)callFunction:(void*)calledOnObject functionName:(NSString*)name args:(const NoPL_FunctionValue*)args argCount:(unsigned int)count
{
	if(calledOnObject || count > 0)
		return NoPL_FunctionValue();
	
	//resolve the index
	NoPL_DataType type = NoPL_DataType_Number;
	NSNumber* obj = [numbers objectForKey:name];
	if(!obj)
	{
		type = NoPL_DataType_Boolean;
		obj = [booleans objectForKey:name];
		if(!obj)
		{
			type = NoPL_DataType_Pointer;
			obj = [pointers objectForKey:name];
			if(!obj)
			{
				type = NoPL_DataType_String;
				obj = [strings objectForKey:name];
			}
			
			if(!obj)
				return NoPL_FunctionValue();
		}
	}
	
	int index = [obj intValue];
	return queryValue(handle, type, index);
}

-(NoPL_FunctionValue)getSubscript:(void*)calledOnObject index:(NoPL_FunctionValue)index
{
	return NoPL_FunctionValue();
}

@end
