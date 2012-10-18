//
//  PlistScriptData.h
//  NoPLGanger
//
//  Created by Brad Bambara on 10/18/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "DataManager.h"
#import "ScriptData.h"

@interface PlistScriptData : ScriptData <DataContainer>
{
	NSDictionary* plistData;
}

-(id)initWithPath:(NSString*)path;

@end
