//
//  NSDictionary+NSDictionary_NoPL.m
//  NoPLGanger
//
//  Created by Brad Bambara on 10/18/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import "NSDictionary+NSDictionary_NoPL.h"
#import "DataManager.h"

@implementation NSDictionary (NSDictionary_NoPL)

-(NoPL_FunctionValue)callFunction:(void*)calledOnObject functionName:(NSString*)name args:(const NoPL_FunctionValue*)args argCount:(unsigned int)count
{
	if(count > 0)
		return NoPL_FunctionValue();
	
	//only respond if the called object is self
	if(!calledOnObject || calledOnObject == (__bridge void *)(self))
	{
		//attempt to retreive the value and convert to function result
		id val = [self objectForKey:name];
		if(val)
			return [DataManager objectToFunctionValue:val];
		
		if([name isEqualToString:@"size"] || [name isEqualToString:@"count"] || [name isEqualToString:@"length"])
		{
			NoPL_FunctionValue val;
			val.numberValue = [self count];
			val.type = NoPL_DataType_Number;
			return val;
		}
	}
	return NoPL_FunctionValue();
}

-(NoPL_FunctionValue)getSubscript:(void*)calledOnObject index:(NoPL_FunctionValue)index
{
	//only respond if the called object is self
	if(!calledOnObject || calledOnObject == (__bridge void *)(self))
	{
		//bail if the index is not a string
		if(index.type == NoPL_DataType_String)
		{
			//attempt to retreive the value and convert to function result
			id val = [self objectForKey:[NSString stringWithUTF8String:index.stringValue]];
			if(val)
				return [DataManager objectToFunctionValue:val];
		}
		else if(index.type == NoPL_DataType_Number)
		{
			//attempt to retreive the value and convert to function result
			int arrIndex = (int)index.numberValue;
			if(arrIndex < [self count] && arrIndex >= 0)
			{
				id val = [self objectForKey:[[self allKeys] objectAtIndex:arrIndex]];
				if(val)
					return [DataManager objectToFunctionValue:val];
			}
		}
	}
	return NoPL_FunctionValue();
}

@end
