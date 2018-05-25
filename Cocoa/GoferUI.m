// -*- Mode: ObjC; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108 -*-
//
//  GoferUI.m
//  Gofer
//
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

#import "GoferAppDelegate.h"
#import "GoferUI.h"
#import "st.h"
#import "filelist.h"
#import <sys/time.h>

@implementation GoferUI

@synthesize win;
@synthesize inputBox0;
@synthesize inputBox1;
@synthesize inputBox2;
@synthesize inputBox3;
@synthesize inputBox4;
@synthesize inputBox5;
@synthesize inputBox6;
@synthesize inputBox7;
@synthesize distanceBox;

@synthesize distancePopUp;
@synthesize distanceMenu;
@synthesize orMenuItem;
@synthesize distanceMenuItem;
@synthesize andMenuItem;
@synthesize notMenuItem;

@synthesize precisionMenu;
@synthesize ignoreSpaceCap;
@synthesize ignoreSpace;
@synthesize ignoreNothing;
@synthesize precisionPopUp;

@synthesize tabView;
@synthesize searchSettingTab;
@synthesize savedSearchTab;
@synthesize searchResultsTab;

@synthesize equationField;
@synthesize dirTable;

@synthesize nextMatchButton;
@synthesize nextFileMatchButton;
@synthesize prevMatchButton;
@synthesize prevFileMatchButton;

@synthesize stopButton;
@synthesize findButton;
@synthesize findMenuItem;
@synthesize findNextMenuItem;
@synthesize findPreviousMenuItem;

@synthesize fileContentView;
@synthesize viewFile;
@synthesize statusMessageField;

@synthesize files;
@synthesize closing;
@synthesize searching;
@synthesize keepSearching;
@synthesize firstMatch;
@synthesize haveContents;

@synthesize filesMatched;
@synthesize matchCount;

@synthesize debug_log;
@synthesize startSec;
@synthesize dirsChecked;
@synthesize dirNames;

- (id)init
{
	initialized = NO;
	self = [super initWithWindowNibName:@"GoferUI" owner: self];
	if (self == nil)
		printf("initWithWindowNibName failed.\n");
	return self;
}

- (void)windowDidLoad
{
	printf("windowDidLoad\n");
	[super windowDidLoad];
	printf("selecting the searchSettings tabView.\n");
	[tabView selectTabViewItem: searchSettingTab];
	[tabView setNeedsDisplay: TRUE];
}

- (void)windowWillLoad
{
	printf("windowWillLoad\n");
	[super windowWillLoad];
}

- (void)newMatch
{
	matchCount++;
	if (firstMatch)
	{
		firstMatch = NO;
		[self showCurMatch];
	}
	else
	{
		[self setMatchButtonStates];
	}
}

- (void)newFileMatch
{
	filesMatched++;
}

- (void)startClose
{
	if (searching)
	{
		closing = YES;
		keepSearching = NO;
	}
	[win performClose: self];
	if (!closing)
		[self finishClosing];
}

- (void)finishClosing
{
	if (files)
		filelist_free(files);
}

- (void)dealloc
{
	printf("deallocating %p\n", self);
}

- (void)awakeFromNib
{
	struct timeval tv;

	[super awakeFromNib];

	printf("wakeFromNib...\n");

	[[self window] setTitle: @"Gofer 20171117.1003"];
	[dirTable registerForDraggedTypes:
			 [NSArray arrayWithObject: NSPasteboardTypeString]];

	checkboxColumn = [dirTable tableColumnWithIdentifier: @"checked"];
	NSButtonCell *buttonColumn = [NSButtonCell new];
	[buttonColumn setButtonType: NSSwitchButton];
	[buttonColumn setTitle: @""];
	[checkboxColumn setDataCell: buttonColumn];
	dirnameColumn = [dirTable tableColumnWithIdentifier: @"dirname"];

	NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
	NSArray *dirNamesDef = [defs arrayForKey: @"dirNames"];
	NSArray *dirsCheckedDef = [defs arrayForKey: @"dirsChecked"];

	if (dirNamesDef != nil)
		dirNames = [dirNamesDef mutableCopyWithZone: NULL];
	else
		dirNames = [NSMutableArray new];
	if (dirsCheckedDef != nil)
		dirsChecked = [dirsCheckedDef mutableCopyWithZone: NULL];
	else
		dirsChecked = [NSMutableArray new];
    if ([dirNames count] != [dirsChecked count])
	{
		dirsChecked = [NSMutableArray new];
		[dirNames enumerateObjectsUsingBlock: ^(id obj, NSUInteger idx, BOOL *stop)
				  {
					  printf("dirsChecked %d\n", (unsigned)idx);
					  [self.dirsChecked
							  addObject: [[NSNumber alloc] initWithBool: NO]];
				  }];
        
	}
		 
	if ([defs objectForKey: @"searchExactitude"] != nil)
	{
		st_match_type_t searchExactitude =
			(st_match_type_t)[defs integerForKey: @"searchExactitude"];
		if (searchExactitude == match_exactly)
			[precisionPopUp selectItem: ignoreNothing];
		else if (searchExactitude == match_ignores_spaces_and_case)
			[precisionPopUp selectItem: ignoreSpaceCap];
		else
			[precisionPopUp selectItem: ignoreSpace];
	}
	else
		[precisionPopUp selectItem: ignoreSpaceCap];

	if ([defs objectForKey: @"distance"] != nil)
	{
		int distance = (int)[defs integerForKey: @"distance"];
		[distanceBox setIntValue: distance];
	}
	else
		[distanceBox setIntValue: 2];
	[self distanceAction: self];

	if ([defs objectForKey: @"combiner"] != nil)
	{
		st_expr_type_t combiner = (st_expr_type_t)
			[defs integerForKey: @"combiner"];
		if (combiner == ste_and)
			[distancePopUp selectItem: andMenuItem];
		else if (combiner == ste_or)
			[distancePopUp selectItem: orMenuItem];
		else if (combiner == ste_not)
			[distancePopUp selectItem: notMenuItem];
		else
			[distancePopUp selectItem: distanceMenuItem];
	}
	else
		[distancePopUp selectItem: distanceMenuItem];

	files = new_filelist();
	[dirTable setDataSource: self];
	curFileName = 0;
	firstMatch = YES;
	keepSearching = YES;
	closing = NO;
	searching = NO;
	gettimeofday(&tv, NULL);
	startSec = tv.tv_sec;
	debug_log = fopen("/tmp/gofer.log", "w");
	if (debug_log == NULL)
	{
		[self setStatusMessage: @"Unable to open debug log"];
		debug_log = stderr;
	} else
		setlinebuf(debug_log);
	[tabView setDelegate: self];
	[tabView selectFirstTabViewItem: self];
	[win makeFirstResponder: inputBox0];

	[self constrainUIToDisplay];
	[tabView setNeedsDisplay: TRUE];
}

- (void)constrainUIToDisplay
{
	NSArray *screenArray = [NSScreen screens];
	NSScreen *myScreen = [win screen];
	NSEnumerator *enumerator = [screenArray objectEnumerator];
	NSScreen *screen;
	int ix = 0;
  
	while (screen = [enumerator nextObject])
	{
		NSRect screenRect = [screen visibleFrame];
		NSString *mString = ((myScreen == screen) ? @"Mine" : @"not mine");
		ix++;
    
		NSLog(@"Screen #%d (%@) Frame: %@",
			  ix, mString, NSStringFromRect(screenRect));
		if (myScreen == screen)
		{
			NSRect frameRect = [win frame];
			int modified = 0;
			if (frameRect.size.width > screenRect.size.width)
			{
				modified = 1;
				frameRect.size.width = screenRect.size.width;
			}
			frameRect.size.height = screenRect.size.height;
			if (modified)
			{
				frameRect.origin.x = 0;
			}
			frameRect.origin.y = 0;
			[win setFrame: frameRect display: YES animate: YES];
		}
	}
  
}

st_expr_t *getText(st_expr_t *expr, NSTextField *src)
{
    search_term_t *ms;
    st_expr_t *n1, *n2;

    const char *cptr = [[src stringValue] UTF8String];
    int len = (int)strlen(cptr);
    if (cptr && len)
    {
        ms = (search_term_t *)malloc(sizeof (search_term_t));
        if (!ms)
            gofer_fatal("no memory for matchset %s", cptr);
        memset(ms, 0, sizeof (search_term_t));
        ms->len = len;
        if (ms->len > ST_LIMIT)
            ms->len = ST_LIMIT;
        memcpy(ms->buf, cptr, ms->len);
        printf(" |%.*s|", (int)ms->len, ms->buf);

        n1 = (st_expr_t *)malloc(sizeof *n1);
        if (!n1)
            gofer_fatal("no memory for matchset expr for %s", cptr);
        memset(n1, 0, sizeof *n1);
        n1->type = ste_term;
        n1->subexpr.term = ms;

        if (!expr)
            expr = n1;
        else
        {
            n2 = (st_expr_t *)malloc(sizeof *n2);
            if (!n2)
                gofer_fatal("no memory for or expr");
            memset(n2, 0, sizeof *n2);
            n2->type = ste_or;
            n2->subexpr.exprs[0] = expr;
            n2->subexpr.exprs[1] = n1;
            expr = n2;
        }
    }
    return expr;
}

- (st_expr_t *)genEquation
{
	st_expr_t *expr, *lhs, *rhs;
  
	lhs = rhs = 0;
  
	/* There are eight input areas for entering match strings.   The first
	 * four are joined together with ors, if values are present in them.
	 * The second four are as well.   So for each set of four, generate an
	 * expression that ors all the terms that have values; put the expression
	 * for the top four into lhm, and the expression for the bottom four into
	 * rhm.
	 */
  
	printf("Search strings:");
	lhs = rhs = 0;
	lhs = getText(lhs, inputBox0);
	lhs = getText(lhs, inputBox1);
	lhs = getText(lhs, inputBox2);
	lhs = getText(lhs, inputBox3);
  
	rhs = getText(rhs, inputBox4);
	rhs = getText(rhs, inputBox5);
	rhs = getText(rhs, inputBox6);
	rhs = getText(rhs, inputBox7);
	printf("\n");
  
	/* If there were expressions to derived from both sets of boxes, then
	 * combine them using whatever combination value is set on the combiner
	 * PopUpButton list.
	 */
	if (lhs && rhs)
	{
		expr = (st_expr_t *)malloc(sizeof *expr);
		if (!expr)
			gofer_fatal("no memory for matchset expr for combiner");
		memset(expr, 0, sizeof *expr);
		NSMenuItem *selectedItem = [distancePopUp selectedItem];
		if (selectedItem == orMenuItem)
			expr->type = ste_or;
		else if (selectedItem == andMenuItem)
			expr->type = ste_and;
		else if (selectedItem == notMenuItem)
			expr->type = ste_not;
		else if (selectedItem == distanceMenuItem)
			expr->type = ste_near_lines;
		else
			expr->type = ste_or;
		expr->subexpr.exprs[0] = lhs;
		expr->subexpr.exprs[1] = rhs;
		if (expr->type == ste_near || expr->type == ste_near_lines)
			expr->n = [distanceBox intValue];
	}
	else if (lhs)
	{
		expr = lhs;
	}
	else if (rhs)
	{
		expr = rhs;
	}
	else
	{
		expr = 0;
	}
  
	if (expr)
	{
		const char *pe = print_expr(expr);
		fprintf(debug_log, "equation: %s\n", pe);
		if (pe)
			[win setTitle: [[NSString alloc] initWithUTF8String: pe]];
	}  
	return expr;
}

- (bool)deleteHit: (id)sender
{
	if ([tabView selectedTabViewItem] == searchResultsTab)
	{
		[self prevMatch: sender];
		return YES;
	}
	else
		return NO;
}

- (void)
	returnHit: (id)sender
{
	if ([tabView selectedTabViewItem] == searchResultsTab)
		[self nextMatch: sender];
	else
		[self findClicked: sender];
}

- (void)
	commandRet: (id)sender
{
	if ([tabView selectedTabViewItem] == searchResultsTab)
		[self nextFileMatch: sender];
	// Doesn't mean anything if we don't have search results.
}

- (IBAction)
	findClicked: (id)sender
{
	st_expr_t *expr = [self genEquation];
	dispatch_queue_t defQ;
	int ui_index;

	if (files)
	{
		filelist_free(files);
		files = new_filelist();
	}
	filesMatched = 0;
	matchCount = 0;
	curFileName = 0;
	firstMatch = YES;
	keepSearching = YES;
	searching = YES;

	/* We need something to search before we can do a search. */
	if ([dirNames count] == 0)
	{
		[equationField
			setStringValue:
				@"Please click on Add to add a directory in which to search."];
		return;
	}
  
	if (!expr)
	{
		[equationField setStringValue:
						   @"Please enter some text to search for."];
		return;
	}
  
	ui_index = [GoferAppDelegate ui_index: self];
  
	[findButton setEnabled: NO];
	[findMenuItem setEnabled: NO];
	[stopButton setEnabled: YES];

	defQ = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

	NSMenuItem *selectedItem = [precisionPopUp selectedItem];
	NSArray *dirNamesCopy = [dirNames copyWithZone: NULL];
	NSArray *dirsCheckedCopy = [dirsChecked copyWithZone: NULL];
	search_term_t *terms;
    int n;
	n = extract_search_terms(&terms, expr);

	st_match_type_t searchExactitude = match_exactly;

	if (selectedItem == self.ignoreNothing)
		searchExactitude = match_exactly;
	else if (selectedItem == self.ignoreSpaceCap)
		searchExactitude = match_ignores_spaces_and_case;
	else
		searchExactitude = match_ignores_spaces;

	dispatch_async(defQ, ^{

			int i;

			for (i = 0; i < [dirNamesCopy count]; i++)
			{
				NSString *dir = [dirNamesCopy objectAtIndex: i];
				NSNumber *checked = [dirsCheckedCopy objectAtIndex: i];

				if ([checked boolValue])
				{
					fprintf(self.debug_log, "searching tree %s\n", [dir UTF8String]);
					if (!search_tree([dir UTF8String],
									 expr, terms, n,
									 one_element,
                                     one_file,
                                     (void *)(unsigned long)ui_index,
									 0, searchExactitude))
						break;
				}
				if (self.keepSearching == NO)
					break;
			}

			if (!self.closing)
			{
				dispatch_async(dispatch_get_main_queue(), ^{
						[self.findMenuItem setEnabled: YES];
						[self.findButton setEnabled: YES];
						[self.stopButton setEnabled: NO];
						fprintf(self.debug_log, "searching completed.\n");
						if (self.keepSearching == NO) {
							[self.equationField setStringValue: @"Search Interrupted"];
							fprintf(self.debug_log, "statusMessage: Search Interrupted.\n");
							[self.statusMessageField setStringValue: @"Search Interrupted"];
							[self.viewFile setStringValue: @""];
						} else {
							if (self.haveContents)
							{
								[self.equationField setStringValue: @"Search Complete"];
								fprintf(self.debug_log, "statusMessage: %d matches in %d files.\n",
										self.matchCount, self.filesMatched);
								[self.statusMessageField setStringValue:
														[[NSString alloc]
															initWithFormat: @"1/%d matches in %d files",
															self.matchCount, self.filesMatched]];
							}
							else
							{
								NSAlert *alert;
								alert = [NSAlert alertWithMessageText: @"No matches"
														defaultButton: nil
													  alternateButton: nil
														  otherButton: nil
											informativeTextWithFormat: @""];
								[alert runModal];
								[self.tabView selectTabViewItem: self.searchSettingTab];
								[self.equationField setStringValue: @"Search found no matches.\n"];
								fprintf(self.debug_log, "statusMessage: Search found no matches.\n");
								[self.statusMessageField setStringValue:
														@"Search found no matches.\n"];
								[self.viewFile setStringValue: @""];
							}
						}

						self.keepSearching = YES;
					});
			}
      
			if (expr)
				free_expr(expr);
			if (self.closing)
				[self finishClosing];
		});
  
	[tabView selectTabViewItem: searchResultsTab];
}

- (IBAction)
saveClicked: (id)sender
{
	printf("saveClicked.\n");
}

- (IBAction)
duplicateClicked: (id)sender
{
	NSInteger index = [dirTable selectedRow];
	if (index >= 0 && index < [dirNames count])
	{
		[dirNames insertObject: [dirNames objectAtIndex: index] atIndex: index];
		[dirsChecked insertObject: [dirsChecked objectAtIndex: index]
						  atIndex: index];
		[self writeDirDefaults];
		[dirTable reloadData];
	}
}

- (IBAction)
removeClicked: (id)sender
{
	NSInteger index = [dirTable selectedRow];
	if (index >= 0 && index < [dirNames count])
	{
		[dirNames removeObjectAtIndex: index];
		[dirsChecked removeObjectAtIndex: index];
		[self writeDirDefaults];
		[dirTable reloadData];
	}
}

- (IBAction)
addClicked: (id)sender
{
	// Set up a file chooser dialog
	NSOpenPanel* panel = [NSOpenPanel openPanel];
	[panel setCanChooseDirectories:YES];
	[panel setCanChooseFiles:NO];
	[panel setAllowsMultipleSelection:YES];
	[panel setMessage:@"Add one or more directories to the search list."];
    
	// Make it modal to the search UI window
	[panel beginSheetModalForWindow:win completionHandler:^(NSInteger result)
		   {
			   // This code runs when the modal dialog finishes.
			   // If they clicked OK, we add the files or directories
			   // that were selected.
			   if (result == NSFileHandlingPanelOKButton)
			   {
				   NSArray *urls = [panel URLs];
				   NSEnumerator *each = [urls objectEnumerator];
				   NSURL *url;
				   while (url = [each nextObject])
				   {
					   [self.dirNames addObject: [url path]];
					   [self.dirsChecked
								   addObject: [[NSNumber alloc] initWithBool: YES]];
					   printf("you chose %s\n", [[url path] UTF8String]);
				   }
#if 0
				   NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
				   [defs setObject: dirNames forKey: @"dirNames"];
				   [defs setObject: dirsChecked forKey: @"dirsChecked"];
#else
				   [self writeDirDefaults];
#endif
				   [self.dirTable reloadData];
			   }
		   }];
}

- (IBAction)
stopClicked: (id)sender
{
	keepSearching = NO;
}

- (IBAction)
precisionAction: (id)sender
{
	st_match_type_t searchExactitude = match_exactly;
	NSMenuItem *selectedItem = [precisionPopUp selectedItem];
  
	if (selectedItem == ignoreNothing)
		searchExactitude = match_exactly;
	else if (selectedItem == ignoreSpaceCap)
		searchExactitude = match_ignores_spaces_and_case;
	else
		searchExactitude = match_ignores_spaces;
	[[NSUserDefaults standardUserDefaults]
		setInteger: searchExactitude forKey: @"searchExactitude"];
}

- (IBAction)
textAction: (id)sender
{
	st_expr_t *expr = [self genEquation];
	if (expr)
		free_expr(expr);
	printf("textAction.\n");
}

- (IBAction)
distanceAction: (id)sender
{
	int distance = [distanceBox intValue];
	st_expr_type_t xt;

	NSMenuItem *selectedItem = [distancePopUp selectedItem];
	if (selectedItem == orMenuItem)
		xt = ste_or;
	else if (selectedItem == andMenuItem)
		xt = ste_and;
	else if (selectedItem == notMenuItem)
		xt = ste_not;
	else if (selectedItem == distanceMenuItem)
	{
		xt = ste_near_lines;
		[distancePopUp selectItem: distanceMenuItem];
	}
	else
		xt = ste_near_lines;

	NSInteger index = [distancePopUp indexOfItem: distanceMenuItem];
	[distancePopUp removeItemAtIndex: index];
	[distancePopUp
		insertItemWithTitle: [[NSString alloc]
								 initWithFormat: @"Within %d lines of", distance]
					atIndex: index];
	distanceMenuItem = [distancePopUp itemAtIndex: index];

	switch(xt)
	{
	case ste_or:
		[distancePopUp selectItem: orMenuItem];
		break;
	case ste_and:
		[distancePopUp selectItem: andMenuItem];
		break;
	case ste_not:
		[distancePopUp selectItem: notMenuItem];
		break;
	default:
	case ste_near_lines:
		[distancePopUp selectItem: distanceMenuItem];
		break;
	}

	st_expr_t *expr = [self genEquation];
	if (expr)
		free_expr(expr);
	[[NSUserDefaults standardUserDefaults]
		setInteger: distance forKey: @"distance"];

	[[NSUserDefaults standardUserDefaults]
		setInteger: xt forKey: @"combiner"];
	printf("distanceAction.\n");
}

- (NSInteger) numberOfRowsInTableView: (NSTableView *)view {
	NSInteger rv = [dirNames count];
	return rv;
}
            
- (id)            tableView: (NSTableView *)view
  objectValueForTableColumn:(NSTableColumn *)column
						row:(NSInteger)row
{
	id rv;
  
	if (row < 0 || row >= [dirNames count])
	{
		if (column == dirnameColumn)
			rv = @"weirdness";
		else
			rv = [[NSNumber alloc] initWithBool: NO];
	}
	else
	{
		if (column == dirnameColumn)
			rv = [dirNames objectAtIndex: row];
		else if (column == checkboxColumn)
			rv = [dirsChecked objectAtIndex: row];
		else
			rv = @"weirdness";
	}
	return rv;
}

- (void)tableView:(NSTableView *)view
   setObjectValue:(id)obj
   forTableColumn:(NSTableColumn *)column
			  row:(NSInteger)row
{
	if (row >= 0)
	{
		if (row < [dirNames count])
		{
			if (column == dirnameColumn)
				[dirNames replaceObjectAtIndex: row withObject: obj];
			else if (column == checkboxColumn)
				[dirsChecked replaceObjectAtIndex: row withObject: obj];
		}
		else if (row == [dirNames count])
		{
			if (column == dirnameColumn)
				[dirNames addObject: obj];
			else if (column == checkboxColumn)
				[dirsChecked addObject: obj];
		}
	}
	[self writeDirDefaults];
}
            
- (void)zapMatchView
{
	[viewFile setStringValue: @"Searching..."];
	[fileContentView setString: @""];
	haveContents = false;
}

- (void)showCurMatch
{
	fileresults_t *curFile;
	matchzone_t *mz;
	const dispatch_queue_t queue = dispatch_get_current_queue();
	if (queue != dispatch_get_main_queue())
	{
		printf("Not on the main queue.\n");
	}
	else
	{
		printf("On the main queue.\n");
	}

	if (files->cur_file >= files->nfiles)
		return;
	curFile = files->files[files->cur_file];
	if (curFile->curzone >= curFile->nzones)
		return;
	mz = curFile->matches[curFile->curzone];

	if (!curFileName || strcmp(curFileName, curFile->filename))
	{
		if (curFileName)
		{
			fputs("\n", debug_log);
		}
		fprintf(debug_log, "%s\n", curFile->filename);
      
		curFileName = curFile->filename;
		[viewFile setStringValue:
					  [[NSString alloc] initWithUTF8String: curFileName]];
		[viewFile setToolTip:
					  [[NSString alloc] initWithUTF8String: curFileName]];
		[tabView setNeedsUpdateConstraints: TRUE];
		[tabView setNeedsLayout: TRUE];
		[tabView layoutSubtreeIfNeeded];
		[tabView updateConstraintsForSubtreeIfNeeded];
		fprintf(debug_log, "setStringValue\n");
		NSData *bar = [NSData dataWithBytesNoCopy: curFile->contents
										   length: curFile->content_length
									 freeWhenDone: FALSE];
		NSString *foo = [[NSString alloc] initWithData: bar
											  encoding: NSUTF8StringEncoding];
		// If UTF8 encoding failed, just treat it as plain ASCII.
		if (foo == nil)
		{
			foo = [[NSString alloc] initWithData: bar
										encoding: NSASCIIStringEncoding];
		}
		[fileContentView setString: foo];
		fprintf(debug_log, "setStringValue done\n");
		[fileContentView setRichText: YES];
		[self highlightMatchesAt: mz status: YES];
		haveContents = true;
	}
	else
		[self seekMatch];
	[self setMatchButtonStates];

	int matchesSoFar = 1;
	int i;
	for (i = 0; i < files->cur_file; i++)
		matchesSoFar += files->files[i]->nzones;
	if (i < files->nfiles)
		matchesSoFar += files->files[i]->curzone;
	[statusMessageField setStringValue:
							[[NSString alloc]
								initWithFormat: @"%d/%d matches in %d files",
								matchesSoFar, matchCount, filesMatched]];
}

- (void)seekMatch
{
	fileresults_t *curFile;
	matchzone_t *mz;

	if (files->cur_file >= files->nfiles)
		return;
	curFile = files->files[files->cur_file];
	if (curFile->curzone >= curFile->nzones)
		return;
	mz = curFile->matches[curFile->curzone];

	[self highlightMatchesAt: mz status: YES];
	[self setMatchButtonStates];
}

- (IBAction)nextMatch: (id)sender
{
	fileresults_t *curFile;
	if (!files || files->cur_file >= files->nfiles)
	{
		[statusMessageField setStringValue: @"No further matches."];
		[self setNextMatchEnabled: NO];
		[self setNextFileMatchEnabled: NO];
		return;
	}

	[self setPrevMatchEnabled: YES];
	curFile = files->files[files->cur_file];
	if (curFile->curzone + 1 >= curFile->nzones)
	{
		if (files->cur_file + 1 < files->nfiles)
		{
			[self nextFileMatch: sender];
		}
		else
		{
			[self setNextMatchEnabled: NO];
			[self setNextFileMatchEnabled: NO];
			[statusMessageField setStringValue: @"No further matches."];
		}
		return;
	}
	else
	{
		[self unShowMatch];
		curFile->curzone++;
	}
	[self showCurMatch];
}

- (IBAction)nextFileMatch: (id)sender
{
	if (!files || files->cur_file + 1 >= files->nfiles)
	{
		[nextFileMatchButton setEnabled: NO];
		return;
	}

	[self unShowMatch];
	files->cur_file++;
	files->files[files->cur_file]->curzone = 0;
	[self showCurMatch];
}

- (IBAction)prevMatch: (id)sender
{
	fileresults_t *curFile;
	if (!files || files->cur_file >= files->nfiles)
	{
		[self setPrevMatchEnabled: NO];
		return;
	}

	curFile = files->files[files->cur_file];
	if (curFile->curzone == 0)
	{
		[self prevFileMatch: sender];
		return;
	}
	else
	{
		[self unShowMatch];
		curFile->curzone--;
	}
	[self showCurMatch];
}

- (IBAction)prevFileMatch: (id)sender
{
	if (!files || files->cur_file == 0)
	{
		[self setPrevFileMatchEnabled: NO];
		return;
	}

	[self unShowMatch];
	files->cur_file--;
	if (files->files[files->cur_file]->nzones > 0)
	{
		files->files[files->cur_file]->curzone =
			files->files[files->cur_file]->nzones - 1;
	}
	[self showCurMatch];
}

- (IBAction)selectionToClipBoardWithInfo: (id)sender
{
	NSRange range = [fileContentView selectedRange];

	/* If there is a selection, and if we have a file, get the text
	 * from it, since NSText doesn't seem to offer a way to do this.
	 */
	if (range.length &&
		files && files->cur_file >= 0 && files->cur_file < files->nfiles)
	{
		fileresults_t *curFile = files->files[files->cur_file];
		// Hopefully the selection is within the file's contents...
		if (range.location + range.length < curFile->content_length)
		{
			NSString *selection =
				[[NSString alloc]
							initWithBytes: curFile->contents + range.location
								   length: range.length
								 encoding: NSASCIIStringEncoding];
			char *dot; // = strrchr(curFile->filename, '.');
			/*	  if (!dot) */
			dot = curFile->filename + strlen(curFile->filename);
			NSString *filename =
				[[NSString alloc] initWithBytes: curFile->filename
										 length: dot - curFile->filename
									   encoding: NSUTF8StringEncoding];
			NSArray *selArray = [NSArray arrayWithObjects:
											 [NSString stringWithFormat:
														   @"%@\n%@", filename, selection, nil], nil];
			NSPasteboard *pb = [NSPasteboard generalPasteboard];
			[pb clearContents];
			[pb writeObjects: selArray];
		}
	}
}

- (void)unShowMatch
{
	fileresults_t *curFile;
	matchzone_t *mz;

	if (files->cur_file >= files->nfiles)
		return;
	curFile = files->files[files->cur_file];
	if (curFile->curzone >= curFile->nzones)
		return;
	mz = curFile->matches[curFile->curzone];

	[self highlightMatchesAt: mz status: NO];
}

- (void)setMatchButtonStates
{
	fileresults_t *curFile;

	// Make sure that we actually can move between search results...
	if (!files || files->nfiles == 0)
	{
		[self setNextMatchEnabled: NO];
		[self setPrevMatchEnabled: NO];
		[self setNextFileMatchEnabled: NO];
		[self setPrevFileMatchEnabled: NO];
		return;
	}

	// Make sure that files->cur_file is in range.
	if (files->cur_file >= files->nfiles)
	{
		// This should never happen.
		files->cur_file = files->nfiles - 1;
		curFile = files->files[files->cur_file];
		curFile->curzone = curFile->nzones - 1;
	}

	// Now we can tell if we can go to a next file or a previous file.
	if (files->cur_file == files->nfiles - 1)
		[self setNextFileMatchEnabled: NO];
	else
		[self setNextFileMatchEnabled: YES];
	if (files->cur_file == 0)
		[self setPrevFileMatchEnabled: NO];
	else
		[self setPrevFileMatchEnabled: YES];

	curFile = files->files[files->cur_file];

	// If there's a next file or a next zone, Next Match should be enabled.
	if (curFile->curzone + 1 >= curFile->nzones ||
		files->cur_file + 1 <= files->nfiles)
		[nextMatchButton setEnabled: YES];
	else
		[nextMatchButton setEnabled: NO];

	if (curFile->curzone > 0 || files->cur_file > 0)
		[prevMatchButton setEnabled: YES];
	else
		[prevMatchButton setEnabled: NO];
}


- (void)setNextMatchEnabled: (BOOL)state
{
	[nextMatchButton setEnabled: state];
	[findNextMenuItem setEnabled: state];
}

- (void)setPrevMatchEnabled: (BOOL)state
{
	[prevMatchButton setEnabled: state];
	[findPreviousMenuItem setEnabled: state];
}

- (void)setNextFileMatchEnabled: (BOOL)state
{
	[nextFileMatchButton setEnabled: state];
}

- (void)setPrevFileMatchEnabled: (BOOL)state
{
	[prevFileMatchButton setEnabled: state];
}

- (void)setStatusMessage: (NSString *)message
{
	[statusMessageField setStringValue: message];
}

- (void)       tabView:(NSTabView *)tabView
  didSelectTabViewItem:(NSTabViewItem *)tabViewItem
{
	printf("didSelectTabViewItem: %s %s\n",
		   [[tabViewItem label] UTF8String],
		   [[searchSettingTab label] UTF8String]);

	if (tabViewItem == searchSettingTab)
	{
		keepSearching = NO;
		[viewFile setStringValue: @""];
		[fileContentView setString: @""];
		[statusMessageField setStringValue: @""];
		[win makeFirstResponder: inputBox0];
	}
}
  
- (BOOL)          tabView:(NSTabView *)tabView 
  shouldSelectTabViewItem:(NSTabViewItem *)tabViewItem
{
	if (initialized)
	{
		printf("shouldSelectTabViewItem %s: YES!\n", [[tabViewItem label] UTF8String]);
		return YES;
	}
	printf("shouldSelectTabViewItem %s: NO!\n", [[tabViewItem label] UTF8String]);
	return NO;
}

- (void)        tabView:(NSTabView *)tabView
  willSelectTabViewItem:(NSTabViewItem *)tabViewItem {
	printf("willSelectTabViewItem: %s\n", [[tabViewItem label] UTF8String]);
}
  
- (void)writeDirDefaults
{
	NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
	[defs setObject: dirNames forKey: @"dirNames"];
	[defs setObject: dirsChecked forKey: @"dirsChecked"];
}

- (BOOL)tableView:(NSTableView *)aTableView
       acceptDrop:(id < NSDraggingInfo >)info
			  row:(NSInteger)row
    dropOperation:(NSTableViewDropOperation)operation
{
	NSPasteboard *pb = [info draggingPasteboard];
	NSArray *array = [pb pasteboardItems];
	int oldRow;
	id filename;
	id flag;
	if ([array count] != 1)
	{
		printf("count not one: %ld\n", [array count]);
		return NO;
	}
	oldRow = [[[array objectAtIndex: 0]
				  stringForType: NSPasteboardTypeString] intValue];

	if (operation == NSTableViewDropAbove)
		--row;

	[dirTable beginUpdates];
	[dirTable moveRowAtIndex: oldRow toIndex: row];
	[dirTable endUpdates];

	filename = [dirNames objectAtIndex: oldRow];
	[dirNames removeObjectAtIndex: oldRow];
	[dirNames insertObject: filename atIndex: row];
	flag = [dirsChecked objectAtIndex: oldRow];
	[dirsChecked removeObjectAtIndex: oldRow];
	[dirsChecked insertObject: flag atIndex: row];

	[self writeDirDefaults];
	return YES;
}

- (NSDragOperation)tableView:(NSTableView *)aTableView
				validateDrop:(id < NSDraggingInfo >)info
				 proposedRow:(NSInteger)row
       proposedDropOperation:(NSTableViewDropOperation)operation
{
	printf("validateDrop: row %ld, operation: %d\n",
		   (long)row, (int)operation);
	if ([info draggingSource] == nil)
		return NSDragOperationNone;
	return NSDragOperationMove;
}

- (BOOL)     tableView:(NSTableView *)aTableView
  writeRowsWithIndexes:(NSIndexSet *)rowIndexes
		  toPasteboard:(NSPasteboard *)pboard
{
	NSMutableArray *array =
		[NSMutableArray arrayWithCapacity: [rowIndexes count]];
	[rowIndexes enumerateIndexesUsingBlock: ^(NSUInteger idx, BOOL *stop) {
			[array addObject: [NSString stringWithFormat: @"%d", (int)idx, nil]];
		}];
	[pboard clearContents];
	[pboard writeObjects: array];
	return YES;
}

- (void)highlightMatchesAt: (matchzone_t *)mz  status: (BOOL)status
{
	NSRange fullRange;
	NSRange firstRange;
	NSRange secondRange;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	fprintf(self.debug_log, "%ld %ld %qd %qd %qd %qd %s\n",
			(long)(tv.tv_sec - self.startSec), (long)tv.tv_usec,
			mz->first_char, mz->first_len, mz->last_char, mz->last_len,
			status ? "t" : "f");
	fullRange.location = mz->first_char;
	if (mz->last_len)
		fullRange.length = mz->last_char - mz->first_char + mz->last_len;
	else
		fullRange.length = mz->first_len;
	if (status)
		[self.fileContentView scrollRangeToVisible: fullRange];

	firstRange.location = mz->first_char;
	firstRange.length = mz->first_len;
	[self setRangeHighlight: firstRange status: YES];
	if (mz->last_len)
	{
		secondRange.location = mz->last_char;
		secondRange.length = mz->last_len;
		[self setRangeHighlight: secondRange status: YES];
	}

	if (status)
		[self.fileContentView showFindIndicatorForRange: fullRange];
}

- (void)setRangeHighlight: (NSRange)range status: (BOOL)status
{
	if (status)
		[fileContentView setTextColor: [NSColor redColor] range: range];
	else
		[fileContentView setTextColor: [NSColor blackColor] range: range];
}

- (void)windowDidBecomeMain: (NSNotification *)notification
{
	initialized = YES;
	[GoferAppDelegate newTopUI: [notification object]];
}

@end

/* Local Variables:  */
/* mode:ObjC */
/* end: */
