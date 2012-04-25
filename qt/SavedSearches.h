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

#ifndef SAVEDSEARCHES_H
#define SAVEDSEARCHES_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>
#include "st.h"

class SearchState;
class QSettings;

class SavedSearches : public QObject
{
  Q_OBJECT

public:
  SavedSearches();
  ~SavedSearches();
  void save(QString *name, SearchState *proto);
  void remove(int index);
  SearchState *get(int index);
  int numSearches();
  void saveSettings(QSettings &settings);
  void loadSettings(QSettings &settings);
private slots:

signals:
  void searchSaved(QString *state);
  void searchRemoved(int index);

private:
  QList<SearchState *> states;
};

#endif

/* Local Variables:  */
/* mode:c++ */
/* c-file-style:"gnu" */
/* end: */
