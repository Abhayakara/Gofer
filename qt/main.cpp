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

#include <QApplication>

void qt_set_sequence_auto_mnemonic(bool enable);

#include "gofer.h"

int main(int argc, char *argv[])
{
    Gofer *gofer;
    // Q_INIT_RESOURCE(Gofer);
    QFont font = QApplication::font();
    font.setPointSize(font.pointSize() + 3);
    QApplication::setFont(font);

    qt_set_sequence_auto_mnemonic(true);

    QApplication app(argc, argv);
    app.setApplicationName(QString::QString("gofer"));
    app.setOrganizationDomain(QString::QString("dmes.org"));
    app.setOrganizationName(QString::QString("Diamond Mountain"));
    gofer = new Gofer;
    gofer->show();
    Gofer::setApplication(app);
    return app.exec();
}
