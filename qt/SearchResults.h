/* Copyright (c) 2004-2006 Edward W. Lemon III
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

#ifndef SEARCHRESULTS_H
#define SEARCHRESULTS_H

#include <QObject>
#include "Gofer.h"
#include "filelist.h"

class QTextEdit;
class QLabel;
class QLineEdit;
class QPushButton;
class QTextCursor;
class TextViewer;

class SearchResults : public QObject
{
    Q_OBJECT

public:
    SearchResults(Gofer *gofer);
    ~SearchResults();
    void setViewer(TextViewer *vwr);
    void setNameView(QLineEdit *name);
    void setMatchButtons(QPushButton *next, QPushButton *nextFile,
			 QPushButton *prev, QPushButton *prevFile);
    void addResult(const char *fname, char *contents, int content_length,
		   int first_line, int last_line, int first_char, int first_len,
		   int last_char, int last_len, int *cur_line, int *cur_char);
    void zapMatchView();
    void zapMatchViewOnMainThread();
    void showCurMatch();
    void setViewFile(QString name);
    void setViewContents(QString &contents);
    void prevMatch();
    void prevFileMatch();
    void nextMatch();
    void nextFileMatch();
    bool haveContents;
    
private:
    void seekMatch();
    bool canNext();
    bool canNextFile();
    void highlightRegion(int start, int len, bool on);
    void setViewChar(int first, int flen, int last, int llen);
    void unShowMatch();

    filelist_t *files;
    QPushButton *nextMatchButton, *nextFileMatchButton,
	    *prevMatchButton, *prevFileMatchButton;
    TextViewer *viewer;
    QLineEdit *fileName;
    int firstMatch;
    char *curFileName;
    Gofer *goferWindow;
};
#endif
