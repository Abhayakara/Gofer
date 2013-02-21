//
//  WinDelegate.m
//  Gofer
//
//  Copyright 2013 Edward W. Y. Lemon III. All rights reserved.
//

#import "WinDelegate.h"
#import "GoferAppDelegate.h"
#import "GoferUI.h"

@implementation WinDelegate

- (void)             window: (NSWindow *)window
   didDecodeRestorableState:(NSCoder *)state
{
}

- (NSSize)                                window: (NSWindow *)window
 willResizeForVersionBrowserWithMaxPreferredSize: (NSSize)maxPreferredSize
				  maxAllowedSize: (NSSize)maxAllowedSize
{
}

- (void)windowDidEnterVersionBrowser: (NSNotification *)notification
{
}

- (void)windowDidExitVersionBrowser: (NSNotification *)notification
{
}

- (void)windowWillEnterVersionBrowser: (NSNotification *)notification
{
}

- (void)windowWillExitVersionBrowser: (NSNotification *)notification
{
}

- (void)windowDidBecomeMain: (NSNotification *)notification
{
  [GoferAppDelegate newTopUI: [notification object]];
}

  @end

/* Local Variables:  */
/* mode:ObjC */
/* c-file-style:"gnu" */
/* end: */
