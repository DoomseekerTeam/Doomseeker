//------------------------------------------------------------------------------
// cfgcustomservers.cpp
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
#ifndef __CFGCUSTOMSERVERS_H_
#define __CFGCUSTOMSERVERS_H_

#include "dptr.h"
#include "gui/configuration/configpage.h"
#include <QIcon>

class CustomServerInfo;
class EnginePlugin;
class QModelIndex;
class QStandardItem;
class QStandardItemModel;

class CFGCustomServers : public ConfigPage
{
	Q_OBJECT

public:
	CFGCustomServers(QWidget *parent = nullptr);
	~CFGCustomServers() override;

	QIcon icon() const override
	{
		return QIcon(":/flags/localhost-small");
	}
	QString name() const override
	{
		return tr("Pinned Servers");
	}

protected:
	void readSettings() override;
	void saveSettings() override;

private:
	enum CheckAndFixPorts
	{
		AllOk,
		AtLeastOneFixed
	};

	enum ColumnIndices
	{
		EngineColumnIndex = 0,
		AddressColumnIndex = 1,
		PortColumnIndex = 2,
		EnabledIndex = 3
	};

	DPtr<CFGCustomServers> d;

	/**
	 *	@brief Moves through rows and checks if network port information
	 *	is correct.
	 *
	 *	@param firstRow - First row to be checked (inclusive).
	 *	@param lastRow - Last row to be checked (inclusive).
	 */
	CheckAndFixPorts checkAndFixPorts(int firstRow, int lastRow);
	const EnginePlugin *getPluginInfoForRow(int rowIndex);
	bool isPortColumnWithinRange(int leftmostColumnIndex, int rightmostColumnIndex);
	bool isPortCorrect(int rowIndex);

	void prepareEnginesComboBox();
	void prepareTable();
	void setEnabledOnSelected(bool enabled);
	void setEngineOnItem(QStandardItem *, const QString &engineName);
	void setPortToDefault(int rowIndex);

	QVector<CustomServerInfo> tableGetServers();

private slots:
	void add();
	void add(const QString &engineName, const QString &host,
		unsigned short port, bool enabled);
	void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
	void disableSelected();
	void enableSelected();
	void remove();
	void setEngine();
};

#endif
