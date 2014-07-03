//
//  GoferUI.m
//  Gofer
//
//  Copyright 2013 Edward W. Y. Lemon III. All rights reserved.
//

#import "GoferUI.h"
#import "GoferAppDelegate.h"
#import "st.h"
#import "filelist.h"

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

- (id)init
{
  initialized = NO;
  self = [super initWithWindowNibName:@"GoferUI"];  
  if (self == nil)
    printf("initWithWindowNibName failed.\n");
  return self;
}

- (void)windowDidLoad
{
  printf("windowDidLoad\n");
  [super windowDidLoad];
  [tabView selectTabViewItem: searchSettingTab];
}

- (void)windowWillLoad
{
  printf("windowWillLoad\n");
  [super windowWillLoad];
}

- (filelist_t *)files
{
  return files;
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
  [super awakeFromNib];

  printf("wakeFromNib...\n");

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
		 
  if ([defs objectForKey: @"searchExactitude"] != nil)
    {
      st_match_type_t searchExactitude =
	[defs integerForKey: @"searchExactitude"];
      if (searchExactitude == match_exactly)
	[precisionPopUp selectItem: ignoreNothing];
      else if (searchExactitude == match_ignores_spaces_and_case)
	[precisionPopUp selectItem: ignoreSpaceCap];
      else
	[precisionPopUp selectItem: ignoreSpace];
    }
  else
    [precisionPopUp selectItem: ignoreSpaceCap];

  if ([defs objectForKey: @"combiner"] != nil)
    {
      st_expr_type_t combiner =
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

  if ([defs objectForKey: @"distance"] != nil)
    {
      int distance = [defs integerForKey: @"distance"];
      [distanceBox setIntValue: distance];
    }
  else
    [distanceBox setIntValue: 2];
  [self distanceAction: self];

  files = new_filelist();
  [dirTable setDataSource: self];
  curFileName = 0;
  firstMatch = YES;
  keepSearching = YES;
  closing = NO;
  searching = NO;
  [tabView setDelegate: self];
  printf("selecting tabView index 0\n");
  [win makeFirstResponder: inputBox0];

  [self constrainUIToDisplay];
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

- (st_expr_t *)genEquation
{
  st_expr_t *expr, *lhs, *rhs, *n1, *n2;
  search_term_t *ms;
  
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
#define GETTEXT(expr, src)						\
  {									\
    const char *cptr = [[src stringValue] UTF8String];			\
    int len = (int)strlen(cptr);					\
    if (cptr && len)							\
      {									\
	ms = (search_term_t *)malloc(sizeof (search_term_t));		\
	if (!ms)							\
	  gofer_fatal("no memory for matchset %s", cptr);		\
	memset(ms, 0, sizeof (search_term_t));				\
	ms->len = len;							\
	if (ms->len > ST_LIMIT)						\
	  ms->len = ST_LIMIT;						\
	memcpy(ms->buf, cptr, ms->len);					\
	printf(" |%.*s|", ms->len, ms->buf);				\
									\
	n1 = (st_expr_t *)malloc(sizeof *n1);				\
	if (!n1)							\
	  gofer_fatal("no memory for matchset expr for %s", cptr);	\
	memset(n1, 0, sizeof *n1);					\
	n1->type = ste_term;						\
	n1->subexpr.term = ms;						\
									\
	if (!expr)							\
	  expr = n1;							\
	else								\
	  {								\
	    n2 = (st_expr_t *)malloc(sizeof *n2);			\
	    if (!n2)							\
	      gofer_fatal("no memory for or expr");			\
	    memset(n2, 0, sizeof *n2);					\
	    n2->type = ste_or;						\
	    n2->subexpr.exprs[0] = expr;				\
	    n2->subexpr.exprs[1] = n1;					\
	    expr = n2;							\
	  }								\
      }									\
  }
  GETTEXT(lhs, inputBox0);
  GETTEXT(lhs, inputBox1);
  GETTEXT(lhs, inputBox2);
  GETTEXT(lhs, inputBox3);
  
  GETTEXT(rhs, inputBox4);
  GETTEXT(rhs, inputBox5);
  GETTEXT(rhs, inputBox6);
  GETTEXT(rhs, inputBox7);
  printf("\n");
  
  /* If there were expressions to derived from both sets of boxes, then
   * combine them using whatever combination value is set on the combiner
   * PopUpButton list.
   */
  if (lhs && rhs)
    {
      expr = (st_expr_t *)malloc(sizeof *n1);
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
      printf("equation: %s\n", pe);
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

  dispatch_async(defQ, ^{
      NSArray *dirNamesCopy = [dirNames copyWithZone: NULL];
      NSArray *dirsCheckedCopy = [dirsChecked copyWithZone: NULL];

      int i;
      int n;
      search_term_t *terms;
      st_match_type_t searchExactitude = match_exactly;
      NSMenuItem *selectedItem = [precisionPopUp selectedItem];

      if (selectedItem == ignoreNothing)
	searchExactitude = match_exactly;
      else if (selectedItem == ignoreSpaceCap)
	searchExactitude = match_ignores_spaces_and_case;
      else
	searchExactitude = match_ignores_spaces;

      n = extract_search_terms(&terms, expr);

      for (i = 0; i < [dirNames count]; i++)
	{
	  NSString *dir = [dirNamesCopy objectAtIndex: i];
	  NSNumber *checked = [dirsCheckedCopy objectAtIndex: i];

	  if ([checked boolValue])
	    {
	      if (!search_tree([dir UTF8String],
			       expr, terms, n,
			       one_element, one_file, (void *)ui_index,
			       0, searchExactitude))
		break;
	    }
	  if (keepSearching == NO)
	    break;
	}

      if (!closing)
	{
	  dispatch_async(dispatch_get_main_queue(), ^{
	      [findMenuItem setEnabled: YES];
	      [findButton setEnabled: YES];
	      [stopButton setEnabled: NO];
	      printf("searching completed.\n");
	      if (keepSearching == NO) {
		[equationField setStringValue: @"Search Interrupted"];
		printf("statusMessage: Search Interrupted.\n");
		[statusMessageField setStringValue: @"Search Interrupted"];
		[viewFile setStringValue: @""];
	      } else {
		if (haveContents)
		  {
		    [equationField setStringValue: @"Search Complete"];
		    printf("statusMessage: %d matches in %d files.\n",
			   matchCount, filesMatched);
		    [statusMessageField setStringValue:
					  [[NSString alloc]
					    initWithFormat: @"1/%d matches in %d files",
					    matchCount, filesMatched]];
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
		    [tabView selectTabViewItem: searchSettingTab];
		    [equationField setStringValue: @"Search found no matches.\n"];
		    printf("statusMessage: Search found no matches.\n");
		    [statusMessageField setStringValue:
					  @"Search found no matches.\n"];
		    [viewFile setStringValue: @""];
		  }
	      }

	      keepSearching = YES;
	    });
	}
      
      if (expr)
	free_expr(expr);
      if (closing)
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
		   [dirNames addObject: [url path]];
		   [dirsChecked
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
	       [dirTable reloadData];
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
  [distanceMenuItem
    setTitle: [[NSString alloc]
		initWithFormat: @"Within %d lines of", distance]];
  st_expr_t *expr = [self genEquation];
  if (expr)
    free_expr(expr);
  [[NSUserDefaults standardUserDefaults]
	setInteger: distance forKey: @"distance"];
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

  if (files->cur_file >= files->nfiles)
    return;
  curFile = files->files[files->cur_file];
  if (curFile->curzone >= curFile->nzones)
    return;
  mz = curFile->matches[curFile->curzone];

  if (!curFileName || strcmp(curFileName, curFile->filename))
    {
      curFileName = curFile->filename;
      [viewFile setStringValue:
		  [[NSString alloc] initWithUTF8String: curFileName]];
      [viewFile setToolTip:
		  [[NSString alloc] initWithUTF8String: curFileName]];
      [fileContentView setString:
	[[NSString alloc]
	 initWithBytesNoCopy: curFile->contents
		      length: curFile->content_length
		    encoding: NSASCIIStringEncoding
		freeWhenDone: NO]];
      [fileContentView setRichText: YES];

      NSRange range;
      range.location = mz->first_char;
      if (mz->last_len)
	range.length = mz->last_char - mz->first_char + mz->last_len;
      else
	range.length = mz->first_len;
      [fileContentView scrollRangeToVisible: range];
      range.length = mz->first_len;
      [fileContentView setTextColor: [NSColor redColor] range: range];
      if (mz->last_len)
	{
	  range.location = mz->last_char;
	  range.length = mz->last_len;
	  [fileContentView setTextColor: [NSColor redColor] range: range];
	}
      haveContents = true;
    }
  else
    [self seekMatch];
  [self setMatchButtonStates];

  int matchesSoFar = 1;
  int i, j;
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

  NSRange range;
  range.location = mz->first_char;
  if (mz->last_len)
    range.length = mz->last_char - mz->first_char + mz->last_len;
  else
    range.length = mz->first_len;
  [fileContentView scrollRangeToVisible: range];
  range.length = mz->first_len;
  [fileContentView setTextColor: [NSColor redColor] range: range];
  if (mz->last_len)
    {
      range.location = mz->last_char;
      range.length = mz->last_len;
      [fileContentView setTextColor: [NSColor redColor] range: range];
    }

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
	  char *dot = strrchr(curFile->filename, '.');
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

  NSRange range;
  range.location = mz->first_char;
  range.length = mz->first_len;
  [fileContentView setTextColor: [NSColor blackColor] range: range];
  if (mz->last_len)
    {
      range.location = mz->last_char;
      range.length = mz->last_len;
      [fileContentView setTextColor: [NSColor blackColor] range: range];
    }
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
  printf("shouldSelectTabViewItem: %s\n", [[tabViewItem label] UTF8String]);
  if (initialized)
    return YES;
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
  int i;
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

- (void)windowDidBecomeMain: (NSNotification *)notification
{
  initialized = YES;
  [GoferAppDelegate newTopUI: [notification object]];
}

@end

/* Local Variables:  */
/* mode:ObjC */
/* c-file-style:"gnu" */
/* end: */
