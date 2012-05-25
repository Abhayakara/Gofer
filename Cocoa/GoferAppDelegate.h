//
//  GoferAppDelegate.h
//  Gofer
//
//  Created by Ted Lemon on 5/1/11.
//  Copyright 2011 Edward W. Y. Lemon III. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "filelist.h"

@interface GoferAppDelegate : NSObject <NSApplicationDelegate, NSTableViewDataSource, NSTabViewDelegate> {
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
  
  NSPopUpButton *distancePopUp;
  NSTextField *distanceBox;
  NSMenu *distanceMenu;
  NSMenuItem *orMenuItem;
  NSMenuItem *andMenuItem;
  NSMenuItem *notMenuItem;
  NSMenuItem *distanceMenuItem;
  NSMenu *precisionMenu;
  NSPopUpButton *precisionPopUp;
  NSMenuItem *ignoreSpaceCap;
  NSMenuItem *ignoreSpace;
  NSMenuItem *ignoreNothing;
  
  NSTextField *equationField;
  NSTableView *dirTable;
  NSTableColumn *checkboxColumn;
  NSTableColumn *dirnameColumn;
  
  NSMutableArray *dirNames;
  NSMutableArray *dirsChecked;

  NSTabView *tabView;
  NSButton *nextMatchButton;
  NSButton *nextFileMatchButton;
  NSButton *prevMatchButton;
  NSButton *prevFileMatchButton;
  NSButton *stopButton;
  NSButton *findButton;
  NSMenuItem *findMenuItem;
  NSMenuItem *findNextMenuItem;
  NSMenuItem *findPreviousMenuItem;

  NSTextView *fileContentView;
  NSTextField *viewFile;
  NSTextField *statusMessageField;

  BOOL keepSearching;
  BOOL firstMatch;
  filelist_t *files;
  BOOL haveContents;

  char *curFileName;

  int filesMatched;
  int matchCount;
}

@property filelist_t *files;
@property BOOL keepSearching;
@property BOOL firstMatch;
@property BOOL haveContents;

@property int filesMatched;
@property int matchCount;

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

@property(assign) IBOutlet NSPopUpButton *distancePopUp;
@property(assign) IBOutlet NSMenu *distanceMenu;
@property(assign) IBOutlet NSMenuItem *orMenuItem;
@property(assign) IBOutlet NSMenuItem *distanceMenuItem;
@property(assign) IBOutlet NSMenuItem *andMenuItem;
@property(assign) IBOutlet NSMenuItem *notMenuItem;

@property(assign) IBOutlet NSTabView *tabView;
@property(assign) IBOutlet NSMenu *precisionMenu;
@property(assign) IBOutlet NSPopUpButton *precisionPopUp;
@property(assign) IBOutlet NSMenuItem *ignoreSpaceCap;
@property(assign) IBOutlet NSMenuItem *ignoreSpace;
@property(assign) IBOutlet NSMenuItem *ignoreNothing;

@property(assign) IBOutlet NSTextField *equationField;

@property(assign) IBOutlet NSTableView *dirTable;

@property(assign) IBOutlet NSButton *nextMatchButton;
@property(assign) IBOutlet NSButton *nextFileMatchButton;
@property(assign) IBOutlet NSButton *prevMatchButton;
@property(assign) IBOutlet NSButton *prevFileMatchButton;
@property(assign) IBOutlet NSButton *stopButton;
@property(assign) IBOutlet NSButton *findButton;

@property(assign) IBOutlet NSMenuItem *findMenuItem;
@property(assign) IBOutlet NSMenuItem *findNextMenuItem;
@property(assign) IBOutlet NSMenuItem *findPreviousMenuItem;

@property(assign) IBOutlet NSTextView *fileContentView;
@property(assign) IBOutlet NSTextField *viewFile;
@property(assign) IBOutlet NSTextField *statusMessageField;

- (IBAction)findClicked:(id)sender;
- (IBAction)saveClicked:(id)sender;
- (IBAction)addClicked:(id)sender;
- (IBAction)removeClicked:(id)sender;
- (IBAction)duplicateClicked:(id)sender;
- (IBAction)stopClicked:(id)sender;
- (IBAction)textAction:(id)sender;
- (IBAction)distanceAction:(id)sender;

- (IBAction)nextMatch: (id)sender;
- (IBAction)nextFileMatch: (id)sender;
- (IBAction)prevMatch: (id)sender;
- (IBAction)prevFileMatch: (id)sender;

- (IBAction)selectionToClipBoardWithInfo: (id)sender;

- (void)zapMatchView;
- (void)showCurMatch;
- (void)seekMatch;
- (void)unShowMatch;

- (void)setMatchButtonStates;
- (void)setNextMatchEnabled: (BOOL)state;
- (void)setPrevMatchEnabled: (BOOL)state;
- (void)setNextFileMatchEnabled: (BOOL)state;
- (void)setPrevFileMatchEnabled: (BOOL)state;
- (void)setStatusMessage: (NSString *)message;

- (void)writeDirDefaults;
@end

/* Local Variables:  */
/* mode:ObjC */
/* c-file-style:"gnu" */
/* end: */
