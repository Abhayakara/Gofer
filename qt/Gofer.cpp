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

extern "C" {
  #include "st.h"
}

#include "gofer.h"
#include "SearchResults.h"
#include "SearchState.h"
#include "ItemSlotReceiver.h"
#include "SavedSearches.h"
#include "TextViewer.h"

QApplication *Gofer::application;
QList<Gofer *>Gofer::goferList;

Gofer::Gofer()
{
    QHBoxLayout *hbox;
    QVBoxLayout *vbox;
    int i, j;
    QFrame *frame;
    SavedSearches *svs;
    QScrollBar *scrollbar;

    /* Get a search state object. */
    state = new SearchState;
    svs = state->saved();

    initializing = true;

    connect(svs, SIGNAL(searchSaved(QString *)),
	    this, SLOT(savedSearchAdd(QString *)));
    connect(svs, SIGNAL(searchRemoved(int)),
	    this, SLOT(savedSearchRemove(int)));

    tabWidget = new QTabWidget;
    tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		      connect(this, SIGNAL(setTabIndex(int)),
	    tabWidget, SLOT(setCurrentIndex(int)));

    setFocusPolicy(Qt::StrongFocus);

    /* Make the vbox for the first tab. */
    vbox = new QVBoxLayout;
    frame = new QFrame;
    frame->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    frame->setLayout(vbox);

    tabWidget->addTab(frame, tr("Search Settings"));

    topLabel = new QLabel;
    topLabel->setText("Enter Text to Go For");
    vbox->addWidget(topLabel);

    /* Make four rows of two text input boxes side by side.   After each
     * second row, put in a combobox.
     */
    for (i = 0; i < 2; i++)
      {
	for (j = 0; j < 2; j++)
	  {
	    int k = i * 4 + j * 2;
	    int l;
	    hbox = new QHBoxLayout;
	    for (l = 0; l < 2; l++)
	      {
		QLineEdit *te = new QLineEdit;
		ItemSlotReceiver *is = new ItemSlotReceiver;
		is->setId(k + l);
		connect(te, SIGNAL(editingFinished()),
			is, SLOT(textChanged()));
		connect(is, SIGNAL(textChangedId(int)),
			this, SLOT(textEntered(int)));
		hbox->addWidget(te);
		inputBoxes[k + l] = te;
		inputSlots[k + l] = is;
	      }
	    vbox->addLayout(hbox);
	}
	if (!i)
	  {
	    QIntValidator *validator;
	    QLabel *label;

	    hbox = new QHBoxLayout;

	    combineDistance =
	      new QLineEdit(QString::number(state->nearness()));
	    validator = new QIntValidator(0, 1000, combineDistance);
	    connect(combineDistance, SIGNAL(editingFinished()),
		    this, SLOT(newCombineDistance()));

	    combineCombo = new QComboBox;
	    combineCombo->setEditable(false);
	    combineCombo->addItem(tr("OR"), QVariant::QVariant(ste_or));
	    combineCombo->addItem(tr("WITHIN %1 LINES OF    "
				     ).arg(state->nearness(), 0, 10),
				  QVariant::QVariant(ste_near_lines));
	    combineCombo->addItem(tr("AND"), QVariant::QVariant(ste_and));
	    combineCombo->addItem(tr("NOT"), QVariant::QVariant(ste_not));
	    combineCombo->setStatusTip(tr("Choose how to combine the "
					  "upper and lower boxes"));
	    connect(combineCombo,
		    SIGNAL(activated(int)), this, SLOT(newCombiner(int)));
	    hbox->addWidget(combineCombo);
	    hbox->addStretch();
	    
	    label = new QLabel(tr("distance: "));
	    hbox->addWidget(label);
	    hbox->addWidget(combineDistance);
	    
	    vbox->addLayout(hbox);
	  }
	else
	  {
	    matchCombo = new QComboBox;
	    matchCombo->setEditable(false);
	    matchCombo->addItem
	      (tr("Ignore differences in spacing and capitalization"),
	       QVariant::QVariant(match_ignores_spaces_and_case));
	    matchCombo->addItem(tr("Match exactly"),
				QVariant::QVariant(match_exactly));
	    matchCombo->addItem(tr("Ignoring differences in spacing"),
				QVariant::QVariant(match_ignores_spaces));
	    matchCombo->setStatusTip(tr("Choose how exactly to match the "
					"text you have entered."));
	    connect(matchCombo,
		    SIGNAL(activated(int)), this, SLOT(newExactness(int)));
	    hbox = new QHBoxLayout;
	    hbox->addWidget(matchCombo);
	    hbox->addStretch(0);
	    vbox->addLayout(hbox);
	  }
      }

    /* Set up the directory search list widgets and data. */
    searchTable = new QTreeWidget;
    searchTable->setColumnCount(1);
    searchTable->setTextElideMode(Qt::ElideLeft);
    connect(searchTable, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
	    this, SLOT(searchDirItemChanged(QTreeWidgetItem *, int)));
    vbox->addWidget(searchTable);

    hbox = new QHBoxLayout;

    /* Button to click to add a search dir. */
    adder = new QPushButton;
    adder->setText(tr("&Add"));
    adder->setShortcut(tr("Ctrl+A"));
    adder->setStatusTip(tr("Add another directory to the search list"));
    connect(adder, SIGNAL(clicked()), this, SLOT(addSearchDirectory()));
    hbox->addWidget(adder);

    /* Button to click to remove search dirs. */
    remover = new QPushButton;
    remover->setStatusTip(tr("Remove selected directories from search list"));
    remover->setText(tr("&Remove"));
    remover->setShortcut(tr("Ctrl+R"));
    connect(remover, SIGNAL(clicked()), this,
	    SLOT(removeSearchDirectory()));
    hbox->addWidget(remover);

    duplicator = new QPushButton;
    duplicator->setStatusTip(tr("Duplicate selected directories on "
				"search list"));
    duplicator->setText(tr("&Duplicate"));
    duplicator->setShortcut(tr("Ctrl+D"));
    connect(duplicator, SIGNAL(clicked()), this,
	    SLOT(duplicateSearchDirectory()));
    hbox->addWidget(duplicator);
    hbox->addStretch(0);

    /* Let them save the search. */
    saver = new QPushButton;
    saver->setText(tr("&Save"));
    saver->setShortcut(tr("Ctrl+S"));
    saver->setStatusTip(tr("Save this search."));
    connect(saver, SIGNAL(clicked()), this, SLOT(saveSearch()));
    hbox->addWidget(saver);

    /* Let them launch the search... */
    searchButton = new QPushButton;
    searchButton->setStatusTip(tr("Begin the search"));
    searchButton->setText(tr("&Find"));
    searchButton->setShortcut(tr("Ctrl+F"));
    connect(searchButton, SIGNAL(clicked()), this, SLOT(startSearching()));
    searchButton->setEnabled(false);
    hbox->addWidget(searchButton);

    vbox->addLayout(hbox);

    /* Make up the Saved Searches tab... */
    vbox = new QVBoxLayout;
    frame = new QFrame;
    frame->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    frame->setLayout(vbox);
    tabWidget->addTab(frame, tr("Saved Searches"));

    /* Set up the directory search list widgets and data. */
    savedSearchTable = new QTreeWidget;
    savedSearchTable->setColumnCount(1);
    savedSearchTable->setTextElideMode(Qt::ElideLeft);
    vbox->addWidget(savedSearchTable);

    /* Add the control buttons... */
    hbox = new QHBoxLayout;

    /* Button to click to use a saved search. */
    loader = new QPushButton;
    loader->setText(tr("&Use"));
    loader->setShortcut(tr("Ctrl+U"));
    loader->setStatusTip(tr("Use settings from selected search."));
    connect(loader, SIGNAL(clicked()), this, SLOT(loadSavedSearch()));
    hbox->addWidget(loader);

    /* Button to click to delete a saved search. */
    deleter = new QPushButton;
    deleter->setText(tr("&Remove"));
    deleter->setShortcut(tr("Ctrl+R"));
    deleter->setStatusTip(tr("Delete selected search."));
    connect(deleter, SIGNAL(clicked()), this, SLOT(deleteSavedSearch()));
    hbox->addWidget(deleter);

    hbox->addStretch(0);

    vbox->addLayout(hbox);

    /* Make up the Search Results tab... */
    vbox = new QVBoxLayout;
    frame = new QFrame;
    frame->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    frame->setLayout(vbox);
    tabWidget->addTab(frame, tr("Search Results"));

    //    textEdit = new QTextEdit;
    //    vbox->addWidget(textEdit);
    hbox = new QHBoxLayout;
    scrollbar = new QScrollBar();
    textViewer = new TextViewer(scrollbar);
    textViewer->setStatusTip(tr("This is the text viewer."));
    hbox->addWidget(textViewer);
    hbox->addWidget(scrollbar);
    vbox->addLayout(hbox);
    hbox = new QHBoxLayout;

    fileName = new QLineEdit();
    fileName->setAlignment(Qt::AlignRight);
    fileName->setReadOnly(true);
    fileName->setStyleSheet(tr("\
      QLineEdit {\
        border: 0px;\
        padding: 0 0px;\
        background-color: lightgray;\
      }"));

    hbox->addWidget(fileName);

    //    hbox->addStretch(0);

    stopSearchButton = new QPushButton;
    stopSearchButton->setStatusTip(tr("Stop searching"));
    stopSearchButton->setText(tr("S&top"));
    stopSearchButton->setShortcut(tr("Ctrl+T"));
    stopSearchButton->setEnabled(true);
    connect(stopSearchButton, SIGNAL(clicked()), this, SLOT(stopSearch()));
    hbox->addWidget(stopSearchButton);
    
    findPrevButton = new QPushButton;
    findPrevButton->setStatusTip(tr("Find the previous match"));
    findPrevButton->setText(tr("Find p&revious"));
    findPrevButton->setShortcut(tr("Ctrl+R"));
    findPrevButton->setEnabled(false);
    connect(findPrevButton, SIGNAL(clicked()), this, SLOT(findPrev()));
    hbox->addWidget(findPrevButton);

    findPrevFileButton = new QPushButton;
    findPrevFileButton->
	setStatusTip(tr("Find the last match in the previous matching file"));
    findPrevFileButton->setText(tr("Pr&evious file"));
    findPrevFileButton->setShortcut(tr("Ctrl+E"));
    findPrevFileButton->setEnabled(false);
    connect(findPrevFileButton, SIGNAL(clicked()), this, SLOT(findPrevFile()));
    hbox->addWidget(findPrevFileButton);

    findNextButton = new QPushButton;
    findNextButton->setStatusTip(tr("Find the next match"));
    findNextButton->setText(tr("Find a&gain"));
    findNextButton->setShortcut(tr("Ctrl+G"));
    findNextButton->setEnabled(false);
    connect(findNextButton, SIGNAL(clicked()), this, SLOT(findNext()));
    hbox->addWidget(findNextButton);
    
    findNextFileButton = new QPushButton;
    findNextFileButton->setStatusTip(tr("Find the first match in the next "
					"file that contains a match"));
    findNextFileButton->setText(tr("Find next fi&le"));
    findNextFileButton->setShortcut(tr("Ctrl+L"));
    findNextFileButton->setEnabled(false);
    connect(findNextFileButton, SIGNAL(clicked()), this,
	    SLOT(findNextFile()));
    hbox->addWidget(findNextFileButton);

    vbox->addLayout(hbox);

    setCentralWidget(tabWidget);

    createActions();
    createMenus();
    createStatusBar();

    if (goferList.size() == 0)
      readSettings();

    results = 0;

    currentAddDir = QDir::homePath();

    goferList.append(this);

    emDialog = 0;
    initializing = false;

    inputBoxes[0]->setFocus(Qt::TabFocusReason);
}

void Gofer::closeEvent(QCloseEvent *event)
{
  if (keepSearching)
    keepSearching = 0;
  event->accept();
}

void Gofer::addSearchDirectory()
{
  QString dir = (QFileDialog::getExistingDirectory
		 (this, tr("choose one or more directories to search"),
		  currentAddDir, QFileDialog::ShowDirsOnly));

  if (dir.size() != 0)
    {
      QDir newDir;
      QTreeWidgetItem *item = new QTreeWidgetItem(searchTable);
      item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable |
		     Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable |
		     Qt::ItemIsEnabled);
      item->setText(0, dir);
      item->setCheckState(0, Qt::Checked);
      state->addSearchDir(dir, true);

      newDir = QDir::QDir(dir);
      newDir.cdUp();
      currentAddDir = newDir.absolutePath();
      if (state->canSearch())
        searchButton->setEnabled(true);
    }
}

void Gofer::removeSearchDirectory()
{
  int i;
  QTreeWidgetItem *item;

  for (i = 0; i < searchTable->topLevelItemCount();)
    {
      if (searchTable->isItemSelected(searchTable->topLevelItem(i)))
	{
	  state->removeSearchDir(i);
	  item = searchTable->takeTopLevelItem(i);
	  delete item;
	  return;
	}
      else
	i++;
    }
}

void Gofer::duplicateSearchDirectory()
{
  int i;
  QTreeWidgetItem *item;

  for (i = 0; i < searchTable->topLevelItemCount();)
    {
      item = searchTable->topLevelItem(i);
      if (searchTable->isItemSelected(item))
	{
	  QTreeWidgetItem *newItem = new QTreeWidgetItem(searchTable);
	  newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable |
			    Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable |
			    Qt::ItemIsEnabled);
	  newItem->setText(0, item->text(0));
	  newItem->setCheckState(0, item->checkState(0));
	  state->addSearchDir(item->text(0),
			      (item->checkState(0) == Qt::Checked
			       ? true
			       : false));
	  return;
	}
      else
	i++;
    }
}

void Gofer::searchDirItemChanged(QTreeWidgetItem *item, int column)
{
  int i;

  if (initializing)
    return;

  for (i = 0; i < searchTable->topLevelItemCount(); i++)
    {
      if (searchTable->topLevelItem(i) == item)
	{
	  state->updateSearchDir(i, item->text(column),
				 (item->checkState(column) == Qt::Checked
				  ? true
				  : false));
	  break;
	}
    }
  if (state->canSearch())
    searchButton->setEnabled(true);
}

void Gofer::about()
{
   QMessageBox::about(this, tr("About gofer"),
            tr("gofer is a flexible, high-speed search tool for textual "
	       "research"));
}

void Gofer::newWindow()
{
  Gofer *newbie = new Gofer();
  QPoint newPos = pos();
  QSize newSize = size();
  newPos.setX(newPos.x() + 40);
  newPos.setY(newPos.y() + 40);
  newbie->setNewState(new SearchState(state));
  newbie->resize(newSize);
  newbie->move(newPos);
  newbie->show();
}

void Gofer::closeWindow()
{
  int i = goferList.indexOf(this);
  if (i != -1)
    {
      goferList.removeAt(i);
    }
  if (goferList.size() == 0)
    writeSettings();
  close();
}

void Gofer::closeAll()
{
  int i;
  for (i = 0; i < goferList.size(); i++)
    {
      if (goferList[i] != this)
	{
	  goferList[i]->close();
	}
    }
  writeSettings();
  close();
}

void Gofer::createActions()
{
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcut(tr("Ctrl+N"));
    newAct->setStatusTip(tr("New Gofer Window"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newWindow()));

    closeAct = new QAction(tr("Close &Window"), this);
    closeAct->setShortcut(tr("Ctrl+W"));
    closeAct->setStatusTip(tr("Close Window"));
    connect(closeAct, SIGNAL(triggered()), this, SLOT(closeWindow()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(closeAll()));

    copyAct = new QAction(tr("&Copy"), this);
    copyAct->setShortcut(tr("Ctrl+C"));
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

    pasteAct = new QAction(tr("&Paste"), this);
    pasteAct->setShortcut(tr("Ctrl+V"));
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

#if 0
    copyAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));
#endif
}

void Gofer::createMenus()
{
  goferMenu = menuBar()->addMenu(tr("Gofer"));
  goferMenu->addAction(newAct);
  goferMenu->addAction(closeAct);
  goferMenu->addAction(exitAct);

  editMenu = menuBar()->addMenu(tr("Edit"));
  editMenu->addAction(copyAct);
  editMenu->addAction(pasteAct);

  menuBar()->addSeparator();

  helpMenu = menuBar()->addMenu(tr("Help"));
  helpMenu->addAction(aboutAct);
  helpMenu->addAction(aboutQtAct);
}

void Gofer::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void Gofer::setStatusMessage(QString msg)
{
  statusBar()->showMessage(msg);
}

void Gofer::readSettings()
{
  QSettings settings;
  int i, max;
  QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
  QSize size = settings.value("size", QSize(400, 400)).toSize();
  resize(size);
  move(pos);

  max = settings.beginReadArray("directories");
  for (i = 0; i < max || (i == 0 && max == 0); i++)
    {
      Qt::CheckState checked;
      QString dirName;
      QTreeWidgetItem *item;
      settings.setArrayIndex(i);
      bool ok;

      checked = (Qt::CheckState)settings.value("checked").toInt(&ok);
      if (!ok)
	continue;
      dirName = settings.value("name").toString();
      item = new QTreeWidgetItem(searchTable);
      item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable |
		     Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable |
		     Qt::ItemIsEnabled);
      item->setText(0, dirName);
      item->setCheckState(0, checked);
      state->addSearchDir(dirName, checked);
    }
  settings.endArray();

  state->saved()->loadSettings(settings);
}

void Gofer::writeSettings()
{
  QSettings settings;
  int i;
  settings.setValue("pos", pos());
  settings.setValue("size", size());
  settings.beginWriteArray("directories");
  for (i = 0; i < searchTable->topLevelItemCount(); i++)
    {
      QTreeWidgetItem *item = searchTable->topLevelItem(i);
      settings.setArrayIndex(i);
      if (item)
	{
	  settings.setValue("checked", (int)item->checkState(0));
	  settings.setValue("name", item->text(0));
	}
    }
  settings.endArray();

  state->saved()->saveSettings(settings);
}

void Gofer::setApplication(QApplication &app)
{
  Gofer::application = &app;
  TextViewer::setApplication(application);
}

int Gofer::one_file(void *obj, const char *filename)
{
  Gofer *me = (Gofer *)obj;
  const char *s = filename;
  int len = strlen(filename);
  if (len > 120)
    {
      s += (len - 120);
      len = 120;
    }

  me->setStatusMessage(QString::fromUtf8(s, len));
  application->processEvents(QEventLoop::AllEvents);
  return me->keepGoing();
}

void Gofer::one_element(void *obj, const char *filename,
			     char *contents, int content_length,
			     int first_line, int last_line,
			     int first_char, int first_len,
			     int last_char, int last_len,
			     int *cur_line, int *cur_char)
{
  Gofer *me;
  SearchResults *results;

  me = (Gofer *)obj;
  results = me->searchResults();
  results->addResult(filename, contents, content_length,
		     first_line, last_line, first_char, first_len,
		     last_char, last_len, cur_line, cur_char);
}

void Gofer::newCombiner(int index)
{
  st_expr_type_t cs = (st_expr_type_t)combineCombo->itemData(index).toInt();
  setCombiner(cs);
  if (!initializing)
    state->setCombiner(cs);
}

void Gofer::setCombiner(st_expr_type_t cs)
{
  if (cs == ste_near_lines || cs == ste_near)
    combineDistance->setEnabled(true);
  else
    combineDistance->setEnabled(false);
}

void Gofer::newCombineDistance()
{
  int distance = combineDistance->text().toInt();
  setCombineDistance(distance);
  if (!initializing)
    state->setNearness(distance);
}

void Gofer::setCombineDistance(int distance)
{
  combineCombo->setItemText(0, tr("WITHIN %1 LINES OF    "
				  ).arg(distance, 0, 10));
  combineCombo->adjustSize();
  combineCombo->updateGeometry();
}

void Gofer::newExactness(int index)
{
  if (!initializing)
    state->setExactitude((st_match_type_t)matchCombo->itemData(index).toInt());
}

void Gofer::textEntered(int id)
{
  if (id < 0 || id > 7)
    return;

  if (!initializing)
    state->setText(id, inputBoxes[id]->text());
  if (state->canSearch())
    searchButton->setEnabled(true);
}

void Gofer::startSearching()
{
  emit setTabIndex(2);
  keepSearching = 1;
  doSearch();
}

void Gofer::stopSearch()
{
  keepSearching = 0;
}

int Gofer::keepGoing()
{
  return keepSearching;
}

void Gofer::findPrev()
{
  if (!results)
    {
      findPrevButton->setEnabled(false);
      findPrevFileButton->setEnabled(false);
      findNextButton->setEnabled(false);
      findNextFileButton->setEnabled(false);
      return;
    }
  results->prevMatch();
}

void Gofer::findPrevFile()
{
  if (!results)
    {
      findPrevButton->setEnabled(false);
      findPrevFileButton->setEnabled(false);
      findNextButton->setEnabled(false);
      findNextFileButton->setEnabled(false);
      return;
    }
  results->prevFileMatch();
}

void Gofer::findNext()
{
  if (!results)
    {
      findPrevButton->setEnabled(false);
      findPrevFileButton->setEnabled(false);
      findNextButton->setEnabled(false);
      findNextFileButton->setEnabled(false);
      return;
    }
  results->nextMatch();
}

void Gofer::findNextFile()
{
  if (!results)
    {
      findPrevButton->setEnabled(false);
      findPrevFileButton->setEnabled(false);
      findNextButton->setEnabled(false);
      findNextFileButton->setEnabled(false);
      return;
    }
  results->nextFileMatch();
}

void Gofer::newSearch()
{
  emit setTabIndex(0);
}

void Gofer::saveSearch()
{
  QString oldtext, text, *tp;
  bool ok = false;

  tp = state->name();
  if (tp)
    oldtext = QString::QString(*tp);
  else
    oldtext = "";
  text = QInputDialog::getText(this, tr("Name of saved search"),
			       tr("What should I call this search?"),
			       QLineEdit::Normal, oldtext, &ok);
  state->save(text);
  writeSettings();
}

void Gofer::loadSavedSearch()
{
  int i;

  for (i = 0; i < savedSearchTable->topLevelItemCount(); )
    {
      if (savedSearchTable->isItemSelected(savedSearchTable->topLevelItem(i)))
	{
	  SearchState *next;
	  next = state->saved()->get(i);
	  initializing = true;
	  if (next)
	    setNewState(next);
	  else
	    {
	      if (emDialog == 0)
		emDialog = new QErrorMessage();
	      emDialog->showMessage(tr("Selected search not found (this is "
				       "probably not your fault)."));
	    }
	  initializing = false;
	  return;
	}
      else
	i++;
    }
}

void Gofer::setNewState(SearchState *next)
{
  int i;

  delete state;
  state = next;
  for (i = 0; i < 8; i++)
    {
      QString *txt = state->text(i);
      if (txt != 0)
	inputBoxes[i]->setText(*txt);
      else
	inputBoxes[i]->clear();
    }
  combineDistance->setText(QString::number(state->nearness()));
  combineCombo->setItemText(0, tr("WITHIN %1 LINES OF    "
				  ).arg(state->nearness(), 0, 10));
  combineCombo->adjustSize();
  combineCombo->updateGeometry();
  for (i = 0; i < combineCombo->count(); i++)
    {
      if (combineCombo->itemData(i) == state->combiner())
	{
	  combineCombo->setCurrentIndex(i);
	  break;
	}
    }
  for (i = 0; i < matchCombo->count(); i++)
    {
      if (matchCombo->itemData(i) == state->exactitude())
	{
	  matchCombo->setCurrentIndex(i);
	  break;
	}
    }
  while (searchTable->topLevelItemCount() > 0)
    {
      QTreeWidgetItem *item;
      item = searchTable->takeTopLevelItem(i);
      delete item;
    }
  printf("getting search state for display.\n");
  for (i = 0; i < state->numSearchDirs(); i++)
    {
      QTreeWidgetItem *item = new QTreeWidgetItem(searchTable);
      item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable |
		     Qt::ItemIsDropEnabled |
		     Qt::ItemIsUserCheckable |
		     Qt::ItemIsEnabled);
      item->setText(0, state->searchDir(i));
      item->setCheckState(0, (state->dirIsChecked(i) ?
			      Qt::Checked :
			      Qt::Unchecked));
    }
  emit setTabIndex(0);
  if (state->canSearch())
    searchButton->setEnabled(true);
}


void Gofer::deleteSavedSearch()
{
  int i;

  for (i = 0; i < savedSearchTable->topLevelItemCount(); )
    {
      if (savedSearchTable->isItemSelected(savedSearchTable->topLevelItem(i)))
	{
	  state->saved()->remove(i);
	  return;
	}
      else
	i++;
    }
}

void Gofer::savedSearchAdd(QString *stateName)
{
  QTreeWidgetItem *item = new QTreeWidgetItem(savedSearchTable);
  item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  item->setText(0, *stateName);
}

void Gofer::savedSearchRemove(int index)
{
  QTreeWidgetItem *item;
  item = savedSearchTable->takeTopLevelItem(index);
  delete item;
}

void Gofer::doSearch()
{
  int i;
  int n;
  search_term_t *terms;
  st_match_type_t searchExactitude;
  st_expr_t *searchExpr;
  QStringList searchList;

  /* We need something to search before we can do a search. */
  if (!state->numSearchDirs()) {
    this->setStatusMessage(tr("Please click on Add to add a directory in "
			      "which to search."));
    return;
  }

  searchExpr = state->genEquation();
  if (!searchExpr)
    {
      this->setStatusMessage(tr("Please enter some text to search for."));
      return;
    }

  searchExactitude = state->exactitude();

  n = extract_search_terms(&terms, searchExpr);

  // Is this even remotely thread-safe?
  if (results)
    delete results;
  results = new SearchResults(this);
  results->setViewer(textViewer);
  results->setNameView(fileName);
  results->setMatchButtons(findNextButton,
			   findNextFileButton,
			   findPrevButton,
			   findPrevFileButton);
  results->zapMatchView();
  
  searchList = QStringList::QStringList();
  for (i = 0; i < searchTable->topLevelItemCount(); i++)
    {
      QTreeWidgetItem *item = searchTable->topLevelItem(i);
      if (item && item->checkState(0) == Qt::Checked)
	searchList.append(item->text(0));
    }

  for (i = 0; i < searchList.size(); i++)
    {
      if (!search_tree(searchList[i].toUtf8().constData(),
		       searchExpr, terms, n,
		       one_element, one_file, (void *)this,
		       0, searchExactitude))
	break;
    }
  printf("done with search list.\n");
  if (!keepSearching)
    this->setStatusMessage(tr("Search Interrupted"));
  else
    this->setStatusMessage(tr("Search complete"));
  //  results->setViewFile(tr(""));
  if (!results->haveContents)
    {
      emit setTabIndex(0);
      this->setStatusMessage(tr("Search found no matches.\n"));
    }

  keepSearching = 1;

  free_expr(searchExpr);
  searchExpr = 0;
}

SearchResults *Gofer::searchResults()
{
  return results;
}

void Gofer::keyReleaseEvent(QKeyEvent *event)
{
  switch(event->key())
    {
    case Qt::Key_Up:
      printf("up\n");
      if (textViewer)
	{
	  event->accept();
	  textViewer->up();
	}
      break;
    case Qt::Key_Down:
      printf("down\n");
      if (textViewer)
	{
	  event->accept();
	  textViewer->down();
	}
      break;
    case Qt::Key_PageUp:
      printf("page up\n");
      if (textViewer)
	{
	  event->accept();
	  textViewer->pageUp();
	}
      break;
    case Qt::Key_PageDown:
      printf("page down\n");
      if (textViewer)
	{
	  event->accept();
	  textViewer->pageDown();
	}
      break;
    case Qt::Key_C:
      printf("C key pressed with %lx.\n", (unsigned long)event->modifiers());
      if (event->modifiers() == Qt::ControlModifier &&
	  textViewer->maybeCopy(false))
	{
	  printf("copy.\n");
	  event->accept();
	}
      else
	event->ignore();
      break;
    case Qt::Key_N:
      printf("N key pressed.\n");
      if (event->modifiers() == Qt::AltModifier)
	{
	  textViewer->maybeCopy(true);
	  event->accept();
	}
      else
	event->ignore();
      break;
    case Qt::Key_End:
      printf("End key pressed.\n");
      if (event->modifiers() = Qt::ControlModifier)
	{
	  event->accept();
	  textViewer->goto_bottom();
	}
      else
	event->ignore();
      break;
    case Qt::Key_Home:
      printf("Home key pressed.\n");
      if (event->modifiers() = Qt::ControlModifier)
	{
	  event->accept();
	  textViewer->goto_top();
	}
      else
	event->ignore();
      break;
    default:
      event->ignore();
      break;
    }
}

void
Gofer::copy(void)
{
  if (textViewer->maybeCopy(false))
    {
      printf("copied.\n");
    }
}

void
Gofer::paste(void)
{
  int i;

  for (i = 0; i < 8; i++)
    if (inputBoxes[i]->hasFocus())
      inputBoxes[i]->paste();
}

/* Local Variables:  */
/* mode:c++ */
/* c-file-style:"gnu" */
/* end: */
