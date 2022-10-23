//------------------------------------------------------------------------------
// gameexecutablepicker.cpp
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
// Copyright (C) 2015 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "gameexecutablepicker.h"

#include "configuration/doomseekerconfig.h"
#include "filefilter.h"
#include "gui/commongui.h"
#include "ini/ini.h"
#include "plugins/engineplugin.h"
#include "serverapi/exefile.h"
#include "serverapi/gameexefactory.h"
#include "serverapi/gamefile.h"
#include "ui_gameexecutablepicker.h"
#include <QFileDialog>

DClass<GameExecutablePicker> : public Ui::GameExecutablePicker
{
public:
	int allowedExecs;
	EnginePlugin *plugin;
};
DPointered(GameExecutablePicker)

GameExecutablePicker::GameExecutablePicker(QWidget *parent)
	: QWidget(parent)
{
	d->setupUi(this);
	d->allowedExecs = 0;
	d->plugin = nullptr;

	showWarning("");
}

GameExecutablePicker::~GameExecutablePicker()
{
}

void GameExecutablePicker::add(const QString &path)
{
	if (!path.trimmed().isEmpty() && d->executableInput->findText(path) < 0)
		d->executableInput->addItem(path);
}

void GameExecutablePicker::browse()
{
	showWarning("");
	QString dialogDir = gConfig.doomseeker.previousCreateServerExecDir;
	QString path = QFileDialog::getOpenFileName(this, tr("Doomseeker - Browse executable"),
		dialogDir, FileFilter::executableFilesFilter());

	if (!path.isEmpty())
	{
		QFileInfo fi(path);
		gConfig.doomseeker.previousCreateServerExecDir = fi.absolutePath();

		d->executableInput->setCurrentText(fi.absoluteFilePath());
		add(path);
	}
}


GameFileList GameExecutablePicker::gameExecutables() const
{
	GameFileList files = d->plugin->gameExe()->gameFiles();
	GameFileList candidates = GameFiles::allFlagMatchExecutables(files, d->allowedExecs);
	if (d->allowedExecs & GameFile::Client)
		candidates.prepend(GameFiles::defaultClientExecutable(files));
	else if (d->allowedExecs & GameFile::Server)
		candidates.prepend(GameFiles::defaultServerExecutable(files));
	else if (d->allowedExecs & GameFile::Offline)
		candidates.prepend(GameFiles::defaultOfflineExecutable(files));
	return candidates;
}

QString GameExecutablePicker::path() const
{
	return d->executableInput->currentText();
}

void GameExecutablePicker::setPath(const QString &path)
{
	d->executableInput->setCurrentText(path);
}

void GameExecutablePicker::setExecutableToDefault()
{
	showWarning("");
	IniSection *cfg = d->plugin->data()->pConfig;
	if (cfg == nullptr)
	{
		showWarning(tr("Plugin doesn't support configuration."));
		return;
	}

	GameFileList execs = gameExecutables();
	if (execs.isEmpty())
	{
		showWarning(tr("Game doesn't define any executables for this game setup."));
		return;
	}

	for (const GameFile &candidate : execs.asQList())
	{
		QString path = cfg->value(candidate.configName()).toString();
		if (!path.isEmpty())
		{
			d->executableInput->setCurrentText(path);
			return;
		}
	}
	showWarning(tr("Default executable for this game isn't configured."));
}

void GameExecutablePicker::setAllowedExecutables(int execs)
{
	d->allowedExecs = execs;
	reloadExecutables();
}

void GameExecutablePicker::setPlugin(EnginePlugin *plugin)
{
	d->plugin = plugin;
	reloadExecutables();
}

void GameExecutablePicker::showWarning(const QString &msg)
{
	d->lblWarning->setVisible(!msg.trimmed().isEmpty());
	d->lblWarning->setToolTip(msg);
}

void GameExecutablePicker::reloadExecutables()
{
	showWarning("");
	if (d->plugin == nullptr)
	{
		showWarning(tr("Game plugin not set."));
		return;
	}
	QString currentExec = d->executableInput->currentText();
	d->executableInput->clear();
	IniSection *cfg = d->plugin->data()->pConfig;
	if (cfg == nullptr)
	{
		d->executableInput->setCurrentText(currentExec);
		return;
	}

	GameFileList files = gameExecutables();
	for (const GameFile &file : files.asQList())
	{
		add(cfg->value(file.configName()).toString());
	}
	for (const ExeFilePath &exe : d->plugin->gameExe()->additionalExecutables(d->allowedExecs))
	{
		QFileInfo fileInfo(exe.path());
		if (fileInfo.isFile())
			add(exe.path());
	}

	if (d->executableInput->findText(currentExec) >= 0)
		d->executableInput->setCurrentText(currentExec);
	else
		setExecutableToDefault();
}
