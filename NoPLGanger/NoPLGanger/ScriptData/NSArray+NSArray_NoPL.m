//
//  NSArray+NSArray_NoPL.m
//  NoPLGanger
//
//  Created by Brad Bambara on 10/18/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import "NSArray+NSArray_NoPL.h"
#import "DataManager.h"

@implementation NSArray (NSArray_NoPL)

-(NoPL_FunctionValue)callFunction:(void*)calledOnObject functionName:(NSString*)name args:(const NoPL_FunctionValue*)args argCount:(unsigned int)count
{
	if(count == 1 && args[0].type == NoPL_DataType_Number
	   && ([name isEqualToString:@"get"] || [name isEqualToString:@"objectAtIndex"]))
	{
		return [self getSubscript:calledOnObject index:args[0]];
	}
	else if(count == 0 && ([name isEqualToString:@"size"] || [name isEqualToString:@"count"] || [name isEqualToString:@"length"]))
	{
		NoPL_FunctionValue val;
		val.numberValue = [self count];
		val.type = NoPL_DataType_Number;
		return val;
	}
	
	return NoPL_FunctionValue();
}

-(NoPL_FunctionValue)getSubscript:(void*)calledOnObject index:(NoPL_FunctionValue)index
{
	//only respond if the called object is self
	if((!calledOnObject || calledOnObject == (__bridge void *)(self))
	   && index.type == NoPL_DataType_Number)
	{
		//attempt to retreive the value and convert to function result
		int arrIndex = (int)index.numberValue;
		if(arrIndex < [self count] && arrIndex >= 0)
		{
			id val = [self objectAtIndex:arrIndex];
			if(val)
				return [DataManager objectToFunctionValue:val];
		}
	}
	return NoPL_FunctionValue();
}

@end
