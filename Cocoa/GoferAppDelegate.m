//
//  GoferAppDelegate.m
//  Gofer
//
//  Created by Ted Lemon on 5/1/11.
//  Copyright 2011 Edward W. Y. Lemon III. All rights reserved.
//

#import "GoferAppDelegate.h"
#import "st.h"

@implementation GoferAppDelegate

@synthesize window;
@synthesize inputBox0;
@synthesize inputBox1;
@synthesize inputBox2;
@synthesize inputBox3;
@synthesize inputBox4;
@synthesize inputBox5;
@synthesize inputBox6;
@synthesize inputBox7;
@synthesize distanceBox;

@synthesize distanceMenu;
@synthesize orMenuItem;
@synthesize distanceMenuItem;
@synthesize andMenuItem;
@synthesize notMenuItem;

@synthesize precisionMenu;
@synthesize ignoreSpaceCap;
@synthesize ignoreSpace;
@synthesize ignoreNothing;

@synthesize equationField;

@synthesize dirTable;

void
one_element(void *obj, const char *filename,
	    char *contents, int content_length,
	    int first_line, int last_line,
	    int first_char, int first_len,
	    int last_char, int last_len,
	    int *cur_line, int *cur_char)
{
}

int
one_file(void *obj, const char *filename)
{

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
      if ([orMenuItem state] == NSOnState)
	expr->type = ste_or;
      else if ([andMenuItem state] == NSOnState)
	expr->type = ste_and;
      else if ([notMenuItem state] == NSOnState)
	expr->type = ste_not;
      else if ([distanceMenuItem state] == NSOnState)
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
	[equationField
	  setStringValue: [[NSString alloc] initWithUTF8String: pe]];
    }  
  return expr;
}

- (void)
applicationDidFinishLaunching:(NSNotification *)aNotification
{
  printf("dirTable: %p\n", dirTable);
  checkboxColumn = [dirTable tableColumnWithIdentifier: @"checked"];
  NSButtonCell *buttonColumn = [NSButtonCell new];
  [buttonColumn setButtonType: NSSwitchButton];
  [buttonColumn setTitle: @""];
  [checkboxColumn setDataCell: buttonColumn];
  dirnameColumn = [dirTable tableColumnWithIdentifier: @"dirname"];
  dirNames = [NSMutableArray new];
  dirsChecked = [NSMutableArray new];
  [dirTable setDataSource: self];
}

- (IBAction)
findClicked: (id)sender
{
  int i;
  int n;
  search_term_t *terms;
  st_match_type_t searchExactitude;
  st_expr_t *searchExpr;
  int ui_index;

  st_expr_t *expr = [self genEquation];

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
      [equationField setStringValue: @"Please enter some text to search for."];
      return;
    }

  for (ui_index = 0; ui_index < [uis count]; ui_index++)
    {
      if ([uis objectAtIndex: ui_index] == self)
	break;
    }
  if (ui_index == [uis count])
    [uis addObject: self];

  if ([ignoreSpaceCap state] == NSOnState)
    searchExactitude = match_ignores_spaces_and_case;
  else if ([ignoreSpace state] == NSOnState)
    searchExactitude = match_ignores_spaces;
  else if ([ignoreNothing state] == NSOnState)
    searchExactitude = match_exactly;

  n = extract_search_terms(&terms, searchExpr);

  for (i = 0; i < [dirNames count]; i++)
    {
      NSString *dir = [dirnames objectAtIndex: i];
      NSNumber *checked = [dirsChecked objectAtIndex: i];
      if ([checked booleanValue])
	{
	  if (!search_tree([dir UTF8String],
			   expr, terms, n,
			   one_element, one_file, ui_index,
			   0, searchExactitude))
	    break;
	}
      if (!keepSearching)
	break;
    }
  if (!keepSearching)
    [equationField setStringValue: @"Search Interrupted"];
  else
    [equationField setStringValue: @"Search Complete"];

  if (!results->haveContents)
    {
      emit setTabIndex(0);
      [equationField setStringValue: @"Search found no matches.\n"];
    }

  keepSearching = 1;

  if (expr)
    free_expr(expr);
}

- (IBAction)
saveClicked: (id)sender
{
  printf("saveClicked.\n");
}

- (IBAction)
duplicateClicked: (id)sender
{
  printf("duplicateClicked.\n");
  NSInteger index = [dirTable selectedRow];
  if (index >= 0 && index < [dirNames count])
    {
      [dirNames insertObject: [dirNames objectAtIndex: index] atIndex: index];
      [dirsChecked insertObject: [dirsChecked objectAtIndex: index]
			atIndex: index];
    }
}

- (IBAction)
removeClicked: (id)sender
{
  printf("removeClicked.\n");
  NSInteger index = [dirTable selectedRow];
  if (index >= 0 && index < [dirNames count])
    {
      [dirNames removeObjectAtIndex: index];
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
  [panel beginSheetModalForWindow:window completionHandler:^(NSInteger result)
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
	       [dirTable reloadData];
	     }
	 }];
  printf("addClicked.\n");
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
  [distanceMenuItem
    setTitle: [[NSString alloc]
		initWithFormat: @"Within %d lines of", distance]];
  st_expr_t *expr = [self genEquation];
  if (expr)
    free_expr(expr);
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
}
            
@end

/* Local Variables:  */
/* mode:ObjC */
/* c-file-style:"gnu" */
/* end: */
