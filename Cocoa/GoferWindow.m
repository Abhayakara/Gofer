//
//  GoferWindow.m
//  Gofer
//
//  Created by Ted Lemon on 4/28/13.
//  Copyright 2013 Edward W. Y. Lemon III. All rights reserved.
//

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
/* c-file-style:"gnu" */
/* end: */
