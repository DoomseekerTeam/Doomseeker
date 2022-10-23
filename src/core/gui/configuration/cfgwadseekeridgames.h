//------------------------------------------------------------------------------
// cfgwadseekeridgames.h
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
#ifndef __WADSEEKERCONFIG_IDGAMES_H_
#define __WADSEEKERCONFIG_IDGAMES_H_

#include "dptr.h"
#include "gui/configuration/configpage.h"
#include <QIcon>

class CFGWadseekerIdgames : public ConfigPage
{
	Q_OBJECT

public:
	CFGWadseekerIdgames(QWidget *parent = nullptr);
	~CFGWadseekerIdgames() override;

	QIcon icon() const override
	{
		return QIcon(":/icons/utilities-file-archiver-2.png");
	}
	QString name() const override
	{
		return tr("Archives");
	}
	void readSettings() override;
	QString title() const override
	{
		return tr("Wadseeker - Archives");
	}

protected slots:
	void btnIdgamesURLDefaultClicked();

protected:
	void saveSettings() override;

private:
	DPtr<CFGWadseekerIdgames> d;
};

#endif
