//------------------------------------------------------------------------------
// logdock.cpp
//------------------------------------------------------------------------------
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA
//
//------------------------------------------------------------------------------
// Copyright (C) 2009 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------

#include "clipboard.h"
#include "log.h"
#include "logdock.h"
#include "ui_logdock.h"

#include <QAction>

DClass<LogDock> : public Ui::LogDock
{
};

DPointered(LogDock)

LogDock::LogDock(QWidget *parent) : QDockWidget(parent)
{
	d->setupUi(this);
	this->toggleViewAction()->setIcon(QIcon(":/icons/log.png"));

	connect(d->btnClear, SIGNAL(clicked()), this, SLOT(clearContent()));
	connect(d->btnCopy, SIGNAL(clicked()), this, SLOT(btnCopyClicked()));
}

LogDock::~LogDock()
{
}

void LogDock::appendLogEntry(const QString &entry)
{
	d->teContent->moveCursor(QTextCursor::End);
	d->teContent->insertPlainText(entry);
}

void LogDock::clearContent()
{
	gLog.clearContent();
	d->teContent->document()->clear();
}

void LogDock::btnCopyClicked()
{
	Clipboard::setText(d->teContent->document()->toPlainText());
}
