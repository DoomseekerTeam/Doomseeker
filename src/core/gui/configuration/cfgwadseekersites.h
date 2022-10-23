//------------------------------------------------------------------------------
// cfgwadseekersites.h
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
#ifndef __WADSEEKERCONFIG_SITES_H_
#define __WADSEEKERCONFIG_SITES_H_

#include "dptr.h"
#include "gui/configuration/configpage.h"
#include <QIcon>

class QUrl;

class CFGWadseekerSites : public ConfigPage
{
	Q_OBJECT

public:
	CFGWadseekerSites(QWidget *parent = nullptr);
	~CFGWadseekerSites() override;

	QIcon icon() const override
	{
		return QIcon(":/icons/internet-web-browser.png");
	}
	QString name() const override
	{
		return tr("Sites");
	}
	void readSettings() override;
	QString title() const override
	{
		return tr("Wadseeker - Sites");
	}

protected slots:
	void btnUrlAddClicked();
	void btnUrlDefaultClicked();
	void btnUrlRemoveClicked();

protected:
	void saveSettings() override;

private:
	DPtr<CFGWadseekerSites> d;

	void insertUrl(const QString &url);
};

#endif
