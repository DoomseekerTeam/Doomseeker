//------------------------------------------------------------------------------
// cfgircdefinenetworkdialog.h
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
#ifndef __CFGIRCDEFINENETWORKDIALOG_H__
#define __CFGIRCDEFINENETWORKDIALOG_H__

#include "dptr.h"

#include <QDialog>
#include <QtContainerFwd>

class IRCNetworkEntity;
class QAbstractButton;

class CFGIRCDefineNetworkDialog : public QDialog
{
	Q_OBJECT

public:
	CFGIRCDefineNetworkDialog(const IRCNetworkEntity &initValuesEntity, QWidget *parent = nullptr);
	CFGIRCDefineNetworkDialog(QWidget *parent = nullptr);
	~CFGIRCDefineNetworkDialog() override;

	IRCNetworkEntity getNetworkEntity() const;
	/**
	 * @brief Overrides extraction from config if not-empty list is set.
	 */
	void setExistingNetworks(const QList<IRCNetworkEntity> &networks);

public slots:
	void accept() override;

private:
	DPtr<CFGIRCDefineNetworkDialog> d;

	bool askToAcceptAnywayWhenCommandsBad(const QStringList &offenders);
	QStringList autojoinCommands() const;
	void construct();
	QStringList formatOffenders(const QStringList &offenders) const;
	void initFrom(const IRCNetworkEntity &networkEntity);
	bool isDescriptionUnique() const;
	bool isValidDescription() const;
	QList<IRCNetworkEntity> listExistingNetworks() const;
	/**
	 * @brief If validation fails, the offending lines are returned.
	 */
	QStringList validateAutojoinCommands() const;
	bool validateDescription();

private slots:
	void buttonClicked(QAbstractButton *button);
};

#endif
