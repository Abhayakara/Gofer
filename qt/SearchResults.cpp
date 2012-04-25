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

#include <QtGui>
#include "SearchResults.h"
#include "TextViewer.h"

SearchResults::SearchResults(Gofer *gofer)
{
  files = new_filelist();
  firstMatch = 1;
  nextMatchButton = 0;
  nextFileMatchButton = 0;
  viewer = 0;
  fileName = 0;
  curFileName = 0;
  goferWindow = gofer;
}

/* We really don't want to leave the files structure dangling... */
SearchResults::~SearchResults()
{
  if (files)
    filelist_free(files);
}

void SearchResults::setViewer(TextViewer *vwr)
{
  viewer = vwr;
}

void SearchResults::setNameView(QLineEdit *name)
{
  fileName = name;
}

void SearchResults::setMatchButtons(QPushButton *next,
				    QPushButton *nextFile,
				    QPushButton *prev,
				    QPushButton *prevFile)
{
  nextMatchButton = next;
  nextFileMatchButton = nextFile;
  prevMatchButton = prev;
  prevFileMatchButton = prevFile;
}

void SearchResults::addResult(const char *fname,
			      char *contents,
			      int content_length,
			      int first_line,
			      int last_line,
			      int first_char,
			      int first_len,
			      int last_char,
			      int last_len,
			      int *cur_line,
			      int *cur_char)
{
  new_entry(files, fname, contents, content_length, first_line, last_line,
	    first_char, first_len, last_char, last_len, cur_line, cur_char);
  if (firstMatch) {
    firstMatch = 0;
    showCurMatch();
  } else {
    if (canNext())
      {
	nextMatchButton->setEnabled(true);
      }
    else
      {
	nextMatchButton->setEnabled(false);
	goferWindow->setStatusMessage(tr("No further matches."));
      }
    nextFileMatchButton->setEnabled(canNextFile());
  }
}

void SearchResults::zapMatchView()
{
  setViewFile(tr("Searching..."));
  viewer->setContents("", 0, 0, 0, 0, 0);
  haveContents = false;
}

/* XXX this can probably go away unless Qt has the same broken thread
 * XXX behavior as Cocoa.
 */
void SearchResults::zapMatchViewOnMainThread()
{
  zapMatchView();
}

void SearchResults::showCurMatch()
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
      setViewFile(QString::QString(curFileName));
      viewer->setFilename(curFileName);
      viewer->setContents(curFile->contents, curFile->content_length,
			  mz->first_char, mz->first_len,
			  mz->last_char, mz->last_len);
      haveContents = true;
    }
  else
    seekMatch();
}

void SearchResults::seekMatch()
{
  fileresults_t *curFile;
  matchzone_t *mz;

  if (files->cur_file >= files->nfiles)
    return;
  curFile = files->files[files->cur_file];
  if (curFile->curzone >= curFile->nzones)
    return;
  mz = curFile->matches[curFile->curzone];

  viewer->setViewChar(mz->first_char, mz->first_len,
		      mz->last_char, mz->last_len);
  if (canNext())
    nextMatchButton->setEnabled(true);
  else
    {
      nextMatchButton->setEnabled(false);
      goferWindow->setStatusMessage(tr("No further matches."));
    }
  nextFileMatchButton->setEnabled(canNextFile());
}

bool SearchResults::canNext()
{
  fileresults_t *curFile;
  if (files->cur_file >= files->nfiles)
    {
      return false;
    }
  curFile = files->files[files->cur_file];
  if (curFile->curzone + 1 >= curFile->nzones)
    {
      if (files->cur_file + 1 >= files->nfiles)
	{
	  return false;
	}
    }
  return true;
}

bool SearchResults::canNextFile() {
  if (files->cur_file + 1 >= files->nfiles)
    {
      return false;
    }
  return true;
}

void SearchResults::nextMatch()
{
  fileresults_t *curFile;
  if (!files || files->cur_file >= files->nfiles)
    {
      goferWindow->setStatusMessage(tr("No further matches."));
      nextMatchButton->setEnabled(false);
      nextFileMatchButton->setEnabled(false);
      return;
    }

  prevMatchButton->setEnabled(true);
  curFile = files->files[files->cur_file];
  if (curFile->curzone + 1 >= curFile->nzones)
    {
      if (files->cur_file + 1 < files->nfiles)
	{
	  nextFileMatch();
	}
      else
	{
	  nextMatchButton->setEnabled(false);
	  nextFileMatchButton->setEnabled(false);
	  goferWindow->setStatusMessage(tr("No further matches."));
	}
      return;
    }
  else
    {
      unShowMatch();
      curFile->curzone++;
    }
  showCurMatch();
}

void SearchResults::nextFileMatch()
{
  if (!files || files->cur_file + 1 >= files->nfiles)
    {
      nextFileMatchButton->setEnabled(false);
      return;
    }

  unShowMatch();
  files->cur_file++;
  files->files[files->cur_file]->curzone = 0;
  showCurMatch();
  prevFileMatchButton->setEnabled(true);
  if (files->cur_file + 1 >= files->nfiles)
    nextFileMatchButton->setEnabled(false);
}

void SearchResults::prevMatch()
{
  fileresults_t *curFile;
  if (!files || files->cur_file >= files->nfiles)
    {
      prevMatchButton->setEnabled(false);
      return;
    }

  nextMatchButton->setEnabled(true);
  curFile = files->files[files->cur_file];
  if (curFile->curzone == 0)
    {
      prevFileMatch();
      return;
    }
  else
    {
      unShowMatch();
      curFile->curzone--;
    }
  showCurMatch();
}

void SearchResults::prevFileMatch()
{
  if (!files || files->cur_file == 0)
    {
      prevFileMatchButton->setEnabled(false);
      return;
    }

  nextFileMatchButton->setEnabled(true);
  unShowMatch();
  files->cur_file--;
  if (files->files[files->cur_file]->nzones > 0)
    {
      files->files[files->cur_file]->curzone =
	  files->files[files->cur_file]->nzones - 1;
      showCurMatch();
    }
}

void SearchResults::setViewFile(QString name)
{
  //  if (name.size() > 80)
  //    name = name.right(80);
  printf ("setViewFile: %s\n", name.toUtf8().constData());
  fileName->setText(name);
}

void SearchResults::unShowMatch()
{
  fileresults_t *curFile;
  matchzone_t *mz;

  if (files->cur_file >= files->nfiles)
    return;
  curFile = files->files[files->cur_file];
  if (curFile->curzone >= curFile->nzones)
    return;
  mz = curFile->matches[curFile->curzone];

  viewer->setViewChar(mz->first_char, mz->first_len,
		      mz->last_char, mz->last_len);
}

/* Local Variables:  */
/* mode:c++ */
/* c-file-style:"gnu" */
/* end: */
