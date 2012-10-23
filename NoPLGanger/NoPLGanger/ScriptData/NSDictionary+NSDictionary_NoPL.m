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
	}
	return NoPL_FunctionValue();
}

-(NoPL_FunctionValue)getSubscript:(void*)calledOnObject index:(NoPL_FunctionValue)index
{
	//bail if the index is not a string
	if(index.type != NoPL_DataType_String)
		return NoPL_FunctionValue();
	
	//return the index in the same way that a function would
	NSString* stringKey = [NSString stringWithUTF8String:index.stringValue];
	return [self callFunction:calledOnObject functionName:stringKey args:NULL argCount:0];
}

@end
