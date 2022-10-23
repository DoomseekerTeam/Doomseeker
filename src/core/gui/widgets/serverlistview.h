//------------------------------------------------------------------------------
// serverlistview.h
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

#ifndef __SERVERLISTVIEW_H_
#define __SERVERLISTVIEW_H_

#include <QTableView>

class QMouseEvent;
class ServerListProxyModel;

#define USERROLE_RIGHTALIGNDECORATION 1

class ServerListView : public QTableView
{
	Q_OBJECT

public:
	ServerListView(QWidget *parent = nullptr);

	void setupTableProperties();

protected:
	void mouseReleaseEvent(QMouseEvent *event) override;
	void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
	void setupTableColumnWidths();

signals:
	void leftMouseDoubleClicked(const QModelIndex &index, const QPoint &cursorPosition);
	void middleMouseClicked(const QModelIndex &index, const QPoint &cursorPosition);
	void rightMouseClicked(const QModelIndex &index, const QPoint &cursorPosition);
};

#endif
