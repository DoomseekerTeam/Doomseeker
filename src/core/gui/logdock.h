//------------------------------------------------------------------------------
// logdock.h
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
#ifndef __LOGDOCK_H_
#define __LOGDOCK_H_

#include "dptr.h"

#include <QDockWidget>

/**
 * @brief Dockable widget designed for application's log presentation.
 */
class LogDock : public QDockWidget
{
	Q_OBJECT

public:
	LogDock(QWidget *parent = nullptr);
	~LogDock() override;

public slots:
	/**
	 * New entry will be appended to the end of the current textedit
	 * content as it is, without any special formatting.
	 */
	void appendLogEntry(const QString &entry);
	void clearContent();

protected slots:
	void btnCopyClicked();

private:
	DPtr<LogDock> d;
};

#endif
