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

#ifndef ITEMSLOTRECEIVER_H
#define ITEMSLOTRECEIVER_H

#include <QObject>

class ItemSlotReceiver : public QObject
{
  Q_OBJECT

public:
  void setId(int newId);

public slots:
  void textChanged();

signals:
  void textChangedId(int id);

private:
  int id;
};
#endif

/* Local Variables:  */
/* mode:c++ */
/* c-file-style:"gnu" */
/* end: */
