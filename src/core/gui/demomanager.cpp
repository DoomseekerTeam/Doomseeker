//------------------------------------------------------------------------------
// demomanager.h
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
// Copyright (C) 2011 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------

#include "datapaths.h"
#include "demomanager.h"
#include "ini/ini.h"
#include "ini/settingsproviderqt.h"
#include "pathfinder/pathfinder.h"
#include "pathfinder/wadpathfinder.h"
#include "plugins/engineplugin.h"
#include "plugins/pluginloader.h"
#include "serverapi/gamecreateparams.h"
#include "serverapi/gameexeretriever.h"
#include "serverapi/gamehost.h"
#include "serverapi/message.h"
#include "serverapi/server.h"
#include "ui_demomanager.h"

#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>

class Demo
{
public:
	QString filename;
	QString port;
	QDateTime time;
	QStringList wads;
	QStringList optionalWads;
};

DClass<DemoManagerDlg> : public Ui::DemoManagerDlg
{
public:
	Demo *selectedDemo;
	QStandardItemModel *demoModel;
	QList<QList<Demo> > demoTree;
};

DPointered(DemoManagerDlg)

DemoManagerDlg::DemoManagerDlg()
{
	d->setupUi(this);
	d->selectedDemo = nullptr;

	d->demoModel = new QStandardItemModel();
	adjustDemoList();

	connect(d->demoList->selectionModel(), SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)), this, SLOT(updatePreview(const QModelIndex&)));
}

DemoManagerDlg::~DemoManagerDlg()
{
}

void DemoManagerDlg::adjustDemoList()
{
	// Get valid extensions
	QStringList demoExtensions;
	for (unsigned i = 0; i < gPlugins->numPlugins(); ++i)
	{
		QString ext = QString("*.%1").arg(gPlugins->info(i)->data()->demoExtension);

		if (!demoExtensions.contains(ext))
			demoExtensions << ext;
	}

	// In order to index the demos we'll convert the dates to integers by calculating the days until today.
	// Also we need to convert double underscores to a single underscore
	QDate today = QDate::currentDate();
	QTime referenceTime(23, 59, 59);
	QDir demosDirectory(gDefaultDataPaths->demosDirectoryPath());
	QStringList demos = demosDirectory.entryList(demoExtensions, QDir::Files);
	typedef QMap<int, Demo> DemoMap;
	QMap<int, DemoMap> demoMap;
	for (const QString &demoName : demos)
	{
		QStringList demoData;
		QString metaData = demoName.left(demoName.lastIndexOf("."));
		// We need to split manually to handle escaping.
		for (int i = 0; i < metaData.length(); ++i)
		{
			if (metaData[i] == '_')
			{
				// If our underscore is followed by another just continue on...
				if (i + 1 < metaData.length() && metaData[i + 1] == '_')
				{
					++i;
					continue;
				}

				// Split the meta data and then restart from the beginning.
				demoData << metaData.left(i).replace("__", "_");
				metaData = metaData.mid(i + 1);
				i = 0;
			}
		}
		// Whatever is left is a part of our data.
		demoData << metaData.replace("__", "_");
		if (demoData.size() < 3) // Should have at least 3 elements port, date, time[, iwad[, pwads]]
			continue;

		QDate date = QDate::fromString(demoData[1], "dd.MM.yyyy");
		QTime time = QTime::fromString(demoData[2], "hh.mm.ss");
		Demo demo;
		demo.filename = demoName;
		demo.port = demoData[0];
		demo.time = QDateTime(date, time);
		if (demoData.size() >= 4)
			demo.wads = demoData.mid(3);
		else
		{
			// New format, read meta data from file!
			QSettings settings(
				gDefaultDataPaths->demosDirectoryPath() + QDir::separator() + demoName + ".ini",
				QSettings::IniFormat);
			SettingsProviderQt settingsProvider(&settings);
			Ini metaData(&settingsProvider);
			demo.wads << metaData.retrieveSetting("meta", "iwad");
			QString pwads = metaData.retrieveSetting("meta", "pwads");
			if (pwads.length() > 0)
				demo.wads << pwads.split(";");
			demo.optionalWads = metaData.retrieveSetting("meta", "optionalPwads").value().toStringList();
		}

		demoMap[date.daysTo(today)][time.secsTo(referenceTime)] = demo;
	}

	// Convert to a model
	d->demoModel->clear();
	d->demoTree.clear();
	for (const DemoMap &demoDate : demoMap)
	{
		QStandardItem *item = new QStandardItem(demoDate.begin().value().time.toString("ddd. MMM d, yyyy"));
		QList<Demo> demoDateList;
		for (const Demo &demo : demoDate)
		{
			demoDateList << demo;
			item->appendRow(new QStandardItem(demo.time.toString("hh:mm:ss")));
		}
		d->demoTree << demoDateList;
		d->demoModel->appendRow(item);
	}
	d->demoList->setModel(d->demoModel);
}

bool DemoManagerDlg::doRemoveDemo(const QString &file)
{
	if (!QFile::remove(file))
		QMessageBox::critical(this, tr("Unable to delete"), tr("Could not delete the selected demo."));
	else
	{
		// Remove ini file as well, but don't bother warning if it can't be deleted for whatever reason
		QFile::remove(file + ".ini");
		d->selectedDemo = nullptr;
		return true;
	}
	return false;
}

void DemoManagerDlg::deleteSelected()
{
	if (QMessageBox::question(this, tr("Delete demo?"),
		tr("Are you sure you want to delete the selected demo?"),
		QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes)
	{
		QModelIndex index = d->demoList->selectionModel()->currentIndex();
		if (d->selectedDemo == nullptr)
		{
			int dateRow = index.row();
			for (int timeRow = 0; index.model()->index(timeRow, 0).isValid(); ++timeRow)
			{
				if (doRemoveDemo(gDefaultDataPaths->demosDirectoryPath() + QDir::separator() + d->demoTree[dateRow][timeRow].filename))
				{
					d->demoModel->removeRow(timeRow, index);
					d->demoTree[dateRow].removeAt(timeRow);
					if (d->demoTree[dateRow].size() == 0)
					{
						d->demoModel->removeRow(dateRow);
						d->demoTree.removeAt(dateRow);
						break;
					}

					// We deleted the top row, so decrement our pointer
					--timeRow;
				}
			}
		}
		else
		{
			if (doRemoveDemo(gDefaultDataPaths->demosDirectoryPath() + QDir::separator() + d->selectedDemo->filename))
			{
				// Adjust the tree
				int dateRow = index.parent().row();
				int timeRow = index.row();

				d->demoModel->removeRow(timeRow, index.parent());
				d->demoTree[dateRow].removeAt(timeRow);
				if (d->demoTree[dateRow].size() == 0)
				{
					d->demoModel->removeRow(dateRow);
					d->demoTree.removeAt(dateRow);
				}
			}
		}
	}
}

void DemoManagerDlg::exportSelected()
{
	if (d->selectedDemo == nullptr)
		return;

	QFileDialog saveDialog(this);
	saveDialog.setAcceptMode(QFileDialog::AcceptSave);
	saveDialog.selectFile(d->selectedDemo->filename);
	if (saveDialog.exec() == QDialog::Accepted)
	{
		// Copy the demo to the new location.
		if (!QFile::copy(gDefaultDataPaths->demosDirectoryPath() + QDir::separator() + d->selectedDemo->filename, saveDialog.selectedFiles().first()))
			QMessageBox::critical(this, tr("Unable to save"), tr("Could not write to the specified location."));
	}
}

void DemoManagerDlg::playSelected()
{
	if (d->selectedDemo == nullptr)
		return;

	// Look for the plugin used to record.
	EnginePlugin *plugin = nullptr;
	for (unsigned i = 0; i < gPlugins->numPlugins(); i++)
	{
		if (d->selectedDemo->port == gPlugins->info(i)->data()->name)
			plugin = gPlugins->info(i);
	}
	if (plugin == nullptr)
	{
		QMessageBox::critical(this, tr("No plugin"),
			tr("The \"%1\" plugin does not appear to be loaded.").arg(d->selectedDemo->port));
		return;
	}

	// Get executable path for pathfinder.
	Message binMessage;
	QString binPath = GameExeRetriever(*plugin->gameExe()).pathToOfflineExe(binMessage);

	// Locate all the files needed to play the demo
	PathFinder pf;
	pf.addPrioritySearchDir(binPath);
	WadPathFinder wadFinder = WadPathFinder(pf);

	QStringList missingWads;
	QStringList wadPaths;

	for (const QString &wad : d->selectedDemo->wads)
	{
		WadFindResult findResult = wadFinder.find(wad);
		if (findResult.isValid())
			wadPaths << findResult.path();
		else
			missingWads << wad;
	}

	if (!missingWads.isEmpty())
	{
		QMessageBox::critical(this, tr("Files not found"),
			tr("The following files could not be located: ") + missingWads.join(", "));
		return;
	}
	QStringList optionalWadPaths;
	for (const QString &wad : d->selectedDemo->optionalWads)
	{
		WadFindResult findResult = wadFinder.find(wad);
		if (findResult.isValid())
			optionalWadPaths << findResult.path();
	}

	// Play the demo
	GameCreateParams params;
	params.setDemoPath(gDefaultDataPaths->demosDirectoryPath()
		+ QDir::separator() + d->selectedDemo->filename);
	params.setIwadPath(wadPaths[0]);
	params.setPwadsPaths(wadPaths.mid(1) + optionalWadPaths);
	params.setHostMode(GameCreateParams::Demo);
	params.setExecutablePath(binPath);

	GameHost *gameRunner = plugin->gameHost();
	Message message = gameRunner->host(params);

	if (message.isError())
		QMessageBox::critical(this, tr("Doomseeker - error"), message.contents());

	delete gameRunner;
}


void DemoManagerDlg::performAction(QAbstractButton *button)
{
	if (button == d->buttonBox->button(QDialogButtonBox::Close))
		reject();
}

void DemoManagerDlg::updatePreview(const QModelIndex &index)
{
	if (!index.isValid() || !index.parent().isValid())
	{
		d->preview->setText("");
		d->selectedDemo = nullptr;
		return;
	}

	int dateRow = index.parent().row();
	int timeRow = index.row();
	d->selectedDemo = &d->demoTree[dateRow][timeRow];

	QString text = "<b>" + tr("Port") + ":</b><p style=\"margin: 0px 0px 0px 10px\">" + d->selectedDemo->port + "</p>" +
		"<b>" + tr("WADs") + ":</b><p style=\"margin: 0px 0px 0px 10px\">";
	for (const QString &wad : d->selectedDemo->wads)
	{
		text += wad + "<br />";
	}
	for (const QString &wad : d->selectedDemo->optionalWads)
	{
		text += "[" + wad + "]<br />";
	}
	text += "</p>";
	d->preview->setText(text);
}
