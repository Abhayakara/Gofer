// -*- Mode: ObjC; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108 -*-
//
//  GoferWindow.m
//  Gofer
//
//  Created by Ted Lemon on 4/28/13.
//  Copyright 2013, 2018 Edward W. Y. Lemon III. All rights reserved.
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

#import "GoferUI.h"
#import "GoferWindow.h"

@implementation GoferWindow
@synthesize ui;

- (void)sendEvent:(NSEvent *)nsevent
{
	CGEventRef event = [nsevent CGEvent];
	CGEventType type = CGEventGetType(event);
	CGEventFlags flags = CGEventGetFlags(event);
	if (type == kCGEventKeyDown || type == kCGEventKeyUp)
    {
		int64_t keycode = CGEventGetIntegerValueField(event,
													  kCGKeyboardEventKeycode);
		if (!((flags & kCGEventFlagMaskSecondaryFn) &&
			  (flags & kCGEventFlagMaskNumericPad)))
		{
			if (keycode == 0x24)
			{
				if (type == kCGEventKeyDown)
                {
					if (flags & kCGEventFlagMaskCommand)
						[ui commandRet: self];
					else
						[ui returnHit: self];
                }
				printf("Return %s.\n",
					   (type == kCGEventKeyDown) ? "pressed" : "released");
				return;
			}
			else if (keycode == 0x33)
			{
				if ((type == kCGEventKeyDown) && [ui deleteHit: self])
					return;
			}
		}
    }
	[super sendEvent: nsevent];
}
@end

/* Local Variables:  */
/* mode:ObjC */
/* end: */
