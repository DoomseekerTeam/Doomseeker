//------------------------------------------------------------------------------
// serverconsole.h
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
#ifndef SERVERCONSOLE_H
#define SERVERCONSOLE_H

#include "dptr.h"

#include <QWidget>

class ServerConsole : public QWidget
{
	Q_OBJECT

public:
	ServerConsole(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);
	~ServerConsole() override;

	/**
	 * @brief Sets keyboard focus to the underlying line edit widget.
	 */
	void setFocus();

public slots:
	void appendMessage(const QString &message);

signals:
	void messageSent(const QString &message);

protected slots:
	void forwardMessage();

private:
	DPtr<ServerConsole> d;
};

#endif // SERVERCONSOLE_H
