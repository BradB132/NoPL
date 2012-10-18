//
//  ScriptData.m
//  NoPLGanger
//
//  Created by Brad Bambara on 10/18/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import "ScriptData.h"

@implementation ScriptData

-(id)initWithPath:(NSString*)path
{
	self = [super init];
	if(self)
	{
		originalPath = path;
	}
	return self;
}

-(NSString*)path
{
	return originalPath;
}

@end
