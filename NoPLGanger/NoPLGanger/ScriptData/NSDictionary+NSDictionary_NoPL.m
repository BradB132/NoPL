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
	//only respond if the called object is self
	if(!calledOnObject || calledOnObject == (__bridge void *)(self))
	{
		//attempt to retreive the value and convert to function result
		id val = [self objectForKey:name];
		if(val)
			return [DataManager objectToFunctionValue:val];
		
		if([name isEqualToString:@"count"])
		{
			NoPL_FunctionValue returnVal;
			returnVal.numberValue = (float)[self count];
			returnVal.type = NoPL_DataType_Number;
			return returnVal;
		}
	}
	return NoPL_FunctionValue();
}

-(NoPL_FunctionValue)getSubscript:(void*)calledOnObject index:(NoPL_FunctionValue)index
{
	//only respond if the called object is self
	if((!calledOnObject || calledOnObject == (__bridge void *)(self))
	   && index.type == NoPL_DataType_String)
	{
		//attempt to retreive the value and convert to function result
		NSString* stringKey = [NSString stringWithUTF8String:index.stringValue];
		id val = [self objectForKey:stringKey];
		if(val)
			return [DataManager objectToFunctionValue:val];
	}
	return NoPL_FunctionValue();
}

@end
