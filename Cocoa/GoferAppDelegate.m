// -*- Mode: ObjC; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108 -*-

//
//  GoferAppDelegate.m
//  Gofer
//
//  Created by Ted Lemon on 5/1/11.
//  Copyright 2011, 2018 Edward W. Y. Lemon III. All rights reserved.
//

// This file is part of GOFER.

// GOFER is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// GOFER is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with GOFER, in a file called COPYING; if not, write
// to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
// Boston, MA  02111-1307  USA

#import "GoferAppDelegate.h"
#import "GoferUI.h"
#import "st.h"
#import "filelist.h"

@implementation GoferAppDelegate

@synthesize separatorMark;
@synthesize winMenu1;
@synthesize winMenu2;
@synthesize winMenu3;
@synthesize winMenu4;
@synthesize winMenu5;
@synthesize winMenu6;
@synthesize winMenu7;
@synthesize winMenu8;
@synthesize winMenu9;
@synthesize winMenus;
@synthesize winMenu;

static NSMutableArray *uis;
BOOL uis_setup = NO;
GoferUI *topUI = nil;

static void reconf(CGDirectDisplayID display,
				   CGDisplayChangeSummaryFlags flags,
				   void *userInfo)
{
	printf("display %x flags %x\n", display, flags);
	if (!uis_setup)
    {
		printf("not ready to resize yet\n");
		return;
    }
	dispatch_async(dispatch_get_main_queue(), ^{
			printf("in async dispatch.\n");
			printf("%lu uis\n", [uis count]);
			[uis enumerateObjectsUsingBlock: ^(id obj, NSUInteger idx, BOOL *stop)
				 {
					 printf("calling constrainUIToDisplay\n");
					 GoferUI *me = obj;
					 [me constrainUIToDisplay];
				 }];
		});
}

void
one_element(void *obj, const char *fname,
			char *contents, off_t content_length,
			off_t first_line, off_t last_line,
			off_t first_char, off_t first_len,
			off_t last_char, off_t last_len,
			off_t *cur_line, off_t *cur_char)
{
	unsigned ui_index = (int)obj;
	if ([uis count] <= ui_index)
		return;
	GoferUI *me = [uis objectAtIndex: ui_index];
	new_entry([me files], fname, contents, content_length, first_line, last_line,
			  first_char, first_len, last_char, last_len, cur_line, cur_char);
	dispatch_async(dispatch_get_main_queue(), ^{
			[me newMatch];
		});   
}

int
one_file(void *obj, const char *filename)
{
	unsigned ui_index = (int)obj;
	if ([uis count] <= ui_index)
		return 1;
	GoferUI *me = [uis objectAtIndex: ui_index];
	const char *s = filename;
	size_t len = strlen(filename);
	if (len > 120)
    {
		s += (len - 120);
		len = 120;
    }
	NSString *sm = [[NSString alloc] initWithFormat: @"Searching: %s", s];

//  printf("file %s\n", filename);

	dispatch_async(dispatch_get_main_queue(), ^{
			[me newFileMatch];
			[me setStatusMessage: sm];
		});
	return me.keepSearching;
}

+ (int)ui_index: (id)ui
{
	int ui_index;

	for (ui_index = 0; ui_index < [uis count]; ui_index++)
    {
		if ([uis objectAtIndex: ui_index] == ui)
			break;
    }
	if (ui_index == [uis count])
		[uis addObject: ui];
	return ui_index;
}

+ (void)newTopUI: (id)ui
{
	[uis enumerateObjectsUsingBlock: ^(id obj, NSUInteger idx, BOOL *stop)
		 {
			 GoferUI *me = obj;
			 printf("UI %p win %p top %p\n", me, [me window], ui);
			 if ([me window] == ui)
				 topUI = me;
		 }];
	printf("newTopUI: %p\n", topUI);
}

- (IBAction)
bugReport: (id)sender
{
// XXX this has to marshall all of the files searched plus the search log and upload them or
// XXX maybe just make a tarball that can be sent via email

	// Each GoferUI sends events to the bug report collector
	// events include:
	//  - New Gofer UI
	//  - Search started (includes search terms and folder list)
	//  - Search process completed
	//  - New file being displayed
	//  - New match being displayed
	//
	// This is all maintained (in memory|on disk?) until a bug report is requested.
	// At that time, we collect all of the files from which results were shown, along
	// with all of the logged data, and either upload it to a web page, or else marshall
	// it into a tar file, which can then be attached to an email message.
}  

- (IBAction)
newWindow: (id)sender
{
	GoferUI *ui;
    
	ui = [[GoferUI alloc] init];
	if (ui != nil)
		[uis addObject: ui];
	[ui setAppDelegate: self];
	[uis enumerateObjectsUsingBlock: ^(id obj, NSUInteger idx, BOOL *stop)
		 {
			 GoferUI *me = obj;
			 printf("UI %p\n", me);
		 }];
	NSWindow *win = [ui window];
	printf("Window: %p\n", win);
	[win setExcludedFromWindowsMenu: YES];
	[win makeKeyAndOrderFront: self];
	NSInteger ix = [uis count] - 1;
	NSInteger baseLocation = [winMenu indexOfItem: separatorMark];
	printf("winMenus: %lu ix %lu atIndex: %lu\n",
		   [winMenus count], ix, baseLocation);
	[winMenu insertItem: [winMenus objectAtIndex: ix]
				atIndex: baseLocation];
	for (ix = 0; ix < [winMenu numberOfItems]; ix++)
    {
		NSMenuItem *foo = [winMenu itemAtIndex: ix];
		NSLog(@"Item %ld: %@", (long)ix, foo);
    }
}

- (void)close: (id)sender
{
	NSInteger ix;

	for (ix = 0; ix < [uis count]; ix++)
    {
		GoferUI *ui = [uis objectAtIndex: ix];
		if (ui == topUI)
		{
			topUI = nil;
			[uis removeObjectAtIndex: ix];
			NSInteger baseLocation = [winMenu indexOfItem: separatorMark];
			[winMenu removeItemAtIndex: baseLocation - 1];
			[ui startClose];
			break;
		}
    }
	if ([uis count] == 0)
		[self newWindow: self];
}

- (void)
applicationDidFinishLaunching:(NSNotification *)aNotification
{
}

- (void)awakeFromNib
{
	[super awakeFromNib];

	if (!uis_setup)
    {
		uis = [NSMutableArray new];
		uis_setup = YES;
    }
	winMenus = [NSArray arrayWithObjects: winMenu1, winMenu2, winMenu3,
						winMenu4, winMenu5, winMenu6, winMenu7, winMenu8,
						winMenu9, nil];
#if 0
	[winMenus enumerateObjectsUsingBlock: ^(id obj, NSUInteger idx, BOOL *stop)
			  {
				  NSMenuItem *menuItem = obj;
				  [winMenu removeItem: menuItem];
			  }];
#else
	[winMenu removeItem: winMenu1];
#endif  
	[self newWindow: nil];

	CGError foo = CGDisplayRegisterReconfigurationCallback(reconf, 0);
	if (foo != kCGErrorSuccess)
		printf("Error registering display reconfiguration callback: %d\n", foo);
}

- (IBAction)findClicked:(id)sender
{
	if (topUI != nil)
		[topUI findClicked: sender];
}

- (IBAction)nextMatch: (id)sender
{
	if (topUI != nil)
		[topUI nextMatch: sender];
}

- (IBAction)prevMatch: (id)sender
{
	if (topUI != nil)
		[topUI prevMatch: sender];
}

- (IBAction)copyWithInfo: (id)sender
{
	if (topUI != nil)
		[topUI selectionToClipBoardWithInfo: sender];
}

- (IBAction)winSwitchClicked: (id)sender
{
	NSInteger ix = [winMenus indexOfObject: sender];
	if (ix == NSNotFound)
    {
		[winMenus enumerateObjectsUsingBlock:
					  ^(id obj, NSUInteger idx, BOOL *stop)
				  {
					  NSMenuItem *menuItem = obj;
					  printf("winSwitchClicked: %p %p\n", menuItem, sender);
				  }];
    }
	else
    {
		if (ix < [uis count])
		{
			GoferUI *ui = [uis objectAtIndex: ix];
			[[ui window] makeKeyAndOrderFront: self];
		}
    }
}

@end

/* Local Variables:  */
/* mode:ObjC */
/* end: */
