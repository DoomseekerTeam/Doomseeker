//------------------------------------------------------------------------------
// cfgwadseekergeneral.h
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
#ifndef __WADSEEKERCONFIG_GENERAL_H_
#define __WADSEEKERCONFIG_GENERAL_H_

#include "dptr.h"
#include "gui/configuration/configpage.h"
#include <QIcon>

class CFGWadseekerGeneral : public ConfigPage
{
	Q_OBJECT

public:
	CFGWadseekerGeneral(QWidget *parent = nullptr);
	~CFGWadseekerGeneral() override;

	QIcon icon() const override
	{
		return QIcon(":/icons/preferences-system-4.png");
	}
	QString name() const override
	{
		return tr("General");
	}
	QString title() const override
	{
		return tr("Wadseeker - General");
	}
	Validation validate() override;

protected:
	void fillTargetDirectoryComboBox();
	void readSettings() override;
	void saveSettings() override;
	void showEvent(QShowEvent *) override;

private:
	DPtr<CFGWadseekerGeneral> d;
	void activateCompleter();
};

#endif
