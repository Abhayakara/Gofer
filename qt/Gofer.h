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

#ifndef GOFER_H
#define GOFER_H

#include <QMainWindow>
#include "st.h"

class QAction;
class QMenu;
class QTextEdit;
class QLineEdit;
class QLabel;
class QTabWidget;
class QComboBox;
class QPushButton;
class QVBoxLayout;
class QTreeWidget;
class QTreeWidgetItem;
class ItemSlotReceiver;
class SearchResults;
class QApplication;
class QFileDialog;
class SearchState;
class QErrorMessage;
class TextViewer;

class Gofer : public QMainWindow
{
  Q_OBJECT

public:
  Gofer();
  static void setApplication(QApplication &app);
  static QApplication *application;
  void setStatusMessage(QString msg);

protected:
  void closeEvent(QCloseEvent *event);

private slots:
  void addSearchDirectory();
  void removeSearchDirectory();
  void duplicateSearchDirectory();
  void searchDirItemChanged(QTreeWidgetItem *item, int column);
  void about();
  void newWindow();
  void closeWindow();
  void closeAll();
  void newCombiner(int index);
  void setCombiner(st_expr_type_t cs);
  void newCombineDistance();
  void setCombineDistance(int distance);
  void newExactness(int index);
  void textEntered(int id);
  void startSearching();
  int keepGoing();
  void stopSearch();
  void findPrev();
  void findPrevFile();
  void findNext();
  void findNextFile();
  void newSearch();
  void saveSearch();
  void loadSavedSearch();
  void setNewState(SearchState *next);
  void deleteSavedSearch();
  void savedSearchAdd(QString *stateName);
  void savedSearchRemove(int index);
  void keyReleaseEvent(QKeyEvent *event);
  void copy(void);
  void paste(void);
signals:
  void setTabIndex(int index);

private:
  static QList<Gofer *> goferList;

  void createActions();
  void createMenus();
  void createToolBars();
  void createStatusBar();
  void readSettings();
  void writeSettings();

  static int one_file(void *obj, const char *filename);
  static void one_element(void *obj, const char *filename,
			  char *contents, int content_length,
			  int first_line, int last_line,
			  int first_char, int first_len,
			  int last_char, int last_len,
			  int *cur_line, int *cur_char);

  void doSearch();
  SearchResults *searchResults();

  QTabWidget *tabWidget;
  //  QTextEdit *textEdit;
  TextViewer *textViewer;

  QLabel *topLabel;
  QLineEdit *inputBoxes[8];
  ItemSlotReceiver *inputSlots[8];
  QComboBox *combineCombo;
  QLineEdit *combineDistance;
  QComboBox *matchCombo;
  QTreeWidget *searchTable;
  QPushButton *adder;
  QPushButton *remover;
  QPushButton *duplicator;
  QTreeWidget *savedSearchTable;
  QPushButton *saver;
  QPushButton *searchButton;
  QPushButton *loader;
  QPushButton *deleter;
  QLineEdit *fileName;
  QPushButton *stopSearchButton;
  QPushButton *findPrevButton;
  QPushButton *findPrevFileButton;
  QPushButton *findNextButton;
  QPushButton *findNextFileButton;
    
  QMenu *goferMenu;
  QMenu *editMenu;
  QMenu *helpMenu;
  QAction *newAct;
  QAction *exitAct;
  QAction *closeAct;
  QAction *copyAct;
  QAction *pasteAct;
  QAction *aboutAct;
  QAction *aboutQtAct;
  
  /* General status info. */
  SearchResults *results;
  SearchState *state;
  QFileDialog *dirAddDialog;
  QErrorMessage *emDialog;
  bool initializing;

  /* We set this in the middle of the search if we're supposed to stop. */
  int keepSearching;

  QString currentAddDir;
};

#endif

/* Local Variables:  */
/* mode:c++ */
/* c-file-style:"gnu" */
/* end: */
