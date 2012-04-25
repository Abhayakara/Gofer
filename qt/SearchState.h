/* Copyright (c) 2006 Edward W. Lemon III
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

#ifndef SEARCHSTATE_H
#define SEARCHSTATE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "st.h"

class SavedSearches;
class QSettings;

class SearchState : public QObject
{
  Q_OBJECT

public:
  SearchState();
  SearchState(SearchState *);
  static SearchState *loadState(QSettings &);
  ~SearchState();
  st_expr_t *genEquation();
  void setText(int ix, QString newText);
  void setExactitude(st_match_type_t newExactitude);
  void setNearness(int newNearness);
  void setCombiner(st_expr_type_t newCombiner);
  void addSearchDir(const QString &searchDir, bool checked);
  void updateSearchDir(int index, const QString &searchDir, bool checked);
  void removeSearchDir(int index);
  int numSearchDirs();
  QString searchDir(int index);
  bool dirIsChecked(int index);
  st_match_type_t exactitude();
  int nearness();
  st_expr_type_t combiner();
  QString *name();
  QString *text(int index);
  void save(QString name);
  void setName(QString name);
  SavedSearches *saved();
  bool canSearch();
  void saveSettings(QSettings &settings);

private slots:

signals:

private:
  static SavedSearches *savedSearches;

  /* Name of this saved search, if any. */
  QString *myName;

  /* Contents of each text box. */
  QString *myText[8];

  /* Match type. */
  st_match_type_t myExactitude;

  /* Combiner. */
  st_expr_type_t myCombiner;

  /* If expression is ste_near or ste_near_lines, how many lines. */
  int myNearness;

  /* List of directories to search. */
  QStringList searchList;
  QList<bool> searchListChecked;
};

#endif

/* Local Variables:  */
/* mode:c++ */
/* c-file-style:"gnu" */
/* end: */
