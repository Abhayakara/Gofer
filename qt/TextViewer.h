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

#ifndef TEXTVIEWER_H
#define TEXTVIEWER_H

#include <QWidget>
#include <QString>
#include <QScrollBar>

typedef struct lines {
  struct lines *next;
  bool boldp, newLine;
  int x, y, bol, eol;
  int width, height;
  QString *thisLine;
} lines_t;


class TextViewer : public QWidget
{
  Q_OBJECT

public:
  TextViewer(QScrollBar *scrollBar);
  ~TextViewer();
  static void setApplication(QApplication *app);
  void setContents(char *data, int len,
		   int initPos1, int ipLen1, int initPos2, int ipLen2);
  void setViewChar(int newPos1, int npLen1, int newPos2, int npLen2);
  void up(void);
  void down(void);
  void pageUp(void);
  void pageDown(void);
  void goto_bottom(void);
  void goto_top(void);
  bool maybeCopy(bool withName);
  void setFilename(const char *name);

protected:
  void resizeEvent(QResizeEvent *event);
  void paintEvent(QPaintEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void leaveEvent(QEvent *event);
  void focusInEvent(QFocusEvent *event);
  void focusOutEvent(QFocusEvent *event);
  void timerEvent(QTimerEvent *);
  void wheelEvent(QWheelEvent *event);

private slots:
  void scrollValueChanged(int value);
  void scrollActionTriggered(int action);

private:
  void paint(QPainter &painter, lines_t *start, lines_t *end);
  void font_setup(void);
  void invalidate_lines(void);
  lines_t *decompose_line(int bol, int eol, int nwidth);
  void prepend_lines(int num, int nwidth);
  void append_lines(int num, int nwidth);
  int findBOL(int pos);
  int findEOL(int pos);
  int prev_line(int pos);
  int next_line(int pos);
  void refill(int nwidth, int nheight);
  void updateSelection(void);
  void locate(bool endp, int x, int y);
  void extend_selection(bool endp, lines_t *lp, int point);
  void free_lines_off_end(lines_t *lines, int count, int num);
  void dump_lines(void);
  void find_top(int begin, int end);

  char *contents;
  int content_length;
  int pos1, posLen1;
  int pos2, posLen2;
  int top;
  int display_lines;
  int selection_start;
  int selection_end;
  int initial_position;
  lines_t *first, *last;
  int selecting;
  QFont *plainFont;
  QFont *boldFont;
  QFontMetrics *plainMetrics;
  QFontMetrics *boldMetrics;
  bool haveFocus;
  QString filename;
  int scrollTimerId;
  bool scrollingDown;
  QScrollBar *scrollbar;
  int pageValue;
  int lastPageStart;
  int widthChars;

  static QApplication *application;
};

#endif

/* Local Variables:  */
/* mode:c++ */
/* c-file-style:"gnu" */
/* end: */
