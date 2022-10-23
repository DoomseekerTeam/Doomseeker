//------------------------------------------------------------------------------
// taskbarbutton.h
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
// Copyright (C) 2015 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef id66f36b50_e785_4dbc_85d2_638742154353
#define id66f36b50_e785_4dbc_85d2_638742154353

#include "dptr.h"
#include <QObject>

class QString;
class QIcon;
class QWindow;
class TaskbarProgress;

/**
 * @brief Platform-agnostic wrapper for QWinTaskbarButton.
 *
 * Actually, the platform wrapping is only meant to avoid having to
 * run #ifdef checks in the code that wishes to use the
 * TaskbarButton. On Windows platform this is a proxy design
 * pattern. On all other platforms objects of this class are no-ops.
 */
class TaskbarButton : public QObject
{
	Q_OBJECT

public:
	TaskbarButton(QObject *parent);

	QString overlayAccessibleDescription() const;
	QIcon overlayIcon() const;
	TaskbarProgress *progress() const;
	void setWindow(QWindow *window);
	QWindow *window() const;

public slots:
	void clearOverlayIcon();
	void setOverlayAccessibleDescription(const QString &description);
	void setOverlayIcon(const QIcon &icon);

private:
	DPtr<TaskbarButton> d;
};

#endif
