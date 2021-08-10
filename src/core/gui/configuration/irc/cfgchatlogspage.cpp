//------------------------------------------------------------------------------
// cfgchatlogspage.cpp
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
#include "cfgchatlogspage.h"
#include "ui_cfgchatlogspage.h"

#include "irc/configuration/chatlogscfg.h"
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QUrl>

DClass<CfgChatLogsPage> : public Ui::CfgChatLogsPage
{
};

DPointered(CfgChatLogsPage)


CfgChatLogsPage::CfgChatLogsPage(QWidget *parent)
	: ConfigPage(parent)
{
	d->setupUi(this);
	d->lblDirWarning->hide();

	this->connect(d->leDir, SIGNAL(editingFinished()),
		SIGNAL(validationRequested()));
}

CfgChatLogsPage::~CfgChatLogsPage()
{
}

void CfgChatLogsPage::browseStorageDirectory()
{
	QString path = QFileDialog::getExistingDirectory(this,
		tr("Browse chat logs storage directory"), d->leDir->text());
	if (!path.isEmpty())
	{
		d->leDir->setText(path);
		emit validationRequested();
	}
}

bool CfgChatLogsPage::checkDir(const QString &directory)
{
	if (directory.trimmed().isEmpty())
	{
		QMessageBox::critical(this, tr("Directory error"), tr("Directory not specified."));
		return false;
	}

	QFileInfo dir(directory);
	if (!dir.exists())
	{
		QMessageBox::critical(this, tr("Directory error"), tr("Directory doesn't exist."));
		return false;
	}
	QString validationError = validateChatLogsPath(dir);
	if (!validationError.isEmpty())
	{
		QMessageBox::critical(this, tr("Directory error"), validationError);
		return false;
	}
	return true;
}

void CfgChatLogsPage::exploreStorageDirectory()
{
	QString path = d->leDir->text().trimmed();
	if (checkDir(path))
	{
		QDesktopServices::openUrl(QString("file:///%1").arg(
			QDir::toNativeSeparators(path)));
	}
}

void CfgChatLogsPage::readSettings()
{
	ChatLogsCfg cfg;
	d->leDir->setText(cfg.chatLogsRootDir());
	d->cbStoreLogs->setChecked(cfg.isStoreLogs());
	d->cbRestoreLogs->setChecked(cfg.isRestoreChatFromLogs());
	d->groupRemoveOldArchives->setChecked(cfg.isRemoveOldLogs());
	d->spinLogRemovalAge->setValue(cfg.oldLogsRemovalDaysThreshold());
}

void CfgChatLogsPage::saveSettings()
{
	ChatLogsCfg cfg;
	cfg.setChatLogsRootDir(d->leDir->text().trimmed());
	cfg.setStoreLogs(d->cbStoreLogs->isChecked());
	cfg.setRestoreChatFromLogs(d->cbRestoreLogs->isChecked());
	cfg.setRemoveOldLogs(d->groupRemoveOldArchives->isChecked());
	cfg.setOldLogsRemovalDaysThreshold(d->spinLogRemovalAge->value());
}

ConfigPage::Validation CfgChatLogsPage::validate()
{
	QString error = validateChatLogsPath(QFileInfo(d->leDir->text().trimmed()));
	d->lblDirWarning->setToolTip(error);
	d->lblDirWarning->setVisible(!error.isEmpty());
	return error.isEmpty() ? VALIDATION_OK : VALIDATION_ERROR;
}

QString CfgChatLogsPage::validateChatLogsPath(const QFileInfo &path) const
{
	if (path.exists() && !path.isDir())
		return tr("Specified path isn't a directory.");
	return "";
}
