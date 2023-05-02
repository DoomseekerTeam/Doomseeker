//------------------------------------------------------------------------------
// cfgircappearance.h
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
#ifndef __CFGIRCAPPEARANCE_H__
#define __CFGIRCAPPEARANCE_H__

#include "dptr.h"
#include "gui/configuration/configpage.h"
#include <QIcon>

class CFGIRCAppearance : public ConfigPage
{
	Q_OBJECT

public:
	CFGIRCAppearance(QWidget *parent = nullptr);
	~CFGIRCAppearance() override;

	QIcon icon() const override
	{
		return QIcon(":/icons/color-fill.png");
	}
	QString name() const override
	{
		return tr("Appearance");
	}
	void readSettings() override;
	QString title() const override
	{
		return tr("IRC - Appearance");
	}

protected:
	void saveSettings() override;

private:
	DPtr<CFGIRCAppearance> d;
};

#endif
