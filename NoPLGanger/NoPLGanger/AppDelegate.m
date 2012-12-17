//
//  AppDelegate.m
//  NoPLGanger
//
//  Created by Brad Bambara on 12/15/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import "AppDelegate.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	// Insert code here to initialize your application
	[self.window makeKeyAndOrderFront:self];
	
	//this app can use fullscreen
	[self.window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
}

@end
