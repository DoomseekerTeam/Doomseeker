//------------------------------------------------------------------------------
// cfgchatlogspage.h
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef id65026263_73af_4292_be84_ddb5bf7f606b
#define id65026263_73af_4292_be84_ddb5bf7f606b

#include "dptr.h"
#include "gui/configuration/configpage.h"
#include <QIcon>

class QFileInfo;

class CfgChatLogsPage : public ConfigPage
{
	Q_OBJECT

public:
	CfgChatLogsPage(QWidget *parent);
	~CfgChatLogsPage() override;

	QIcon icon() const override
	{
		return QIcon(":/icons/log.png");
	}
	QString name() const override
	{
		return tr("Logging");
	}
	void readSettings() override;
	QString title() const override
	{
		return tr("IRC - Logging");
	}
	Validation validate() override;

protected:
	void saveSettings() override;

private:
	DPtr<CfgChatLogsPage> d;

	bool checkDir(const QString &directory);
	QString validateChatLogsPath(const QFileInfo &path) const;

private slots:
	void browseStorageDirectory();
	void exploreStorageDirectory();
};

#endif
