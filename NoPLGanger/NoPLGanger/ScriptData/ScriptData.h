//
//  ScriptData.h
//  NoPLGanger
//
//  Created by Brad Bambara on 10/18/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ScriptData : NSObject
{
	NSString* originalPath;
}

-(id)initWithPath:(NSString*)path;
-(NSString*)path;

@end
