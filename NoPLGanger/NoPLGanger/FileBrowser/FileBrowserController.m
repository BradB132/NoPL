//
//  FileBrowserController.m
//  NoPLGanger
//
//  Created by Brad Bambara on 10/7/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import "FileBrowserController.h"
#import "FileSystemNode.h"

#define kFileBrowser_CurentDirectoryKey @"FileBrowserDirectory"
#define kFileBrowser_DoubleClickTime 0.5
#define kFileBrowser_DefaultRootPath @"/"

@implementation FileBrowserController

#pragma mark - misc

-(void)updateBacktrack
{
	NSURL* url = [NSURL URLWithString:@"/"];
	_rootNode.allowBacktrack = ![[_rootNode.URL absoluteString] isEqualToString:[url absoluteString]];
}

#pragma mark - IB

-(void)awakeFromNib
{
	if(![[NSUserDefaults standardUserDefaults] stringForKey:kFileBrowser_CurentDirectoryKey])
		[[NSUserDefaults standardUserDefaults] setObject:kFileBrowser_DefaultRootPath forKey:kFileBrowser_CurentDirectoryKey];
	
	NSString* savedRootPath = [[NSUserDefaults standardUserDefaults] stringForKey:kFileBrowser_CurentDirectoryKey];
	_rootNode = [[FileSystemNode alloc] initWithURL:[NSURL fileURLWithPath:savedRootPath]];
	[self updateBacktrack];
	
	timeClicked = 0.0;
	clickedCell = nil;
	
	[_browser setDelegate:self];
}

#pragma mark - NSBrowserDelegate

- (id)rootItemForBrowser:(NSBrowser *)browser
{
	return _rootNode;
}

- (NSInteger)browser:(NSBrowser *)browser numberOfChildrenOfItem:(id)item {
    FileSystemNode *node = (FileSystemNode *)item;
    return node.children.count;
}

- (id)browser:(NSBrowser *)browser child:(NSInteger)index ofItem:(id)item {
    FileSystemNode *node = (FileSystemNode *)item;
    return [node.children objectAtIndex:index];
}

- (BOOL)browser:(NSBrowser *)browser isLeafItem:(id)item {
    FileSystemNode *node = (FileSystemNode *)item;
    return !node.isDirectory;
}

- (id)browser:(NSBrowser *)browser objectValueForItem:(id)item {
    FileSystemNode *node = (FileSystemNode *)item;
    return node.displayName;
}

- (IBAction)browserClicked:(id)sender
{
	NSBrowser* browser = sender;
	
	//check if a cell was double clicked
	if(clickedCell && clickedCell == [browser selectedCell] &&
	   ([[NSDate date] timeIntervalSince1970] - timeClicked) <= kFileBrowser_DoubleClickTime)
	{
		//we have a double click
		NSString* rootPath = [[NSUserDefaults standardUserDefaults] stringForKey:kFileBrowser_CurentDirectoryKey];
		NSString* absolutePath = [rootPath stringByAppendingPathComponent:[browser path]];
		
		//check what this path is pointing to
		if([[absolutePath pathExtension] compare:@"nopl" options:NSCaseInsensitiveSearch] == NSOrderedSame)
		{
			//this is a script file
			[[NSNotificationCenter defaultCenter] postNotificationName:kFileBroswer_SelectedScript object:self userInfo:[NSDictionary dictionaryWithObject:absolutePath forKey:kFileBroswer_SelectedPathKey]];
		}
		else if([[absolutePath pathExtension] compare:@"xml" options:NSCaseInsensitiveSearch] == NSOrderedSame ||
				[[absolutePath pathExtension] compare:@"plist" options:NSCaseInsensitiveSearch] == NSOrderedSame)
		{
			//this is a data file
			[[NSNotificationCenter defaultCenter] postNotificationName:kFileBroswer_SelectedData object:self userInfo:[NSDictionary dictionaryWithObject:absolutePath forKey:kFileBroswer_SelectedPathKey]];
		}
		else
		{
			//check if this path is a directory
			BOOL isDirectory;
			[[NSFileManager defaultManager] fileExistsAtPath:absolutePath isDirectory:&isDirectory];
			if(isDirectory)
			{
				//if this is backtracking up a directory, remove some of the path components
				if([[absolutePath lastPathComponent] isEqualToString:@".."])
				{
					absolutePath = [absolutePath stringByDeletingLastPathComponent];
					absolutePath = [absolutePath stringByDeletingLastPathComponent];
				}
				
				//we've double clicked a directory, switch to that directory
				[[NSUserDefaults standardUserDefaults] setObject:absolutePath forKey:kFileBrowser_CurentDirectoryKey];
				_rootNode.URL = [NSURL URLWithString:absolutePath];
				[self updateBacktrack];
				[_rootNode invalidateChildren];
				
				//update the browser with the new path
				[browser reloadColumn:0];
			}
		}
		
		//reset the click attributes
		clickedCell = nil;
		timeClicked = 0.0;
	}
	else
	{
		//not a double click, reset all values
		clickedCell = [browser selectedCell];
		timeClicked = [[NSDate date] timeIntervalSince1970];
	}
}


@end
