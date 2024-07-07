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
#include "demomanager.h"

#include "configuration/doomseekerconfig.h"
#include "datapaths.h"
#include "fileutils.h"
#include "gamedemo.h"
#include "gui/commongui.h"
#include "gui/demometadatadialog.h"
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
#include "templatedpathresolver.h"
#include "ui_demomanager.h"

#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QStandardPaths>

DClass<DemoManagerDlg> : public Ui::DemoManagerDlg
{
public:
	DemoStore demoStore;
	GameDemo *selectedDemo;
	QStandardItemModel *demoModel;
	QList<QList<GameDemo> > demoTree;
};

DPointered(DemoManagerDlg)

DemoManagerDlg::DemoManagerDlg()
{
	d->setupUi(this);
	CommonGUI::setupDialog(*this);
	d->demoStore = DemoStore();
	d->selectedDemo = nullptr;

	auto *menu = new QMenu(this);
	menu->addAction(tr("Export plain demo file"),
		this, SLOT(exportSelectedPlain()));
	menu->addAction(tr("Export with Doomseeker metadata ..."),
		this, SLOT(exportSelectedDoomseeker()));
	d->btnExport->setMenu(menu);

	d->demoModel = new QStandardItemModel();
	adjustDemoList();

	updateUiSelectionState();
	connect(d->demoList->selectionModel(), &QItemSelectionModel::currentChanged,
		this, &DemoManagerDlg::itemSelected);
}

DemoManagerDlg::~DemoManagerDlg()
{
}

void DemoManagerDlg::adjustDemoList()
{
	// In order to index the demos we'll convert the dates to integers by calculating the days until today.
	// Also we need to convert double underscores to a single underscore
	QDate today = QDate::currentDate();
	QTime referenceTime(23, 59, 59);
	QStringList demos = d->demoStore.listManagedDemos();
	typedef QMap<int, GameDemo> DemoMap;
	QMap<int, DemoMap> demoMap;
	for (const QString &demoName : demos)
	{
		GameDemo demo = d->demoStore.loadManagedGameDemo(demoName);
		if (demo.hasDemoFile())
		{
			// TODO when there are two demos recorded at the same time... we're in trouble.
			demoMap[demo.time.date().daysTo(today)][demo.time.time().secsTo(referenceTime)] = demo;
		}
	}

	// Convert to a model
	d->demoModel->clear();
	d->demoTree.clear();
	for (const DemoMap &demoDate : demoMap)
	{
		QStandardItem *item = new QStandardItem(demoDate.begin().value().time.toString("ddd. MMM d, yyyy"));
		QList<GameDemo> demoDateList;
		for (const GameDemo &demo : demoDate)
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
	if (!d->demoStore.removeManagedDemo(file))
		QMessageBox::critical(this, tr("Unable to remove"), tr("Could not remove the selected demo."));
	else
	{
		d->selectedDemo = nullptr;
		updateUiSelectionState();
		return true;
	}
	return false;
}

void DemoManagerDlg::deleteSelected()
{
	if (QMessageBox::question(this, tr("Remove demo?"),
		tr("Are you sure you want to remove the selected demo?"),
		QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes)
	{
		QModelIndex index = d->demoList->selectionModel()->currentIndex();
		if (d->selectedDemo == nullptr)
		{
			int dateRow = index.row();
			for (int timeRow = 0; index.model()->index(timeRow, 0).isValid(); ++timeRow)
			{
				if (doRemoveDemo(d->demoTree[dateRow][timeRow].managedName()))
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
			if (doRemoveDemo(d->selectedDemo->managedName()))
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

void DemoManagerDlg::exportSelectedDoomseeker()
{
	if (d->selectedDemo == nullptr)
		return;

	// Omit Doomseeker's path portablization here, because the feature
	// is explicitly called "export".
	QDir proposedDir = FileUtils::dirOrDir(
		gConfig.doomseeker.previousDemoExportDir,
		QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());

	const QString dirPath = QFileDialog::getExistingDirectory(this,
		tr("Select demo export directory"), proposedDir.path());
	if (dirPath.isEmpty())
		return;

	gConfig.doomseeker.previousDemoExportDir = dirPath;
	const QDir dir(dirPath);

	QFile demoFile(d->selectedDemo->demopath);
	QFile metaFile(DemoStore::metafile(d->selectedDemo->demopath));

	// Check if the files exist at target location and ask to overwrite.
	QFile destDemo(dir.filePath(QFileInfo(demoFile).fileName()));
	QFile destMeta(dir.filePath(QFileInfo(metaFile).fileName()));

	bool demoExists = destDemo.exists();
	bool metaExists = destMeta.exists();
	if (demoExists || metaExists)
	{
		QStringList existingPaths;
		if (demoExists)
			existingPaths << destDemo.fileName();
		if (metaExists)
			existingPaths << destMeta.fileName();
		auto answer = QMessageBox::question(this, tr("Demo export"),
			tr("The exported files already exist at the specified "
				"directory. Overwrite?\n\n%1").arg(existingPaths.join("\n")));
		if (answer == QMessageBox::No)
			return;

		if (demoExists)
			destDemo.remove();
		if (metaExists)
			destMeta.remove();
	}

	// Copy the demo to the new location.
	bool fail = false;
	if (!demoFile.copy(destDemo.fileName()))
	{
		fail = true;
	}

	// Copy the metadata file to the new location.
	if (!fail && metaFile.exists())
	{
		if (!metaFile.copy(destMeta.fileName()))
		{
			fail = true;
			// Delete the already copied demo file to clean-up after an unsuccessful export.
			destDemo.remove();
		}
	}

	if (fail)
	{
		QMessageBox::critical(this, tr("Unable to save"), tr("Could not write to the specified location."));
	}
	return;
}

void DemoManagerDlg::exportSelectedPlain()
{
	if (d->selectedDemo == nullptr)
		return;

	// Omit Doomseeker's path portablization here, because the feature
	// is explicitly called "export".
	QDir dir = FileUtils::dirOrDir(
		gConfig.doomseeker.previousDemoExportDir,
		QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());

	QFileDialog saveDialog(this);
	saveDialog.setAcceptMode(QFileDialog::AcceptSave);
	saveDialog.selectFile(QFileInfo(d->selectedDemo->demopath).fileName());
	saveDialog.setDirectory(dir);
	if (saveDialog.exec() == QDialog::Accepted)
	{
		gConfig.doomseeker.previousDemoExportDir = QFileInfo(saveDialog.selectedFiles().first()).path();
		// Copy the demo to the new location.
		if (!QFile::copy(d->selectedDemo->demopath, saveDialog.selectedFiles().first()))
			QMessageBox::critical(this, tr("Unable to save"), tr("Could not write to the specified location."));
	}
}

void DemoManagerDlg::importDemo()
{
	// Get valid extensions
	QStringList demoExtensions = DemoStore::demoFileFilters();
	demoExtensions << "*.ini";

	QDir proposedDir = FileUtils::dirOrDir(
		gConfig.doomseeker.previousDemoExportDir,
		QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());

	QString selectedFile = QFileDialog::getOpenFileName(this, tr("Import demo"),
		proposedDir.path(),
		tr("Demo files (%1)").arg(demoExtensions.join(" ")));
	if (selectedFile.isEmpty())
		return;

	gConfig.doomseeker.previousDemoExportDir = QFileInfo(selectedFile).path();

	GameDemo demo = DemoStore::loadGameDemo(selectedFile);
	do
	{
		DemoMetaDataDialog importDialog(this, demo);
		importDialog.setWindowTitle(tr("Import game demo"));
		if (importDialog.exec() == QDialog::Accepted)
		{
			GameDemo importedDemo = importDialog.gameDemo();
			importedDemo.demopath = demo.demopath;

			GameDemo existingDemo = d->demoStore.loadManagedGameDemo(importedDemo.managedName());
			if (existingDemo.hasDemoFile())
			{
				QFileInfo existingDemoFile(existingDemo.demopath);
				QFileInfo importedDemoFile(importedDemo.demopath);

				QByteArray existingMd5 = FileUtils::md5(existingDemo.demopath);
				QByteArray importedMd5 = FileUtils::md5(importedDemo.demopath);

				QString question;
				if (existingDemoFile.size() == importedDemoFile.size()
					&& existingMd5 == importedMd5)
				{
					question = tr("It looks like this demo is already imported. Overwrite anyway?");
				}
				else
				{
					question = tr("Another demo with this metadata already exists. Overwrite?");
				}
				QString comparison = tr(
					"Existing: %1\n"
					"New: %2\n")
						.arg(labelDemoFile(existingDemoFile, existingMd5))
						.arg(labelDemoFile(importedDemoFile, importedMd5));

				auto answer = QMessageBox::question(this, tr("Demo import"),
					tr("%1\n\n%2").arg(question, comparison));
				if (answer == QMessageBox::No)
				{
					demo = importedDemo;
					continue;
				}
			}
			if (!d->demoStore.importDemo(importedDemo))
			{
				QMessageBox::critical(this, tr("Demo import"),
					tr("Could not import the selected demo."));
			}
			else

			{
				adjustDemoList();
			}
		}
	} while(false);
}

QString DemoManagerDlg::labelDemoFile(QFileInfo &file, const QByteArray &md5)
{
	return tr("%1 %L2 B (MD5: %3)")
		.arg(file.fileName())
		.arg(file.size())
		.arg(QString::fromLatin1(md5.toHex().toLower()));
}

void DemoManagerDlg::itemDoubleClicked(const QModelIndex &index)
{
	if (d->selectedDemo != nullptr)
		playSelected();
}

void DemoManagerDlg::playSelected()
{
	if (d->selectedDemo == nullptr)
		return;

	// Look for the plugin used to record.
	EnginePlugin *plugin = nullptr;
	for (unsigned i = 0; i < gPlugins->numPlugins(); i++)
	{
		if (d->selectedDemo->game == gPlugins->info(i)->data()->name)
			plugin = gPlugins->info(i);
	}
	if (plugin == nullptr)
	{
		QMessageBox::critical(this, tr("No plugin"),
			tr("The \"%1\" plugin does not appear to be loaded.").arg(d->selectedDemo->game));
		return;
	}

	// Get executable path for pathfinder.
	Message binMessage;
	const QString binPath = gDoomseekerTemplatedPathResolver().resolve(
		GameExeRetriever(*plugin->gameExe()).pathToOfflineExe(binMessage));

	// Locate all the files needed to play the demo
	PathFinder pf;
	pf.addPrioritySearchDir(binPath);
	WadPathFinder wadFinder = WadPathFinder(pf);

	QStringList missingWads;
	QStringList wadPaths;

	QList<PWad> wads = d->selectedDemo->wads;
	wads.prepend(PWad(d->selectedDemo->iwad));

	for (const PWad &wad : wads)
	{
		WadFindResult findResult = wadFinder.find(wad.name());
		if (findResult.isValid())
			wadPaths << findResult.path();
		else if (!wad.isOptional())
			missingWads << wad.name();
	}

	if (!missingWads.isEmpty())
	{
		QMessageBox::critical(this, tr("Files not found"),
			tr("The following files could not be located: ") + missingWads.join(", "));
		return;
	}

	// Play the demo
	GameCreateParams params;
	params.setDemoPath(QFileInfo(d->selectedDemo->demopath).absoluteFilePath());
	params.setIwadPath(wadPaths[0]);
	params.setPwadsPaths(wadPaths.mid(1));
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

void DemoManagerDlg::itemSelected(const QModelIndex &index)
{
	if (!index.isValid() || !index.parent().isValid())
	{
		d->selectedDemo = nullptr;
	}
	else
	{
		int dateRow = index.parent().row();
		int timeRow = index.row();
		d->selectedDemo = &d->demoTree[dateRow][timeRow];
	}
	updateUiSelectionState();
}

void DemoManagerDlg::updateUiSelectionState()
{
	updatePreview();
	d->btnPlay->setEnabled(d->selectedDemo != nullptr);
	d->btnDelete->setEnabled(d->selectedDemo != nullptr);
	d->btnExport->setEnabled(d->selectedDemo != nullptr);
}

void DemoManagerDlg::updatePreview()
{
	if (d->selectedDemo == nullptr)
	{
		d->preview->setText("");
		return;
	}

	static const QString PAR = "<p style=\"margin: 0px 0px 0px 10px\">";
	static const QString ENDPAR = "</p>";

	QString text = "<b>" + tr("Game") + ":</b>" + PAR + d->selectedDemo->game + ENDPAR;
	if (!d->selectedDemo->author.isEmpty())
	{
		text += "<b>" + tr("Author") + ":</b>" + PAR + d->selectedDemo->author + ENDPAR;
	}
	text += "<b>" + tr("WADs / mods") + ":</b>" + PAR;
	text += d->selectedDemo->iwad + "<br />";
	for (const PWad &wad : d->selectedDemo->wads)
	{
		if (wad.isOptional())
			text += "[" + wad.name() + "]";
		else
			text += wad.name();
		text += "<br />";
	}
	text += ENDPAR;
	d->preview->setText(text);
}
