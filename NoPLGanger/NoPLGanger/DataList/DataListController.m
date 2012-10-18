//
//  DataListController.m
//  NoPLGanger
//
//  Created by Brad Bambara on 10/17/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import "DataListController.h"
#import "DataManager.h"
#import "FileBrowserController.h"

@implementation DataListController

-(void)awakeFromNib
{
	//make sure we have an instance of the data manager
	[DataManager sharedInstance];
	
	[dataTable setDataSource:self];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(dataFileWasAdded:) name:kFileBroswer_SelectedData object:NULL];
}

-(void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
	return [[[DataManager sharedInstance] dataContainers] count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
	id<DataContainer> container = [[[DataManager sharedInstance] dataContainers] objectAtIndex:row];
	NSString* path = [container path];
	
	NSString* returnedPath = [[path stringByDeletingLastPathComponent] lastPathComponent];
	returnedPath = [returnedPath stringByAppendingPathComponent:[path lastPathComponent]];
	
	return returnedPath;
}

-(void)dataFileWasAdded:(NSNotification*)note
{
	//reload the table, but make sure the data manager has had a chance to update first
	[dataTable performSelector:@selector(reloadData) withObject:NULL afterDelay:0];
}

@end
