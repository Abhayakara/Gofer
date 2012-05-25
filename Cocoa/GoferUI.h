//
//  GoferUI.h
//  Gofer
//
//  Created by Ted Lemon on 6/19/11.
//  Copyright 2011 Edward W. Y. Lemon III. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface GoferUI : NSObject {
    NSTextField *inputBox0;
    NSTextField *inputBox1;
    NSTextField *inputBox2;
    NSTextField *inputBox3;
    NSTextField *inputBox4;
    NSTextField *inputBox5;
    NSTextField *inputBox6;
    NSTextField *inputBox7;
}

@property(assign) IBOutlet NSTextField *inputBox0;
@property(assign) IBOutlet NSTextField *inputBox1;
@property(assign) IBOutlet NSTextField *inputBox2;
@property(assign) IBOutlet NSTextField *inputBox3;
@property(assign) IBOutlet NSTextField *inputBox4;
@property(assign) IBOutlet NSTextField *inputBox5;
@property(assign) IBOutlet NSTextField *inputBox6;
@property(assign) IBOutlet NSTextField *inputBox7;

@end

/* Local Variables:  */
/* mode:ObjC */
/* c-file-style:"gnu" */
/* end: */
