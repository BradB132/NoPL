//
//  NoPLScriptData.h
//  NoPLGanger
//
//  Created by Brad Bambara on 10/24/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "NoPLRuntime.h"
#import "DataManager.h"
#import "ScriptData.h"

@interface NoPLScriptData : ScriptData <DataContainer>
{
	NSMutableDictionary* numbers;
	NSMutableDictionary* booleans;
	NSMutableDictionary* pointers;
	NSMutableDictionary* strings;
	NoPL_DebugHandle handle;
}

-(id)initWithPath:(NSString *)path andHandle:(NoPL_DebugHandle)debugHandle;

-(void)addVariable:(NoPL_DataType)type name:(NSString*)varName index:(int)index;

@end
