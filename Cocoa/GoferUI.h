//
//  GoferAppDelegate.h
//  Gofer
//
//  Created by Ted Lemon on 5/1/11.
//  Copyright 2011 Edward W. Y. Lemon III. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "filelist.h"

@interface GoferUI : NSWindowController <NSTableViewDataSource,
					 NSTabViewDelegate> {
@private
  NSWindow *win;

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
  NSTabViewItem *searchSettingTab;
  NSTabViewItem *savedSearchTab;
  NSTabViewItem *searchResultsTab;
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

  BOOL searching;
  BOOL keepSearching;
  BOOL firstMatch;
  filelist_t *files;
  BOOL haveContents;

  char *curFileName;

  int filesMatched;
  int matchCount;
  bool initialized;
}

@property filelist_t *files;
@property BOOL closing;
@property BOOL searching;
@property BOOL keepSearching;
@property BOOL firstMatch;
@property BOOL haveContents;

@property int filesMatched;
@property int matchCount;

@property (retain) IBOutlet NSWindow *win;

@property(retain) IBOutlet NSTextField *inputBox0;
@property(retain) IBOutlet NSTextField *inputBox1;
@property(retain) IBOutlet NSTextField *inputBox2;
@property(retain) IBOutlet NSTextField *inputBox3;
@property(retain) IBOutlet NSTextField *inputBox4;
@property(retain) IBOutlet NSTextField *inputBox5;
@property(retain) IBOutlet NSTextField *inputBox6;
@property(retain) IBOutlet NSTextField *inputBox7;

@property(retain) IBOutlet NSTextField *distanceBox;

@property(retain) IBOutlet NSPopUpButton *distancePopUp;
@property(retain) IBOutlet NSMenu *distanceMenu;
@property(retain) IBOutlet NSMenuItem *orMenuItem;
@property(retain) IBOutlet NSMenuItem *distanceMenuItem;
@property(retain) IBOutlet NSMenuItem *andMenuItem;
@property(retain) IBOutlet NSMenuItem *notMenuItem;

@property(retain) IBOutlet NSTabView *tabView;
@property(retain) IBOutlet NSTabViewItem *searchSettingTab;
@property(retain) IBOutlet NSTabViewItem *savedSearchTab;
@property(retain) IBOutlet NSTabViewItem *searchResultsTab;

@property(retain) IBOutlet NSMenu *precisionMenu;
@property(retain) IBOutlet NSPopUpButton *precisionPopUp;
@property(retain) IBOutlet NSMenuItem *ignoreSpaceCap;
@property(retain) IBOutlet NSMenuItem *ignoreSpace;
@property(retain) IBOutlet NSMenuItem *ignoreNothing;

@property(retain) IBOutlet NSTextField *equationField;

@property(retain) IBOutlet NSTableView *dirTable;

@property(retain) IBOutlet NSButton *nextMatchButton;
@property(retain) IBOutlet NSButton *nextFileMatchButton;
@property(retain) IBOutlet NSButton *prevMatchButton;
@property(retain) IBOutlet NSButton *prevFileMatchButton;
@property(retain) IBOutlet NSButton *stopButton;
@property(retain) IBOutlet NSButton *findButton;

@property(retain) IBOutlet NSMenuItem *findMenuItem;
@property(retain) IBOutlet NSMenuItem *findNextMenuItem;
@property(retain) IBOutlet NSMenuItem *findPreviousMenuItem;

@property(retain) IBOutlet NSTextView *fileContentView;
@property(retain) IBOutlet NSTextField *viewFile;
@property(retain) IBOutlet NSTextField *statusMessageField;

- (filelist_t *)files;
- (void)newMatch;
- (void)newFileMatch;
- (void)startClose;
- (void)finishClosing;
- (void)constrainUIToDisplay;
- (bool)deleteHit:(id)sender;
- (void)returnHit:(id)sender;
- (void)commandRet:(id)sender;
- (IBAction)findClicked:(id)sender;
- (IBAction)saveClicked:(id)sender;
- (IBAction)addClicked:(id)sender;
- (IBAction)removeClicked:(id)sender;
- (IBAction)duplicateClicked:(id)sender;
- (IBAction)stopClicked:(id)sender;
- (IBAction)textAction:(id)sender;
- (IBAction)distanceAction:(id)sender;
- (IBAction)precisionAction:(id)sender;

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
