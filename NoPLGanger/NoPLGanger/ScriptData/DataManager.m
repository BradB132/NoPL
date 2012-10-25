//
//  DataManager.m
//  NoPLGanger
//
//  Created by Brad Bambara on 10/18/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import "DataManager.h"
#import "XMLScriptData.h"
#import "PlistScriptData.h"
#import "FileBrowserController.h"
#import "NoPLStandardFunctions.h"

#define kDataManager_FileListKey @"DataManagerFileList"

static DataManager* instance;

#pragma mark - NoPL callback functions

NoPL_FunctionValue evalFunction(void* calledOnObject, const char* functionName, const NoPL_FunctionValue* argv, unsigned int argc)
{
	NSString* stringFunctionName = [NSString stringWithUTF8String:functionName];
	
	//check if our object responds directly to the function
	id obj = (__bridge id)calledOnObject;
	SEL functionSEL = @selector(callFunction:functionName:args:argCount:);
	if([obj respondsToSelector:functionSEL])
	{
		//the object does respond, call the function
		NoPL_FunctionValue val = [(id<DataContainer>)obj callFunction:calledOnObject functionName:stringFunctionName args:argv argCount:argc];
		if(val.type != NoPL_DataType_Uninitialized)
			return val;
	}
	
	//we're calling a global, start at any potential root objects
	if(!calledOnObject)
	{
		for(id<DataContainer> container in [instance dataContainers])
		{
			NoPL_FunctionValue val = [container callFunction:calledOnObject functionName:stringFunctionName args:argv argCount:argc];
			if(val.type != NoPL_DataType_Uninitialized)
				return val;
		}
		
		//we failed to find anything, check the standard functions
		NoPL_FunctionValue val = nopl_standardFunctions(calledOnObject, functionName, argv, argc);
		if(val.type != NoPL_DataType_Uninitialized)
			return val;
	}
	
	/* //opening up reflection this much is actually really dangerous
	if(obj)
	{
		SEL checkSel = NSSelectorFromString(stringFunctionName);
		NSMethodSignature* sig = [[obj class] instanceMethodSignatureForSelector:checkSel];
		NSInvocation* invocation = [NSInvocation invocationWithMethodSignature:sig];
		[invocation setSelector:checkSel];
		[invocation setTarget:obj];
		[invocation invoke];
		void* buffer = (void *)malloc([sig methodReturnLength]);
		[invocation getReturnValue:buffer];
		const char* returnType = [sig methodReturnType];
		if(!strcmp(returnType, "@"))
		{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
			id value = [obj performSelector:checkSel];
#pragma clang diagnostic pop
			return [DataManager objectToFunctionValue:value];
		}
		else if(!strcmp(returnType, "c"))
		{
			//this is a bool
			NoPL_FunctionValue returnVal;
			returnVal.booleanValue = *((BOOL*)buffer);
			returnVal.type = NoPL_DataType_Boolean;
			return returnVal;
		}
		else if(!strcmp(returnType, "f"))
		{
			//this is a number
			NoPL_FunctionValue returnVal;
			returnVal.numberValue = *((float*)buffer);
			returnVal.type = NoPL_DataType_Number;
			return returnVal;
		}
		else if(!strcmp(returnType, "Q"))
		{
			NSUInteger intVal = *((NSUInteger*)buffer);
			NoPL_FunctionValue returnVal;
			returnVal.numberValue = (float)intVal;
			returnVal.type = NoPL_DataType_Number;
			return returnVal;
		}
	}*/
	
	return NoPL_FunctionValue();
}

NoPL_FunctionValue evalSubscript(void* calledOnObject, NoPL_FunctionValue index)
{
	//check if our object responds directly to the function
	id obj = (__bridge id)calledOnObject;
	SEL functionSEL = @selector(getSubscript:index:);
	if([obj respondsToSelector:functionSEL])
	{
		//the object does respond, call the function
		return [(id<DataContainer>)obj getSubscript:calledOnObject index:index];
	}
	
	//we're calling a global, start at any potential root objects
	if(!calledOnObject)
	{
		for(id<DataContainer> container in [instance dataContainers])
		{
			NoPL_FunctionValue val = [container getSubscript:calledOnObject index:index];
			if(val.type != NoPL_DataType_Uninitialized)
				return val;
		}
	}
	
	//we failed to find anything, return a dummy value
	return NoPL_FunctionValue();
}

void printString(const char* string, NoPL_StringFeedbackType type)
{
	NSString* printedString = NULL;
	switch (type) {
		case NoPL_StringFeedbackType_PrintStatement:
			printedString = [NSString stringWithFormat:@"NoPL Print: %s", string];
			break;
		case NoPL_StringFeedbackType_DebugInfo:
			printedString = [NSString stringWithFormat:@"NoPL Debug: %s", string];
			break;
		case NoPL_StringFeedbackType_RuntimeError:
			printedString = [NSString stringWithFormat:@"NoPL Error: %s", string];
			break;
	}
	
	if(printedString)
	{
		NSDictionary* params = [NSDictionary dictionaryWithObject:printedString	forKey:kNoPL_ConsoleOutputKey];
		[[NSNotificationCenter defaultCenter] postNotificationName:kNoPL_ConsoleOutputNotification object:instance userInfo:params];
	}
}

#pragma mark - DataManager

@implementation DataManager

+(DataManager*)sharedInstance
{
	if(!instance)
		instance = [[DataManager alloc] init];
	return instance;
}

+(NoPL_Callbacks)callbacks
{
	//set up the callbacks struct
	NoPL_Callbacks callbacks;
	callbacks.evaluateFunction = &evalFunction;
	callbacks.subscript = &evalSubscript;
	callbacks.stringFeedback = &printString;
	
	return callbacks;
}

-(id)init
{
	self = [super init];
	if(self)
	{
		//start with a blank list of containers
		dataContainers = [NSMutableArray array];
		
		//attempt to recover the array from file
		if([[NSUserDefaults standardUserDefaults] arrayForKey:kDataManager_FileListKey])
		{
			//we have some containers, load them
			[self refreshAllContainers];
		}
		
		//listen for added files
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(dataFileWasAdded:) name:kFileBroswer_SelectedData object:NULL];
	}
	return self;
}

-(void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

-(void)dataFileWasAdded:(NSNotification*)note
{
	NSString* path = [[note userInfo] objectForKey:kFileBroswer_SelectedPathKey];
	[self addDataFromPath:path];
}

-(NSArray*)dataContainers
{
	return dataContainers;
}

-(void)refreshAllContainers
{
	//dump our old objects
	[dataContainers removeAllObjects];
	
	//load all the containers we should have
	NSArray* containerPaths = [[NSUserDefaults standardUserDefaults] arrayForKey:kDataManager_FileListKey];
	for(NSString* containerPath in containerPaths)
	{
		[self addDataFromPath:containerPath];
	}
}

-(void)addDataFromPath:(NSString*)path
{
	//prevent redundant additions
	for(id<DataContainer> container in dataContainers)
	{
		if([[container path] isEqualToString:path])
			return;
	}
	
	//create a new container depending on file type
	id<DataContainer> container = NULL;
	if([[path pathExtension] compare:@"xml" options:NSCaseInsensitiveSearch] == NSOrderedSame)
		container = [[XMLScriptData alloc] initWithPath:path];
	else if([[path pathExtension] compare:@"plist" options:NSCaseInsensitiveSearch] == NSOrderedSame)
		container = [[PlistScriptData alloc] initWithPath:path];
	
	//add the new container to persistent list
	if(container)
	{
		[dataContainers addObject:container];
		
		//check if this needs to be added to the list
		NSArray* containerPaths = [[NSUserDefaults standardUserDefaults] arrayForKey:kDataManager_FileListKey];
		if(![containerPaths containsObject:path])
		{
			NSMutableArray* mutablePaths = [containerPaths mutableCopy];
			[[NSUserDefaults standardUserDefaults] setObject:mutablePaths forKey:kDataManager_FileListKey];
		}
	}
}

-(void)removeDataForPath:(NSString*)path
{
	//remove container
	for(int i = ((int)[dataContainers count])-1; i >= 0; i--)
	{
		id<DataContainer> container = [dataContainers objectAtIndex:0];
		if([[container path] isEqualToString:path])
			[dataContainers removeObjectAtIndex:i];
	}
	
	//remove file from paths list
	NSMutableArray* containerPaths = [[[NSUserDefaults standardUserDefaults] arrayForKey:kDataManager_FileListKey] mutableCopy];
	[containerPaths removeObject:path];
	[[NSUserDefaults standardUserDefaults] setObject:containerPaths forKey:kDataManager_FileListKey];
}

-(void)addDataObject:(id<DataContainer>)dataContainer
{
	if([dataContainers containsObject:dataContainer])
		return;
	
	[dataContainers addObject:dataContainer];
}

-(void)removeDataObject:(id<DataContainer>)dataContainer
{
	[dataContainers removeObject:dataContainer];
}

+(NoPL_FunctionValue)objectToFunctionValue:(id)val
{
	if([val isKindOfClass:[NSString class]])
	{
		NoPL_FunctionValue returnVal;
		NoPL_assignString([(NSString*)val UTF8String], returnVal);
		returnVal.type = NoPL_DataType_String;
		return returnVal;
	}
	else if([val isKindOfClass:[NSNumber class]])
	{
		NSNumber* n = (NSNumber*)val;
		if (strcmp([n objCType], @encode(BOOL)) == 0)
		{
			//this number was originally a bool
			NoPL_FunctionValue returnVal;
			returnVal.booleanValue = [n boolValue];
			returnVal.type = NoPL_DataType_Boolean;
			return returnVal;
		}
		else
		{
			//this number was not initialized with boolean
			NoPL_FunctionValue returnVal;
			returnVal.numberValue = [n floatValue];
			returnVal.type = NoPL_DataType_Number;
			return returnVal;
		}
	}
	
	//this is not one of our primitives
	NoPL_FunctionValue returnVal;
	returnVal.pointerValue = (__bridge void *)(val);
	returnVal.type = NoPL_DataType_Pointer;
	return returnVal;
}

@end
