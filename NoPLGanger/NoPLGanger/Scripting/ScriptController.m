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

#define kScriptController_CompileDelay 0.5

@implementation ScriptController

+(NSColor*)colorWithHexColorString:(NSString*)inColorString
{
    NSColor* result = nil;
    unsigned colorCode = 0;
    unsigned char redByte, greenByte, blueByte;
	
    if (nil != inColorString)
    {
		NSScanner* scanner = [NSScanner scannerWithString:inColorString];
		(void) [scanner scanHexInt:&colorCode]; // ignore error
    }
    redByte = (unsigned char)(colorCode >> 16);
    greenByte = (unsigned char)(colorCode >> 8);
    blueByte = (unsigned char)(colorCode); // masks off high bits
	
    result = [NSColor
			  colorWithCalibratedRed:(CGFloat)redByte / 0xff
			  green:(CGFloat)greenByte / 0xff
			  blue:(CGFloat)blueByte / 0xff
			  alpha:1.0];
	return result;
}

-(void)awakeFromNib
{
	[scriptView setDelegate:self];
	
	//create a list of colors from plist
	NSString* dataPath = [[NSBundle mainBundle] pathForResource:@"EditorData" ofType:@"plist"];
	NSArray* stringColors = [[NSDictionary dictionaryWithContentsOfFile:dataPath] objectForKey:@"TextColors"];
	NSMutableArray* newColors = [NSMutableArray arrayWithCapacity:[stringColors count]];
	for(NSString* strColor in stringColors)
	{
		[newColors addObject:[ScriptController colorWithHexColorString:strColor]];
	}
	colors = newColors;
	
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
	//format the command to always print the expression
	stringCommand = [stringCommand stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
	stringCommand = [NSString stringWithFormat:@"#%@;", stringCommand];
	
	//debug commands should be interpreted as script, attempt to compile
	NoPL_CompileContext ctx = newNoPL_CompileContext();
	NoPL_CompileOptions options = NoPL_CompileOptions();
	compileContextWithString([stringCommand UTF8String], &options, &ctx);
	
	//check if the compile succeded
	if(!ctx.errDescriptions)
	{
		//TODO: add current script as data if there is one
		
		//run the script
		NoPL_Callbacks callbacks = [DataManager callbacks];
		runScript(ctx.compiledData, ctx.dataLength, &callbacks);
	}
	else
	{
		//show the compile error in the console
		[self appendToConsole:@"Invalid debug statement."];
	}
	
	//cleanup
	freeNoPL_CompileContext(&ctx);
}

-(void)clearConsole
{
	[[consoleView textStorage] deleteCharactersInRange:NSMakeRange(0, [[consoleView textStorage] length])];
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
	//clear the console before we show anything for this new script
	[self clearConsole];
	
	//get the script from the text view
	NSString* script = [scriptView string];
	
	//set up objects for compilation
	NoPL_CompileContext ctx = newNoPL_CompileContext();
	NoPL_CompileOptions options = NoPL_CompileOptions();
	options.createTokenRanges = 1;
	
	//compile the script
	compileContextWithString([script UTF8String], &options, &ctx);
	
	//check if the compile succeded
	NSString* outputPath = NULL;
	if(!ctx.errDescriptions)
	{
		//find the path for the output file
		outputPath = [[currentFilePath stringByDeletingPathExtension] stringByAppendingPathExtension:@"noplb"];
		
		//save the script to file
		NSData* compiledData = [NSData dataWithBytes:ctx.compiledData length:ctx.dataLength];
		[compiledData writeToFile:outputPath atomically:YES];
		
		//evaluate colors for highlighting the sript
		[scriptView setTextColor:[NSColor blackColor]];
		for(int i = 0; i < NoPL_TokenRangeType_count; i++)
		{
			if(ctx.tokenRanges->counts[i] > 0)
			{
				for(int j = 0; j < ctx.tokenRanges->counts[i]; j++)
				{
					NoPL_TokenRange range = ctx.tokenRanges->ranges[i][j];
					[scriptView setTextColor:[colors objectAtIndex:i] range:NSMakeRange(range.startIndex, (range.endIndex-range.startIndex))];
				}
			}
		}
	}
	else
	{
		//show the compile error in the console
		[self appendToConsole:[NSString stringWithUTF8String:ctx.errDescriptions]];
	}
	
	freeNoPL_CompileContext(&ctx);
	
	return outputPath;
}

- (void)compileScriptFromTimer
{
	recompileTimer = NULL;
	
	[self compileScript];
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
	
	[self compileScript];
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
	if(!outputPath)
	{
		[self appendToConsole:@"Script Was not run because it did not compile successfully."];
		return;
	}
	
	//run the compiled script from file
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
	
	//get the string that the user entered
	NSString* command = [debugInputView stringValue];
	if([command isEqualToString:@""])
		return;
	
	//remove the command from the input view
	[debugInputView setStringValue:@""];
	
	//append the command to the console
	[self appendToConsole:command];
	
	//run the command
	[self processDebugCommand:command];
}

-(void)textDidChange:(NSNotification *)notification
{
	if(recompileTimer)
		[recompileTimer invalidate];
	recompileTimer = [NSTimer scheduledTimerWithTimeInterval:kScriptController_CompileDelay target:self selector:@selector(compileScriptFromTimer) userInfo:NULL repeats:NO];
}

@end
