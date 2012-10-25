//
//  DataManager.h
//  NoPLGanger
//
//  Created by Brad Bambara on 10/18/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "NoPLRuntime.h"

#define kNoPL_ConsoleOutputNotification @"NoPLConsoleOutput"
#define kNoPL_ConsoleOutputKey @"NoPLConsoleOutputKey"

@protocol DataContainer <NSObject>

-(NSString*)path;

-(NoPL_FunctionValue)callFunction:(void*)calledOnObject functionName:(NSString*)name args:(const NoPL_FunctionValue*)args argCount:(unsigned int)count;
-(NoPL_FunctionValue)getSubscript:(void*)calledOnObject index:(NoPL_FunctionValue)index;

@end

@interface DataManager : NSObject
{
	NSMutableArray* dataContainers;
}

+(DataManager*)sharedInstance;
+(NoPL_FunctionValue)objectToFunctionValue:(id)val;
+(NoPL_Callbacks)callbacks;

-(NSArray*)dataContainers;

-(void)addDataFromPath:(NSString*)path;
-(void)addDataObject:(id<DataContainer>)dataContainer;
-(void)removeDataForPath:(NSString*)path;
-(void)removeDataObject:(id<DataContainer>)dataContainer;

-(void)refreshAllContainers;

@end
