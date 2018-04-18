// -*- Mode: ObjC; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108 -*-
//
//  WinDelegate.m
//  Gofer
//
// Copyright (c) 2011-2014, 2018 Edward W. Lemon III

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

#import "WinDelegate.h"
#import "GoferAppDelegate.h"
#import "GoferUI.h"

@implementation WinDelegate

- (void)             window: (NSWindow *)window
   didDecodeRestorableState:(NSCoder *)state
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
/* end: */
