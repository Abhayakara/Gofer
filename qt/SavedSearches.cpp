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

#include <QObject>

#include "SearchState.h"
#include "SavedSearches.h"

SavedSearches::SavedSearches()
{
}

SavedSearches::~SavedSearches()
{
  int i;

  for (i = states.size() - 1; i >= 0; i++)
    {
      SearchState *s = states.takeAt(i);
      delete s;
    }
}

void SavedSearches::save(QString *name, SearchState *proto)
{
  SearchState *my_copy = new SearchState(proto);
  int i;

  /* If there's already one by that name, replace it. */
  for (i = 0; i < states.size(); i++)
    {
      QString *oldName = states[i]->name();
      if ((*oldName) == (*name))
	{
	  SearchState *old = states[i];
	  states[i] = my_copy;
	  delete old;
	  return;
	}
    }

  states.append(my_copy);
  emit searchSaved(name);
}

void SavedSearches::remove(int index)
{
  SearchState *old;
  if (index < 0 || index >= states.size())
    return;

  old = states[index];
  states.removeAt(index);
  delete old;

  emit searchRemoved(index);
}

SearchState *SavedSearches::get(int index)
{
  SearchState *copy;
  if (index < 0 || index >= states.size())
    return 0;

  copy = new SearchState(states[index]);
  return copy;
}

int SavedSearches::numSearches()
{
  return states.size();
}

void SavedSearches::saveSettings(QSettings &settings)
{
  int i;

  settings.beginWriteArray("savedSearches");
  for (i = 0; i < states.size(); i++)
    {
      settings.setArrayIndex(i);
      states[i]->saveSettings(settings);
    }
  settings.endArray();
}

void SavedSearches::loadSettings(QSettings &settings)
{
  int i;
  int max;

  max = settings.beginReadArray("savedSearches");
  for (i = 0; i < max || (i == max && max == 0); i++)
    {
      SearchState *reload;
      settings.setArrayIndex(i);
      reload = SearchState::loadState(settings);
      if (reload)
	{
	  states.append(reload);
	  emit searchSaved(reload->name());
	}
    }
  settings.endArray();
}

/* Local Variables:  */
/* mode:c++ */
/* c-file-style:"gnu" */
/* end: */
