//
//  ScriptController.h
//  NoPLGanger
//
//  Created by Brad Bambara on 10/19/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ScriptController : NSObject <NSTextViewDelegate>
{
	IBOutlet NSTextView* scriptView;
	IBOutlet NSTextView* consoleView;
	IBOutlet NSTextField* debugInputView;
	IBOutlet NSButton* buildRunBtn;
	IBOutlet NSButton* buildBtn;
	IBOutlet NSButton* continueBtn;
	IBOutlet NSButton* stepBtn;
	IBOutlet NSButton* stopBtn;
	
	NSArray* colors;
	
	NSTimer* recompileTimer;
	
	NSString* currentFilePath;
}
@end
