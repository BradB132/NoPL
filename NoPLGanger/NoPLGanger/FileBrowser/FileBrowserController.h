//
//  FileBrowserController.h
//  NoPLGanger
//
//  Created by Brad Bambara on 10/7/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import <Foundation/Foundation.h>

#define kFileBroswer_SelectedScript @"FileBrowserControllerSelectedScript"
#define kFileBroswer_SelectedData @"FileBrowserControllerSelectedData"
#define kFileBroswer_SelectedPathKey @"path"

@class FileSystemNode;

@interface FileBrowserController : NSObject <NSBrowserDelegate>
{
	IBOutlet NSBrowser* _browser;
    FileSystemNode* _rootNode;
	NSCell* clickedCell;
	NSTimeInterval timeClicked;
}

- (IBAction)browserClicked:(id)sender;

@end
