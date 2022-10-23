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

#include <QRegularExpression>
#include <QTimer>

/**
 * In certain conditions many small messages may arrive very rapidly in
 * succession. Adding them to the display immediately as they come causes a
 * performance hit. To mitigate that, buffer the messages that arrive and
 * anticipate more messages to arrive in a short period of time. Only flush them
 * and append them to the display box after the grace period expires.
 *
 * Fixes: #4026
 */
static const int BUFFER_FLUSH_GRACE_MSECS = 100;

DClass<ServerConsole> : public Ui::ServerConsole
{
public:
	MemoryLineEdit *consoleInput;
	QString buffer;
	QTimer bufferFlushTimer;
};

DPointeredNoCopy(ServerConsole)

ServerConsole::ServerConsole(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
	d->setupUi(this);

	d->consoleInput = new MemoryLineEdit();
	layout()->addWidget(d->consoleInput);

	connect(d->consoleInput, &QLineEdit::returnPressed,
		this, &ServerConsole::forwardMessage);
	connect(&d->bufferFlushTimer, &QTimer::timeout,
		this, &ServerConsole::flushBuffer);
	d->bufferFlushTimer.setSingleShot(true);
}

ServerConsole::~ServerConsole() = default;

void ServerConsole::appendMessage(const QString &message)
{
	QString appendMessage = QString(message).toHtmlEscaped();
	if (!appendMessage.endsWith('\n'))
		appendMessage += '\n';
	appendMessage.replace('\n', "<br>");

	// Process colors
	if (gConfig.doomseeker.bColorizeServerConsole)
		appendMessage = Strings::colorizeString(appendMessage);
	else
	{
		static const QRegularExpression colorCode("\034(\\[[a-zA-Z0-9]*\\]|[a-v+\\-!*])");
		appendMessage.remove(colorCode);
	}

	d->buffer += appendMessage;
	if (!d->bufferFlushTimer.isActive())
	{
		d->bufferFlushTimer.start(BUFFER_FLUSH_GRACE_MSECS);
	}
}

void ServerConsole::flushBuffer()
{
	// Emulate append since we need to force HTML on (append auto detects which fails if &lt; or < is not found).
	d->consoleOutput->moveCursor(QTextCursor::End);
	d->consoleOutput->insertHtml(d->buffer);
	d->consoleOutput->moveCursor(QTextCursor::End);
	d->buffer.clear();
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
