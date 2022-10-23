//------------------------------------------------------------------------------
// cfgircnetworks.h
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
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __CFGIRCNETWORKS_H__
#define __CFGIRCNETWORKS_H__

#include "dptr.h"
#include "gui/configuration/configpage.h"
#include <QIcon>
#include <QtContainerFwd>

class IRCNetworkEntity;
class QModelIndex;
class QStandardItem;

class CFGIRCNetworks : public ConfigPage
{
	Q_OBJECT

public:
	CFGIRCNetworks(QWidget *parent = nullptr);
	~CFGIRCNetworks() override;

	QIcon icon() const override
	{
		return QIcon(":/flags/lan-small");
	}
	QString name() const override
	{
		return tr("Networks");
	}
	QVector<IRCNetworkEntity *> networks();
	void readSettings() override;
	QString title() const override
	{
		return tr("IRC - Networks");
	}

protected:
	void saveSettings() override;

private:
	void addRecord(IRCNetworkEntity *pNetworkEntity);
	void cleanUpTable();
	QList<QStandardItem *> generateTableRecord(IRCNetworkEntity *pNetworkEntity);
	IRCNetworkEntity *network(int row) const;
	QList<IRCNetworkEntity> networksAsQList() const;

	/**
	 *	@brief Never call this function directly. Use network() instead.
	 */
	IRCNetworkEntity *obtainNetworkEntity(QStandardItem *pItem) const;

	void prepareTable();
	void saveNetworks();
	IRCNetworkEntity *selectedNetwork();
	int selectedRow();
	void updateRecord(int row);

	DPtr<CFGIRCNetworks> d;
private slots:
	void addButtonClicked();
	void editButtonClicked();
	void removeButtonClicked();
	void tableDoubleClicked();
};

#endif
