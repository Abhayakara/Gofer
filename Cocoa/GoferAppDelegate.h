//
//  GoferAppDelegate.h
//  Gofer
//
//  Created by Ted Lemon on 5/1/11.
//  Copyright 2011 Edward W. Y. Lemon III. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface GoferAppDelegate : NSObject <NSApplicationDelegate, NSTableViewDataSource> {
@private
  NSWindow *window;

  NSTextField *inputBox0;
  NSTextField *inputBox1;
  NSTextField *inputBox2;
  NSTextField *inputBox3;
  NSTextField *inputBox4;
  NSTextField *inputBox5;
  NSTextField *inputBox6;
  NSTextField *inputBox7;
  
  NSTextField *distanceBox;
  NSMenu *distanceMenu;
  NSMenuItem *orMenuItem;
  NSMenuItem *andMenuItem;
  NSMenuItem *notMenuItem;
  NSMenuItem *distanceMenuItem;
  NSMenu *precisionMenu;
  NSMenuItem *ignoreSpaceCap;
  NSMenuItem *ignoreSpace;
  NSMenuItem *ignoreNothing;
  
  NSTextField *equationField;
  NSTableView *dirTable;
  NSTableColumn *checkboxColumn;
  NSTableColumn *dirnameColumn;
  
  NSMutableArray *dirNames;
  NSMutableArray *dirsChecked;
}

@property (assign) IBOutlet NSWindow *window;

@property(assign) IBOutlet NSTextField *inputBox0;
@property(assign) IBOutlet NSTextField *inputBox1;
@property(assign) IBOutlet NSTextField *inputBox2;
@property(assign) IBOutlet NSTextField *inputBox3;
@property(assign) IBOutlet NSTextField *inputBox4;
@property(assign) IBOutlet NSTextField *inputBox5;
@property(assign) IBOutlet NSTextField *inputBox6;
@property(assign) IBOutlet NSTextField *inputBox7;

@property(assign) IBOutlet NSTextField *distanceBox;

@property(assign) IBOutlet NSMenu *distanceMenu;
@property(assign) IBOutlet NSMenuItem *orMenuItem;
@property(assign) IBOutlet NSMenuItem *distanceMenuItem;
@property(assign) IBOutlet NSMenuItem *andMenuItem;
@property(assign) IBOutlet NSMenuItem *notMenuItem;


@property(assign) IBOutlet NSMenu *precisionMenu;
@property(assign) IBOutlet NSMenuItem *ignoreSpaceCap;
@property(assign) IBOutlet NSMenuItem *ignoreSpace;
@property(assign) IBOutlet NSMenuItem *ignoreNothing;

@property(assign) IBOutlet NSTextField *equationField;

@property(assign) IBOutlet NSTableView *dirTable;

- (IBAction)findClicked:(id)sender;
- (IBAction)saveClicked:(id)sender;
- (IBAction)addClicked:(id)sender;
- (IBAction)removeClicked:(id)sender;
- (IBAction)duplicateClicked:(id)sender;
- (IBAction)textAction:(id)sender;
- (IBAction)distanceAction:(id)sender;

@end

/* Local Variables:  */
/* mode:ObjC */
/* c-file-style:"gnu" */
/* end: */
