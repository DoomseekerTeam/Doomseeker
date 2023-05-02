//------------------------------------------------------------------------------
// serverconsole.cpp
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
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------
#include "configuration/doomseekerconfig.h"
#include "gui/widgets/memorylineedit.h"
#include "serverconsole.h"
#include "strings.hpp"
#include "ui_serverconsole.h"

#include <QRegExp>

DClass<ServerConsole> : public Ui::ServerConsole
{
public:
	MemoryLineEdit *consoleInput;
};

DPointered(ServerConsole)

ServerConsole::ServerConsole(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
	d->setupUi(this);

	d->consoleInput = new MemoryLineEdit();
	layout()->addWidget(d->consoleInput);

	connect(d->consoleInput, SIGNAL(returnPressed()), this, SLOT(forwardMessage()));
}

ServerConsole::~ServerConsole()
{
}

void ServerConsole::appendMessage(const QString &message)
{
	QString appendMessage = QString(message).toHtmlEscaped();
	if (appendMessage.endsWith('\n')) // Remove the trailing new line since appendPlainText seems to add one automatically.
		appendMessage.chop(1);
	appendMessage.replace('\n', "<br>");

	// Process colors
	if (gConfig.doomseeker.bColorizeServerConsole)
		appendMessage = Strings::colorizeString(appendMessage);
	else
	{
		static const QRegExp colorCode("\034(\\[[a-zA-Z0-9]*\\]|[a-v+\\-!*])");
		appendMessage.remove(colorCode);
	}

	// Emulate append since we need to force HTML on (append auto detects which fails if &lt; or < is not found).
	d->consoleOutput->moveCursor(QTextCursor::End);
	d->consoleOutput->insertHtml("<br>" + appendMessage);
	d->consoleOutput->moveCursor(QTextCursor::End);
}

void ServerConsole::forwardMessage()
{
	QString msg = d->consoleInput->text();
	if (msg[0] == ':')
		msg.replace(0, 1, "say ");

	emit messageSent(msg);
	d->consoleInput->setText("");
}

void ServerConsole::setFocus()
{
	d->consoleInput->setFocus();
}
