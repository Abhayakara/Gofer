//
//  GoferAppDelegate.m
//  Gofer
//
//  Created by Ted Lemon on 5/1/11.
//  Copyright 2011 Edward W. Y. Lemon III. All rights reserved.
//

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
	    char *contents, int content_length,
	    int first_line, int last_line,
	    int first_char, int first_len,
	    int last_char, int last_len,
	    int *cur_line, int *cur_char)
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
  int len = strlen(filename);
  if (len > 120)
    {
      s += (len - 120);
      len = 120;
    }
  NSString *sm = [[NSString alloc] initWithFormat: @"Searching: %s", s];

  printf("file %s\n", filename);

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
newWindow: (id)sender
{
  GoferUI *ui;

  ui = [[GoferUI alloc] init];
  if (ui)
    [uis addObject: ui];
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
      NSLog(@"Item %d: %@", ix, foo);
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
/* c-file-style:"gnu" */
/* end: */
