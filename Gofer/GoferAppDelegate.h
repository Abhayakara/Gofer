//
//  GoferAppDelegate.h
//  Gofer
//
//  Created by Ted Lemon on 5/1/11.
//  Copyright 2011 Diamond Mountain University. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface GoferAppDelegate : NSObject <NSApplicationDelegate> {
@private
    NSWindow *window;
}

@property (assign) IBOutlet NSWindow *window;

@end
