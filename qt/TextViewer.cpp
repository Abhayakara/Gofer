/* Copyright (c) 2006, 2011 Edward W. Lemon III
 *
 *  This file is part of GOFER.
 *
 *  GOFER is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GOFER is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GOFER, in a file called COPYING; if not, write
 *  to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 *  Boston, MA  02111-1307  USA
 */

#include <QtGui>
#include <QAbstractSlider>
#include <qapplication.h>
#include "TextViewer.h"
#include "st.h"

QApplication *TextViewer::application;

TextViewer::TextViewer(QScrollBar *scrollBar)
{
  setAutoFillBackground(true);
  setBackgroundRole(QPalette::Base);
  scrollbar = scrollBar;
  QObject::connect(scrollbar, SIGNAL(valueChanged(int)), this,
		   SLOT(scrollValueChanged(int)));
  QObject::connect(scrollbar, SIGNAL(actionTriggered(int)), this,
		   SLOT(scrollActionTriggered(int)));
  contents = 0;
  content_length = 0;
  pos1 = posLen1 = pos2 = posLen2 = 0;
  selection_start = -1;
  selection_end = -1;
  initial_position = -1;
  top = display_lines = -1;
  selecting = false;
  filename = tr("");
  scrollTimerId = -1;
  first = last = 0;

  /* Get the plain and boldface fonts so that we know how tall they are
   * and can measure the width of each glyph.
   */
  QFont pf(font());
  QFont bf(font());

  bf.setBold(true);
  bf.setPointSize(18);
  pf.setPointSize(18);

  plainMetrics = new QFontMetrics(pf);
  boldMetrics = new QFontMetrics(bf);

  plainFont = new QFont(pf);
  boldFont = new QFont(bf);

  haveFocus = false;

  setFocusPolicy(Qt::StrongFocus);
}

TextViewer::~TextViewer()
{
  delete plainMetrics;
  delete boldMetrics;
  delete plainFont;
  delete boldFont;
}

void
TextViewer::setApplication(QApplication *app)
{
  TextViewer::application = app;
}

/* Find the beginning of the line containing the specified position.   This is
 * either the character after the first newliney character preceding pos, or
 * it's the beginning of the buffer.   Note that pos could, in theory, be
 * newliney; that doesn't count.   Probably won't ever happen in practice
 * anyway.
 */
int
TextViewer::findBOL(int pos)
{
  int bol;

  if (pos == 0)
    return pos;
  for (bol = pos - 1; (bol > 0 &&
		       contents[bol] != '\n' &&
		       contents[bol] != '\r'); bol--)
    ;
  if (bol != 0)
    bol++;
  return bol;
}

/* Find the end of the line containing pos; if pos is already newliney, that's
 * the end of the line; if there is no newliney character in the buffer
 * following pos, the length of the buffer is returned.
 */
int
TextViewer::findEOL(int pos)
{
  int eol;

  for (eol = pos; (eol < content_length &&
		   contents[eol] != '\n' &&
		   contents[eol] != '\r'); eol++)
    ;
  return eol;
}

/* Return the position in the buffer of the beginning of the previous line.
 * If there is no previous line, returns -1.
 */
int
TextViewer::prev_line(int pos)
{
  int bol = findBOL(pos);
  if (pos - bol > widthChars)
    pos = bol - widthChars;
  if (bol == 0)
    return -1;
  if (bol > 2 && contents[bol - 2] == '\r' && contents[bol - 1] == '\n')
    bol -= 2;
  else
    bol -= 1;
  return findBOL(bol);
}

/* Return the position in the buffer of the beginning of the next line; if
 * there is no next line, return -1.
 */

int
TextViewer::next_line(int pos)
{
  int eol = findEOL(pos);
  if (eol - pos > widthChars)
    eol = pos + widthChars;
  if (eol >= content_length)
    return -1;
  if (content_length - eol > 2 &&
      contents[eol] == '\r' && contents[eol + 1] == '\n')
    return eol + 2;
  if (content_length - eol > 1 &&
      (contents[eol] == '\r' || contents[eol] == '\n'))
    return eol + 1;
  return eol;
}

/* The window shape has changed in a way that means the lines we've computed
 * are no good, so we have to revalidate them.
 */
void
TextViewer::invalidate_lines(void)
{
  lines_t *lp = first, *nl;

  first = last = 0;

  /* Get rid of any existing lines. */
  for (; lp; lp = nl)
    {
      nl = lp->next;
      if (lp->thisLine)
	delete lp->thisLine;
      free(lp);
    }
  display_lines = -1;
}

/* Move up one line. */
void
TextViewer::up(void)
{
  int next;
  if (top == -1)
    {
      printf("no top to move up.\n");
      return;
    }
  next = prev_line(top);
  if (next == -1)
    {
      printf("no up to go.\n");
      return;
    }
  top = next;
  refill(width(), height());
  update();
}

/* Move up one line. */
void
TextViewer::down(void)
{
  int next;
  if (top == -1)
    {
      printf("no top to move down.\n");
      return;
    }
  next = next_line(top);
  if (next == -1)
    {
      printf("no down to go.\n");
      return;
    }
  top = next;
  refill(width(), height());
  update();
}

void
TextViewer::pageUp(void)
{
  int next;
  int i;
  if (top == -1 || display_lines == -1)
    {
      printf("no top to move up.\n");
      return;
    }
  for (i = 0; i < display_lines - 2; i++)
    {
      next = prev_line(top);
      if (next == -1)
	{
	  printf("no up to go.\n");
	  break;
	}
      top = next;
    }
  refill(width(), height());
  update();
}

void
TextViewer::pageDown(void)
{
  int next;
  int i;
  if (top == -1 || display_lines == -1)
    {
      printf("no top to move up.\n");
      return;
    }
  next = top;
  for (i = 0; i < display_lines - 2; i++)
    {
      next = next_line(next);
      if (next == -1)
	{
	  printf("no up to go.\n");
	  return;
	}
    }
  top = next;
  refill(width(), height());
  update();
}

void
TextViewer::goto_top(void)
{
  top = 0;
  refill(width(), height());
  update();
}

void
TextViewer::goto_bottom(void)
{
  top = content_length - 1;
  refill(width(), height());
  update();
}

/* Find the first line in the viewport that we need to show.  The
 * object-local variable /lines/ is the number of lines away from
 * centered that we want to be.
 */

/* The algorithm goes like this:
 *
 *  1. Find the vertical center of the viewport.
 *  2. Find the height and width of the viewport.
 *  3. The top of the viewport is 1/2 height/line-height lines above
 *     the center.
 *
 * This is complicated by the fact that we need to wrap lines.  So a
 * real line break is a newliney character sequence, but there will be
 * line breaks due to wrapping in the middle of a line if it's longer
 * than the width of the screen.
 *
 * So to handle this, we find the center and then figure out n lines
 * beyond it and n lines before it, where a line is a line_t
 * structure; that is, a wrapped line.
 */
   
void
TextViewer::refill(int nwidth, int nheight)
{
  int bol, eol;
  int y;
  lines_t *nl, *lp;

  /* If the contents have gone away, get rid of cached lines. */
  if (!contents)
    {
      invalidate_lines();
      initial_position = selection_start = selection_end = -1;
      return;
    }

#if 0
  y = plainMetrics->ascent();
  // If we don't have a top, make it midway between the two positions.
  if (top == -1)
    {
      if (!pos1)
	top = pos2;
      else if (!pos2)
	top = pos1;
      else if (pos1 > pos2)
	top = pos2 + (pos1 - pos2) / 2;
      else
	top = pos1 + (pos2 - pos1) / 2;
    }
  /* Guessing 40 characters per line, on average. */
  pageValue = 40 * height() / y;
  lastPageStart = content_length - pageValue;
  if (lastPageStart < 0)
    lastPageStart = 0;
  scrollbar->setMinimum(0);
  scrollbar->setMaximum(content_length);
  scrollbar->setPageStep(pageValue);
  scrollbar->setSingleStep(40);
  QObject::disconnect(scrollbar, SIGNAL(valueChanged(int)), this,
		   SLOT(scrollValueChanged(int)));
  scrollbar->setValue(top);
  QObject::connect(scrollbar, SIGNAL(valueChanged(int)), this,
		   SLOT(scrollValueChanged(int)));

  // Get rid of cached lines.
  invalidate_lines();
  bol = findBOL(top);
  eol = findEOL(top);
  nl = decompose_line(bol, eol, nwidth);
  first = last = nl;
  for (; last->next; last = last->next)
    ;

#ifdef DEBUG_REFILL
  printf("refill: initial line\n");
  dump_lines();
#endif

  // How many lines can we display?
  display_lines = nheight / plainMetrics->lineSpacing();

  append_lines((display_lines - 1) / 2, nwidth);
#ifdef DEBUG_REFILL
  printf("refill: after initial append:\n");
  dump_lines();
#endif

  // How many lines do we have now?
  int fc = 0;
  for (nl = first; nl; nl = nl->next)
    {
      if (nl->newLine)
	fc++;
    }
  // Prepend enough lines to fill the screen.
  prepend_lines(display_lines - fc, nwidth);
#ifdef DEBUG_REFILL
  printf("refill: after initial prepend.\n");
  dump_lines();
#endif

  /* It's possible at this point that there weren't enough lines up to
   * the beginning of the file to fill the screen; if that's the case,
   * try to get more lines from the end.  It's not a problem if there
   * are no more lines, though--that just means that the whole file
   * fits on the screen.
   */
  int sc = 0;
  for (nl = first; nl; nl = nl->next)
    {
      if (nl->newLine)
	sc++;
    }
  if (sc < display_lines)
    {
      append_lines(display_lines - sc, nwidth);
#ifdef DEBUG_REFILL
      printf("refill: did second append.\n");
      dump_lines();
#endif
    }

  /* At this point first is the first line we we want to display. */
  /* Set the y position of each line. */
  y = plainMetrics->lineSpacing();
  for (lp = first; lp; lp = lp->next)
    {
      lp->y = y;
      if (lp->newLine)
	y += plainMetrics->lineSpacing();
    }
#else
  if (top == -1)
    {
      if (pos1 && pos2)
	find_top(pos1, pos2 + posLen2);
      else if (pos1)
	find_top(pos1, pos1 + posLen1);
      else if (pos2)
	find_top(pos2, pos2 + posLen2);
      else
	top = 0;
    }

  /* Guessing 40 characters per line, on average. */
  pageValue = 40 * height() / plainMetrics->lineSpacing();
  lastPageStart = content_length - pageValue;
  if (lastPageStart < 0)
    lastPageStart = 0;
  QObject::disconnect(scrollbar, SIGNAL(valueChanged(int)), this,
		   SLOT(scrollValueChanged(int)));
  scrollbar->setMinimum(0);
  scrollbar->setMaximum(content_length);
  scrollbar->setPageStep(pageValue);
  scrollbar->setSingleStep(40);
  printf("setValue: %d\n", top);
  scrollbar->setValue(top);
  QObject::connect(scrollbar, SIGNAL(valueChanged(int)), this,
		   SLOT(scrollValueChanged(int)));
  // Get rid of cached lines.
  invalidate_lines();

  // How many lines can we display?
  display_lines = nheight / plainMetrics->lineSpacing();

  eol = findEOL(top);
  nl = decompose_line(top, eol, nwidth);
  first = last = nl;
  int num_lines = 0;
  for (nl = first; nl; nl = nl->next)
    {
      if (nl->newLine)
	num_lines++;
      if (!nl->next)
	last = nl;
    }
  if (num_lines < display_lines)
    {
      append_lines(display_lines - num_lines, nwidth);
#ifdef DEBUG_REFILL
      printf("refill: did second append.\n");
      dump_lines();
#endif
    }
  y = plainMetrics->lineSpacing();
  for (lp = first; lp; lp = lp->next)
    {
      lp->y = y;
      if (lp->newLine)
	y += plainMetrics->lineSpacing();
    }
#endif
}


void
TextViewer::resizeEvent(QResizeEvent *event)
{
  if (event->oldSize().width() != event->size().width())
    {
      printf("full\n");
      invalidate_lines();
      refill(event->size().width(), event->size().height());
      update();
    }
  else
    {
      printf("partial\n");
      refill(event->size().width(), event->size().height());
      update();
    }
}

void
TextViewer::setContents(char *data, int len,
			int initPos1, int ipLen1,
			int initPos2, int ipLen2)
{
  contents = data;
  content_length = len;
  pos1 = initPos1;
  pos2 = initPos2;
  posLen1 = ipLen1;
  posLen2 = ipLen2;
  top = -1;
  if (first)
    invalidate_lines();
  first = last = 0;
  initial_position = selection_start = selection_end = -1;
  refill(width(), height());
  update();
}

void
TextViewer::setViewChar(int newPos1, int npLen1, int newPos2, int npLen2)
{
  pos1 = newPos1;
  pos2 = newPos2;
  posLen1 = npLen1;
  posLen2 = npLen2;
  top = -1;
  refill(width(), height());
  update();
}

void
TextViewer::dump_lines()
{
  lines_t *lp;
  int i = 0;
  for (lp = first; lp; lp = lp -> next)
    {
      if (lp == last)
	printf("last:");
      else
	printf("%d: ", i);
      printf("%*.*s %s\n", lp->eol - lp->bol, lp->eol-lp->bol, &contents[lp->bol], lp->boldp ? "bold" : "normal");
      i++;
    }
  printf("\n");
}

/* Decompose some number of lines either before or after the current set of
 * cached lines.
 */

void
TextViewer::prepend_lines(int num, int nwidth)
{
  lines_t *nl, *lp;
  int bol, eol;
  int count;

  /* Get the beginning of the line before the first line in the cache. */
  bol = prev_line(first->bol);

  /* If there are no lines preceding the one we have, we're done. */
  if (bol == -1)
    return;
  eol = findEOL(bol);

  nl = decompose_line(bol, eol, nwidth);
  count = 1;
  for (lp = nl; lp->next; lp = lp->next)
    if (nl->newLine)
      count++;
  lp->next = first;
  first = nl;
  while (count > num)
    {
      lp = first->next;
      if (first->newLine)
	count--;
      free(first);
      first = lp;
    }

  /* If the line we just decomposed didn't give us enough lines, keep
   * fetching.
   */
  if (count < num)
    return prepend_lines(num - count, nwidth);
}

/* Decompose some number of lines either before or after the current set of
 * cached lines.
 */

void
TextViewer::append_lines(int num, int nwidth)
{
  lines_t *nl, *lp;
  int bol, eol;
  int count;

  /* Get the beginning of the line after the first line in the cache. */
  bol = findEOL(last->bol);
  if ((bol != content_length && contents[bol] == '\n'))
    bol++;
  else if (bol + 1 < content_length &&
	   contents[bol] == '\r' && contents[bol + 1] == '\n')
    bol += 2;

  /* If there are no lines preceding the one we have, we're done. */
  if (bol == -1)
    return;
  eol = findEOL(bol);

  nl = decompose_line(bol, eol, nwidth);
  count = 0;
  for (lp = nl; lp->next; lp = lp->next)
    if (nl->newLine)
      count++;
  if (nl->newLine)
    count++;
  free_lines_off_end(nl, count, num);
  last->next = nl;
  last = lp;

  /* If the line we just decomposed didn't give us enough lines, keep
   * fetching.
   */
  if (count < num)
    return append_lines(num - count, nwidth);
}

void
TextViewer::free_lines_off_end(lines_t *lines, int count, int num)
{
  if (!lines)
    return;
  if (num == 0)
    {
      free_lines_off_end(lines->next, 0, 0);
      free(lines->next);
      lines->next = 0;
      return;
    }
  if (lines->newLine)
      free_lines_off_end(lines->next, count - 1, num - 1);
  else
    free_lines_off_end(lines->next, count, num);
}

/* This is bog-stupid.   We just pick a start point that's at least half a
 * screen prior to begin, and a stop point that's half a screen after end,
 * and decompose that entire section into lines using an average character
 * width.
 *
 * Based on this approximate idea of the placement of begin and end on the
 * screen, we then decide the character position of the top of the screen.
 * We then simply decompose lines off the source until we have enough.
 */
void
TextViewer::find_top(int begin, int end)
{
# define MAX_HEIGHT 255	// Can't handle a screen taller than this many chars.
  int lbuf[MAX_HEIGHT];
  int i, j = 0;
  int heightChars = height() / plainMetrics->lineSpacing();
  int cwidth = plainMetrics->averageCharWidth();
  int start, stop;
  int curline = 0;
  int bline = -1, eline = -1;
  int line_width = 0;

  widthChars = width() / cwidth; // XXX
  start = begin - widthChars * heightChars / 2;
  if (start < 0)
    start = 0;
  stop = end + widthChars * heightChars / 2;
  if (stop > content_length)
    stop = content_length;
  lbuf[0] = start;
  
  for (i = start; i < stop; i++)
    {
      if (i == begin)
	bline = curline;
      if (i == end)
	eline = curline;

      /* If it's not printable, treat it as a space. */
      if (!isascii(contents[i]) || (!isprint(contents[i]) &&
				    contents[i] != '\n' &&
				    contents[i] != ' '))
	contents[i] = ' ';

      /* If we've hit a newline or we've filled up an entire line,
       * do the next line.
       */
      if (contents[i] == '\n' || line_width + 1 >= widthChars)
	{
	  // Don't remember the line after stop if it's immediately
	  // after stop.
	  if (i + 1 == stop)
	    break;
	  if (curline + 1 == MAX_HEIGHT)
	    {
	      // If we haven't hit the beginning yet, we can just
	      // reset the counter.
	      if (bline == -1)
		curline = 0;
	      // If we've already passed the end, we don't need to
	      // continue.
	      else if (eline != -1)
		break;
	      // If we have more lines than will fit on the screen,
	      // there's no way to display both matches, so we might
	      // as well stop.
	      else if (curline - bline > heightChars)
		break;
	      // Otherwise, we need to skip back.
	      memmove(lbuf, &lbuf[bline], (curline - bline) * sizeof (int));
	      curline -= bline;
	      bline = 0;
	    }
	  curline = curline + 1;
	  lbuf[curline] = i + 1; // remember where this line starts.
	  line_width = 0;
	}
      else
	line_width++;
    }

#ifdef DEBUG_FIND_TOP
  printf("lbuf <%d %d>:", begin, end);
  for (i = 0; i < curline; i++)
    {
      putchar(' ');
      if (i == bline)
	putchar('<');
      printf("%d", lbuf[i]);
      if (i == eline)
	putchar('>');
    }
  putchar('\n');
#endif

  /* At this point we may have both begin and end lines, or if the match
   * spanned a really long line, we might only have the begin line.   This
   * is essentially the same as if the end line is more than a screen height
   * past the begin line.   So we have to deal with just these two cases.
   */
  if (eline == -1 || eline - bline > heightChars)
    {
      /* In the case where we can't display both, center the beginning.
       * This is a degenerate case, not a legitimate situation,
       * so there's no need to try to put lipstick on the pig.   Ideally
       * we would count really long lines as multiple lines in the
       * near line match, but that's hard.
       */
      top = eline - heightChars / 2;
    }
  else
    {
      /* In this case, we'll place the begin line zero or more lines
       * down from the top--the distance from the top will be half
       * the height of the screen less the distance between the begin
       * and end lines, so that the begin line should be the same
       * distance from the bottom that the end line is from the top
       * (modulo fractions).
       */
      top = bline - ((heightChars - (eline - bline)) / 2);
#ifdef DEBUG_FIND_TOP
      printf("top = %d  <%d %d> heightChars = %d\n",
	     top, bline, eline, heightChars);
#endif
    }

  // These should never happen, but I'd have to do math to prove it.
  if (top < 0)
    top = 0;
  if (top >= MAX_HEIGHT)
    top = heightChars;
#ifdef DEBUG_FIND_TOP
  printf("top = %d\n", top);
#endif

  // lbuf between [0,heightChars) should be valid.
  top = lbuf[top];
#ifdef DEBUG_FIND_TOP
  printf("find_top(%d, %d) => %d\n", begin, end, top);
#endif
}

/* Given a position in the buffer of the beginning of the line, bol, and a
 * position in the buffer of the end of the line (which is either the position
 * after the last character in the buffer, or the position of the first
 * newliney character following bol), and given the current width of the
 * screen, nwidth, produce a linked list of line_t structures representing
 * the line as it needs to be broken to be displayed on a screen of the
 * specifed width.   Each line_t will have a y value that is the supplied
 * y value plus whatever height is necessary to display subsequent lines if
 * the current line must be broken.
 */

lines_t *
TextViewer::decompose_line(int bol, int eol, int nwidth)
{
  lines_t *lines = 0, *prev = 0, *nl;
  QString curLine = QString::fromUtf8(&contents[bol], eol - bol);
  int curLineChars = eol - bol;
  int x = 0;
  int socx = 0;
  int lpos = 0;
  bool noBreak, boldp;
  int space;

  /* Blank line. */
  if (lpos == curLineChars)
    {
      nl = (lines_t *)malloc(sizeof *nl);
      if (!nl)
	gofer_fatal("no memory for lines_t.");
      memset(nl, 0, sizeof *nl);
      nl->newLine = true;
      nl->boldp = false;
      nl->x = 0;
      nl->y = 0;
      nl->width = 0;
      nl->height = plainMetrics->lineSpacing();
      nl->bol = bol;
      nl->eol = bol + lpos;
      return nl;
    }

  while (lpos < curLineChars)
    {
      boldp = ((pos1 && bol >= pos1 && bol < pos1 + posLen1) ||
	       (pos2 && bol >= pos2 && bol < pos2 + posLen2));
      noBreak = false;

      space = lpos;

      while (x < nwidth && lpos < curLineChars)
	{
	  int cw;
	  if (curLine[lpos].isSpace())
	    space = lpos;
	  if (boldp)
	    cw = boldMetrics->charWidth(curLine, lpos);
	  else
	    cw = plainMetrics->charWidth(curLine, lpos);
	  if (cw + x > nwidth && lpos != space)
	    break;
	  if (lpos &&
	      ((pos1 && (bol + lpos == pos1 ||
			 bol + lpos == pos1 + posLen1)) ||
	       (pos2 && (bol + lpos == pos2 ||
			 bol + lpos == pos2 + posLen2))))
	    {
	      noBreak = true;
	      break;
	    }
	  x += cw;
	  lpos++;
	}
  
      if (!noBreak && lpos != curLineChars)
	{
	  if (space != 0)
	    {
	      if (space == eol)
	        lpos = space;
	      else
	        lpos = space + 1;
	    }
	}

      nl = (lines_t *)malloc(sizeof *nl);
      if (!nl)
	gofer_fatal("no memory for lines_t.");
      memset(nl, 0, sizeof *nl);

      nl->boldp = boldp;
      nl->x = socx;
      nl->y = 0;
      nl->width = x - socx;
      nl->height = plainMetrics->lineSpacing();
      nl->bol = bol;
      nl->eol = bol + lpos;
      nl->newLine = !noBreak;
      if (!noBreak)
	x = 0;
      socx = x;

      if (prev)
	prev->next = nl;
      else
	lines = nl;
      prev = nl;
  
      if (lpos != curLineChars)
	{
	  prev->thisLine = new QString(QString::fromUtf8(&contents[bol],
							 lpos));
	  curLine = QString::fromUtf8(&contents[bol + lpos],
				      curLineChars - lpos);
	  curLineChars -= lpos;
	  bol += lpos;
	  lpos = 0;
	}
      else
	{
	  prev->thisLine = new QString(curLine);
	  nl->newLine = true;
	}
    }
  return lines;
}

void
TextViewer::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);

  painter.setClipRegion(event->region(), Qt::ReplaceClip);
  paint(painter, first, 0);
}

void
TextViewer::paint(QPainter &painter, lines_t *begin, lines_t *end)
{
  lines_t *line;

  QBrush whiteBrush(Qt::white);
  QBrush blackBrush(Qt::black);
  QPen whitePen(QColor(Qt::white));
  QPen blackPen(QColor(Qt::black));

  painter.setBackgroundMode(Qt::OpaqueMode);

  /* Do the drawing. */
  for (line = begin; line != end; line = line->next)
    {
      if (line->thisLine)
	{
	  if (line->boldp)
	    painter.setFont(*boldFont);
	  else
	    painter.setFont(*plainFont);

	  /* If the current line contains both the selection start and
	   * the selection end, we need to draw three parts.   Sigh.
	   */

	  int x = line->x;
	  int ranges[3];
	  int colors[3];
	  int i = 0, j, pos = 0;
	  if (selection_start >= line->eol || selection_end <= line->bol)
	    {
	      ranges[i] = line->eol - line->bol;
	      colors[i] = 0;
	      i++;
	    }
	  else
	    {
	      if (selection_start > line->bol && selection_start < line->eol)
		{
		  ranges[i] = selection_start - line->bol;
		  colors[i] = 0;
		  i++;
		}
	      if (selection_end > line->bol)
		{
		  if (selection_end <= line->eol)
		    ranges[i] = selection_end -  line->bol;
		  else
		    ranges[i] = line->eol -  line->bol;
		  colors[i] = 1;
		  i++;
		}
	      if (selection_end < line->eol)
		{
		  ranges[i] = line->eol - line->bol;
		  colors[i] = 0;
		  i++;
		}
	    }
	  QString segment;
	  for (j = 0; j < i; j++)
	    {
	      int len = ranges[j] - pos;
	      if (len > 0)
		{
		  segment = line->thisLine->mid(pos, len);
		  if (colors[j])
		    {
		      painter.setBackground(blackBrush);
		      painter.setPen(whitePen);
		    }
		  else
		    {
		      painter.setBackground(whiteBrush);
		      painter.setPen(blackPen);
		    }
		  painter.drawText(x, line->y, segment);
		  if (line->boldp)
		    x += boldMetrics->width(segment);
		  else
		    x += plainMetrics->width(segment);
		}
	      pos = ranges[j];
	    }
	}
    }
}

void
TextViewer::updateSelection(void)
{
  if (selection_start == -1 || selection_end == -1)
    return;
  
  repaint();
  application->processEvents();
}

void
TextViewer::locate(bool endp, int x, int y)
{
  lines_t *lp;
  int i;
  int width;
  if (x < 0)
    x = 0;

  if (y < 0)
    {
      if (scrollTimerId != -1 && !scrollingDown)
	return;
      up();
      scrollingDown = false;
      scrollTimerId = startTimer(100);
      return;
    }
  /* Figure out what line we're on... */
  for (lp = first; lp; lp = lp->next)
    {
      if (lp->y - lp->height <= y && lp->y > y)
	{
	  if (lp->newLine)
	    break;
	  if (lp->x <= x && lp->x + lp->width > x)
	    {
	      break;
	    }
	}
      if (!lp->next)
	break;
    }
  if (y > height())
    {
      if (scrollTimerId != -1)
	return;
      down();
      scrollingDown = true;
      scrollTimerId = startTimer(100);
      return;
    }
  if (scrollTimerId != -1)
    {
      killTimer(scrollTimerId);
      scrollTimerId = -1;
    }

  /* To figure out what character position we're at, we basically have two
   * choices - we can walk linearly across the string noticing how wide
   * the string is in pixels at a particular length in characters, until we
   * come to a width that's past the mouse's x position; the character that
   * pushed us over the edge is the one.
   *
   * Or we can do a binary search to find the same endpoint.   That's faster,
   * but harder to code correctly.   What to do, what to do...?
   */

  for (i = 0; i < lp->eol - lp->bol; i++)
    {
      if (lp->boldp)
	width = boldMetrics->width(*lp->thisLine, i);
      else
	width = plainMetrics->width(*lp->thisLine, i);
      char foo[256];
      snprintf(foo, 255, "i = %d  lp->x = %d  width = %d  x = %d\n",
	      i, lp->x, width, x);
      setStatusTip(tr(foo));
      if (lp->x + width > x)
	break;
    }
  extend_selection(endp, lp, lp->bol + i);
}

void
TextViewer::extend_selection(bool endp, lines_t *lp, int position)
{
  if (!endp)
    {
      selection_start = position;
      selection_end = selection_start;
      initial_position = selection_start;
    }
  else
    {
      if (position <= initial_position)
	{
	  if (position > 0)
	    position = position - 1;
	  selection_start = position;
	  selection_end = initial_position;
	}
      else
	{
	  selection_start = initial_position - 1;
	  selection_end = position;
	}
    }
  if (selection_start != -1 && selection_end != -1)
      updateSelection();
}

void
TextViewer::mousePressEvent(QMouseEvent *event)
{
  /* If there is already a selection, clicking again clears the selection. */
  if (selection_start != -1)
    {
      selection_start = selection_end = initial_position = -1;
      update();
    }
  locate(false, event->x(), event->y());
  selecting = true;
}

void
TextViewer::mouseMoveEvent(QMouseEvent *event)
{
  if (!selecting)
    return;
  locate(true, event->x(), event->y());
}

void
TextViewer::mouseReleaseEvent(QMouseEvent *event)
{
  if (!selecting)
    return;
  locate(true, event->x(), event->y());
  if (scrollTimerId != -1)
    {
      killTimer(scrollTimerId);
      scrollTimerId = -1;
    }
  selecting = false;
}

void
TextViewer::timerEvent(QTimerEvent *event)
{
  if (scrollingDown)
    down();
  else
    up();
}

void
TextViewer::scrollValueChanged(int value)
{
  int bol;
  printf("valueChanged: %d\n", value);
  top = value;
  if (top < 0)
    top = 0;
  else if (top > lastPageStart)
    top = lastPageStart;
  bol = findBOL(top);
  if (top - bol <= widthChars)
    top = bol;
  refill(width(), height());
  update();
}

void
TextViewer::scrollActionTriggered(int action)
{
  switch(action)
    {
    case QAbstractSlider::SliderSingleStepAdd:
      printf("sliderSingleStepAdd\n");
      down();
      break;
    case QAbstractSlider::SliderSingleStepSub:
      printf("sliderSingleStepSub\n");
      up();
      break;
    case QAbstractSlider::SliderPageStepAdd:
      printf("sliderPageStepAdd\n");
      pageDown();
      break;
    case QAbstractSlider::SliderPageStepSub:
      printf("sliderPageStepSub\n");
      pageUp();
      break;
    case QAbstractSlider::SliderToMinimum:
      printf("sliderToMinimum\n");
      top = 0;
      refill(width(), height());
      update();
      break;
    case QAbstractSlider::SliderToMaximum:
      printf("sliderToMaximum\n");
      top = findBOL(lastPageStart);
      refill(width(), height());
      update();
      break;
    case QAbstractSlider::SliderMove:
      printf("sliderMove\n");
      break;
    default:
      printf("slider: %d\n", action);
      break;
    }
}

void
TextViewer::wheelEvent(QWheelEvent *event)
{
  int distance = event->delta() / 15;
  int i;
  int next;

  if (distance < 0)
    {
      for (i = 0; i > distance; i--)
	{
	  next = prev_line(top);
	  if (next == -1)
	    {
	      printf("scrolling above the top.\n");
	      break;
	    }
	  top = next;
	}
    }
  else
    {
      next = top;
      for (i = 0; i < distance; i++)
	{
	  next = next_line(next);
	  if (next == -1)
	    {
	      printf("scrolling below the bottom.\n");
	      return;
	    }
	}
      top = next;
    }
  refill(width(), height());
  update();
}

void
TextViewer::leaveEvent(QEvent *event)
{
  printf("the mouse has left the viewport.\n");
}

void
TextViewer::focusInEvent(QFocusEvent *event)
{
  printf("we have focus.\n");
  haveFocus = true;
}

void
TextViewer::focusOutEvent(QFocusEvent *event)
{
  printf("we lost focus.\n");
  haveFocus = false;
}

bool
TextViewer::maybeCopy(bool withName)
{
  if (!haveFocus)
    {
      printf("We don't have focus.\n");
      return false;
    }
  if (selection_start == -1)
    {
      printf("We don't have a selection.\n");
      return false;
    }
  QClipboard *clip = application->clipboard();
  QString clipText = QString::fromUtf8(&contents[selection_start],
				       selection_end - selection_start);
  QMimeData *mimeData = new QMimeData;
  if (withName)
      clipText = filename + tr(":\n") + clipText;
  mimeData->setText(clipText);
  mimeData->setHtml(tr("<span style=\"font-style: normal; "
		       "font-weight: normal;\">") + clipText + tr("</span>"));
  clip->setMimeData(mimeData);
  printf("set the clipboard.\n");
  return true;
}

void
TextViewer::setFilename(const char *name)
{
  filename = QString::fromUtf8(name);
}

/* Local Variables:  */
/* mode:c++ */
/* c-file-style:"gnu" */
/* end: */
