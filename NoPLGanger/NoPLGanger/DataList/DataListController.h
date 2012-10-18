//
//  DataListController.h
//  NoPLGanger
//
//  Created by Brad Bambara on 10/17/12.
//  Copyright (c) 2012 Brad Bambara. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface DataListController : NSObject <NSTableViewDelegate, NSTableViewDataSource>
{
	IBOutlet NSTableView* dataTable;
}

@end
