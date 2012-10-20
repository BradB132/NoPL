//
//  ScriptController.m
//  NoPLGanger
//
//  Created by Brad Bambara on 10/19/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import "ScriptController.h"
#import "FileBrowserController.h"
#import "DataManager.h"
#import "NoPLRuntime.h"
#import "NoPLc.h"

@implementation ScriptController

-(void)awakeFromNib
{
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(scriptFileWasAdded:) name:kFileBroswer_SelectedScript object:NULL];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(scriptDidOutput:) name:kNoPL_ConsoleOutputNotification object:NULL];
}

-(void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Script logic

-(void)processDebugCommand:(NSString*)stringCommand
{
	NSLog(@"Run command: '%@'", stringCommand);
}

-(void)appendToConsole:(NSString*)output
{
	//apend the string to the console
	output = [NSString stringWithFormat:@"%@\n", output];
	NSAttributedString* attrCommand = [[NSAttributedString alloc] initWithString:output];
	NSTextStorage *storage = [consoleView textStorage];
	
	[storage beginEditing];
	[storage appendAttributedString:attrCommand];
	[storage endEditing];
}

-(NSString*)compileScript
{
	//get the script from the text view
	NSString* script = [scriptView string];
	
	//set up objects for compilation
	NoPL_CompileContext ctx = newNoPL_CompileContext();
	NoPL_CompileOptions options = NoPL_CompileOptions();
	
	//compile the script
	compileContextWithString([script UTF8String], &options, &ctx);
	
	//find the path for the output file
	NSString* outputPath = [[currentFilePath stringByDeletingPathExtension] stringByAppendingPathExtension:@"noplb"];
	
	//save the script to file
	NSData* compiledData = [NSData dataWithBytes:ctx.compiledData length:ctx.dataLength];
	[compiledData writeToFile:outputPath atomically:YES];
	
	return outputPath;
}

#pragma mark - Notifications

-(void)scriptFileWasAdded:(NSNotification*)note
{
	//TODO: save the old file
	
	//open the new file
	NSString* path = [[note userInfo] objectForKey:kFileBroswer_SelectedPathKey];
	NSError* err;
	NSString* pathContents = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:&err];
	if(!err)
	{
		currentFilePath = path;
		[scriptView setString:pathContents];
	}
}

-(void)scriptDidOutput:(NSNotification*)note
{
	NSString* appendedStr = [[note userInfo] objectForKey:kNoPL_ConsoleOutputKey];
	[self appendToConsole:appendedStr];
}

#pragma mark - IB functions

- (IBAction)buildRunClicked:(id)sender
{
	//compile the script and get the path
	NSString* outputPath = [self compileScript];
	
	//get the compiled script from file
	NSData* compiledData = [NSData dataWithContentsOfFile:outputPath];
	NoPL_Callbacks callbacks = [DataManager callbacks];
	runScript([compiledData bytes], (unsigned int)[compiledData length], &callbacks);
}

- (IBAction)buildClicked:(id)sender
{
	[self compileScript];
}

- (IBAction)continueClicked:(id)sender
{
	
}

- (IBAction)stepClicked:(id)sender
{
	
}

- (IBAction)stopClicked:(id)sender
{
	
}

- (IBAction)debugCommandEntered:(id)sender
{
	if(sender != debugInputView)
		return;
	
	//run the command
	NSString* command = [debugInputView stringValue];
	[self processDebugCommand:command];
	
	//remove the command from the input view
	[debugInputView setStringValue:@""];
	
	[self appendToConsole:command];
}

@end
