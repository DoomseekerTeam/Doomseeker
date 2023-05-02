//------------------------------------------------------------------------------
// dockBuddiesList.h
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

#ifndef __DOCKBUDDIESLIST_H__
#define __DOCKBUDDIESLIST_H__

#include "dptr.h"
#include "serverapi/serverptr.h"

#include <QDialog>
#include <QDockWidget>

class MasterManager;
class QAbstractButton;
class QModelIndex;

class DockBuddiesList : public QDockWidget
{
	Q_OBJECT

public:
	enum ColumnId
	{
		BLCID_ID,
		BLCID_BUDDY,
		BLCID_LOCATION,

		HOW_MANY_BUDDIESLIST_COLUMNS
	};

	DockBuddiesList(QWidget *parent = nullptr);
	~DockBuddiesList() override;

	bool hasBuddy(const ServerPtr &server);

public slots:
	void addBuddy();
	void scan(const MasterManager *master = nullptr);

signals:
	void joinServer(const ServerPtr &server);
	void scanCompleted();

protected slots:
	void deleteBuddy();
	void followBuddy(const QModelIndex &index);
	void patternsListContextMenu(const QPoint &pos) const;

private:
	DPtr<DockBuddiesList> d;

	const MasterManager *masterClient;
	bool save;
};

class AddBuddyDlg : public QDialog
{
	Q_OBJECT

public:
	AddBuddyDlg(QWidget *parent = nullptr);
	~AddBuddyDlg() override;

	QRegExp pattern() const;

protected slots:
	void buttonBoxClicked(QAbstractButton *button);

private:
	DPtr<AddBuddyDlg> d;
};

#endif /* __DOCKBUDDIESLIST_H__ */
