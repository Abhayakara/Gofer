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

#include "SearchState.h"
#include "SavedSearches.h"

SavedSearches *SearchState::savedSearches;

SearchState::SearchState()
{
  myName = 0;
  memset(myText, 0, sizeof myText);
  myCombiner = ste_near_lines;
  myExactitude = match_ignores_spaces_and_case;
  myNearness = 3;
  if (!SearchState::savedSearches)
    SearchState::savedSearches = new SavedSearches;
}

SearchState::SearchState(SearchState *proto)
{
  int i;
  QString *newName;

  newName = proto->name();
  if (newName)
    myName = new QString(*newName);
  else
    myName = 0;

  for (i = 0; i < 8; i++)
    {
      QString *s = proto->text(i);
      if (s)
	myText[i] = new QString(*s);
      else
	myText[i] = 0;
    }
  myCombiner = proto->combiner();
  myExactitude = proto->exactitude();
  myNearness = proto->nearness();

  for (i = 0; i < proto->numSearchDirs(); i++)
    {
      searchList.append(QString::QString(proto->searchDir(i)));
      searchListChecked.append(proto->dirIsChecked(i));
    }

  if (!SearchState::savedSearches)
    SearchState::savedSearches = new SavedSearches;
}

SearchState *SearchState::loadState(QSettings &settings)
{
  int i;
  int max;
  QVariant v;
  bool ok = 0;
  SearchState *rv;
  int nearness = settings.value("nearness").toInt(&ok);

  if (!ok)
    return 0;

  rv = new SearchState();
  rv->setName(settings.value("name").toString());
  rv->setExactitude((st_match_type_t)settings.value("exactitude").toInt());
  rv->setNearness(nearness);
  rv->setCombiner((st_expr_type_t)settings.value("combiner").toInt());

  max = settings.beginReadArray("text");
  for (i = 0; i < 8 || (i == 0 && max == 0); i++)
    {
      QString text;
      settings.setArrayIndex(i);
      text = settings.value("value").toString();
      if (text.size())
	rv->setText(i, text);
    }
  settings.endArray();

  max = settings.beginReadArray("dirs");
  for (i = 0; i < max || (i == 0 && max == 0); i++)
    {
      bool checked;
      QString dir;
      settings.setArrayIndex(i);
      checked = settings.value("checked").toBool();
      dir = settings.value("name").toString();
      if (dir.size())
	rv->addSearchDir(QString::QString(dir), checked);
    }
  settings.endArray();

  return rv;
}

SearchState::~SearchState()
{
  int i;

  for (i = 0; i < 8; i++)
    {
      if (myText[i])
	delete myText[i];
    }
  if (myName)
    delete myName;

  /* If a SavedSearches object is referring to this object, the only
   * way this object can be deleted is by the SavedSearches object, so
   * we don't need to do anything here to make sure that anybody has
   * let go of this SearchState object.
   */
}

st_expr_t *SearchState::genEquation()
{
  st_expr_t *expr, *lhs, *rhs, *n1, *n2;
  search_term_t *ms;
  int i;
  int offset;
  //  char *pe;
  
  lhs = rhs = 0;
  
  /* There are eight input areas for entering match strings.   The first
   * four are joined together with ors, if values are present in them.
   * The second four are as well.   So for each set of four, generate an
   * expression that ors all the terms that have values; put the expression
   * for the top four into lhm, and the expression for the bottom four into
   * rhm.
   */
  
  for (offset = 0; offset < 8; offset += 4)
    {
      expr = 0;
      for (i = offset; i < offset + 4; i++)
	{
	  if (myText[i] && myText[i]->size())
	    {
	      ms = (search_term_t *)malloc(sizeof (search_term_t));
	      if (!ms)
		gofer_fatal("no memory for matchset %s",
			    myText[i]->toUtf8().constData());
	      memset(ms, 0, sizeof (search_term_t));
	      ms->len = myText[i]->size();
	      if (ms->len > ST_LIMIT)
		ms->len = ST_LIMIT;
	      memcpy(ms->buf, myText[i]->toUtf8().constData(), ms->len);
	      
	      n1 = (st_expr_t *)malloc(sizeof *n1);
	      if (!n1)
		gofer_fatal("no memory for matchset expr for %s",
			    myText[i]->toUtf8().constData());
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
        }
        if (offset == 0)
	  lhs = expr;
	else
	  rhs = expr;

    }

  /* If necessary, fetch exactitude from thingy. */

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
      expr->type = myCombiner;
      expr->subexpr.exprs[0] = lhs;
      expr->subexpr.exprs[1] = rhs;
      if (myCombiner == ste_near || myCombiner == ste_near_lines)
	expr->n = myNearness;
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

  //  if (expr)
  //    {
  //      pe = print_expr(expr);
  //      if (pe)
  //        expression->setText(QString::fromUtf8(pe, strlen(pe)));
  //    }

  return expr;
}

void SearchState::setText(int ix, QString newText)
{
  if (ix >= 8 || ix < 0)
    return;
  if (myText[ix])
    delete myText[ix];
  myText[ix] = new QString(newText);
}

void SearchState::setExactitude(st_match_type_t newExactitude)
{
  myExactitude = newExactitude;
}

void SearchState::setNearness(int newNearness)
{
  myNearness = newNearness;
}

void SearchState::setCombiner(st_expr_type_t newCombiner)
{
  myCombiner = newCombiner;
}

void SearchState::addSearchDir(const QString &searchDir, bool checked)
{
  searchList.append(QString::QString(searchDir));
  searchListChecked.append(checked);
}

void SearchState::updateSearchDir(int index,
				  const QString &searchDir, bool checked)
{
  if (index < 0 || index >= searchList.size())
    return;
  searchList[index] = QString::QString(searchDir);
  searchListChecked[index] = checked;
}

void SearchState::removeSearchDir(int index)
{
  if (index < 0 || index >= searchList.size())
    return;
  searchList.removeAt(index);
  searchListChecked.removeAt(index);
}

int SearchState::numSearchDirs()
{
  return searchList.size();
}

QString SearchState::searchDir(int index)
{
  if (index < 0 || index >= searchList.size())
    return 0;
  return searchList[index];
}

bool SearchState::dirIsChecked(int index)
{
  if (index < 0 || index >= searchListChecked.size())
    return false;
  return searchListChecked[index];
}

st_match_type_t SearchState::exactitude()
{
  return myExactitude;
}

int SearchState::nearness()
{
  return myNearness;
}

st_expr_type_t SearchState::combiner()
{
  return myCombiner;
}

QString *SearchState::name()
{
  return myName;
}

QString *SearchState::text(int index)
{
  if (index < 0 || index >= 8)
    return 0;
  return myText[index];
}

void SearchState::save(QString newName)
{
  if (myName)
    delete myName;
  myName = new QString(newName);
  savedSearches->save(myName, this);
}

void SearchState::setName(QString newName)
{
  if (myName)
    delete myName;
  myName = new QString(newName);
}

SavedSearches *SearchState::saved()
{
  return savedSearches;
}

bool SearchState::canSearch()
{
  int i;

  if (searchList.size() == 0)
    return false;

  for (i = 0; i < 8; i++)
    {
      if (myText[i] && myText[i]->size() != 0)
	return true;
    }
  return false;

}

void SearchState::saveSettings(QSettings &settings)
{
  int i;

  settings.setValue("name", *myName);

  settings.beginWriteArray("text");
  for (i = 0; i < 8; i++)
    {
      settings.setArrayIndex(i);
      if (myText[i])
	settings.setValue("value", *myText[i]);
    }
  settings.endArray();

  settings.beginWriteArray("dirs");
  for (i = 0; i < searchList.size(); i++)
    {
      settings.setArrayIndex(i);
      settings.setValue("checked", searchListChecked[i]);
      settings.setValue("name", searchList[i]);
    }
  settings.endArray();

  settings.setValue("exactitude", (int)myExactitude);
  settings.setValue("nearness", myNearness);
  settings.setValue("combiner", (int)myCombiner);
}

/* Local Variables:  */
/* mode:c++ */
/* c-file-style:"gnu" */
/* end: */
