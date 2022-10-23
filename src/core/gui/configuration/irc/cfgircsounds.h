//------------------------------------------------------------------------------
// cfgircsounds.h
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
// Copyright (C) 2011 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __CFGIRCSOUNDS_H__
#define __CFGIRCSOUNDS_H__

#include "gui/configuration/configpage.h"
#include <QIcon>

class QFileInfo;
class QLineEdit;

class CFGIRCSounds : public ConfigPage
{
	Q_OBJECT

public:
	CFGIRCSounds(QWidget *parent = nullptr);
	~CFGIRCSounds() override;

	QIcon icon() const override
	{
		return QIcon(":/icons/audio-speaker.png");
	}
	QString name() const override
	{
		return tr("Sounds");
	}
	void readSettings() override;
	QString title() const override
	{
		return tr("IRC - Sounds");
	}
	Validation validate() override;

protected:
	void saveSettings() override;

private:
	QString getPathToWav();
	void setPath(QLineEdit *pLineEdit, const QString &path);

	DPtr<CFGIRCSounds> d;

	void playSound(const QString &path) const;
	QString validateFilePath(const QString &path) const;

private slots:
	void browseNicknameUsed();
	void browsePrivateMessage();
	void playNicknameUsed();
	void playPrivateMessage();
};


#endif
