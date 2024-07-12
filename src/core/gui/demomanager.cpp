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
#include "fileutils.h"
#include "gamedemo.h"
#include "gui/commongui.h"
#include "gui/demometadatadialog.h"
#include "pathfinder/pathfinder.h"
#include "pathfinder/wadpathfinder.h"
#include "plugins/enginedefaults.h"
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
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QString>
#include <cstdint>

namespace
{
class DemoModel : public QStandardItemModel
{
	Q_OBJECT;

public:
	enum Column : int8_t
	{
		ColData = 0,
		ColGame = 0,
		ColCreatedTime,
		ColAuthor,
		ColIwad,
		ColWads,
	};

	enum Role : int
	{
		ManagedNameRole = Qt::UserRole + 1,
		FilePathRole = Qt::UserRole + 2,
	};

	DemoModel(QObject *parent) : QStandardItemModel(parent)
	{
		setHorizontalHeaderLabels({"", tr("Created Time"), tr("Author"), tr("IWAD"), tr("WADs")});
	}

	void addGameDemo(const GameDemo &demo)
	{
		auto gameinfo = this->gameInfo(demo.game);
		auto *itemGame = new QStandardItem;
		itemGame->setIcon(gameinfo.icon);
		itemGame->setToolTip(gameinfo.name);

		auto *itemCreatedTime = new QStandardItem;
		itemCreatedTime->setData(demo.time, Qt::EditRole);
		itemCreatedTime->setToolTip(demo.time.toString());

		auto *itemAuthor = new QStandardItem(demo.author);
		itemAuthor->setToolTip(demo.author);

		auto *itemIwad = new QStandardItem(demo.iwad);
		itemIwad->setToolTip(demo.iwad);

		QStringList wads;
		for (const PWad &wad : demo.wads)
		{
			wads << (wad.isOptional()
				? ("[" + wad.name() + "]")
				: wad.name());
		}
		auto *itemWads = new QStandardItem(wads.join("; "));
		itemWads->setToolTip(wads.join("\n"));

		QStandardItem *itemData = itemGame;
		itemData->setData(demo.managedName(), ManagedNameRole);
		itemData->setData(demo.demopath, FilePathRole);

		appendRow(QList<QStandardItem *> {itemGame, itemCreatedTime, itemAuthor, itemIwad, itemWads});
	}

	QString demoNameFromIndex(const QModelIndex &index) const
	{
		return index.isValid() ? demoNameFromRow(index.row()) : QString();
	}

	QString demoNameFromRow(int row) const
	{
		if (row >= this->rowCount())
			return QString();
		auto *item = this->item(row, ColData);
		return item->data(FilePathRole).toString();
	}

	void removeAll()
	{
		this->removeRows(0, this->rowCount());
	}

private:
	struct GameInfo
	{
		QIcon icon;
		QString name;

		static GameInfo unknown(QString name)
		{
			return GameInfo {gUnknownEngineIcon(), name};
		}
	};

	/// A beyond-reasonable limit of games Doomseeker will support.
	static const int GAMES_LIMIT = 10000;

	mutable QMap<QString, GameInfo> games;

	GameInfo gameInfo(const QString &name) const
	{
		if (this->games.contains(name))
			return this->games[name];
		if (this->games.size() > GAMES_LIMIT)
		{
			// Something fishy is going on if we went beyond this limit.
			// Protect against DoS.
			return GameInfo::unknown(name);
		}
		GameInfo gameinfo = gameInfoFromPlugin(name);
		this->games[name] = gameinfo;
		return gameinfo;
	}

	static GameInfo gameInfoFromPlugin(const QString &name)
	{
		EnginePlugin *plugin = gPlugins->info(name);
		return plugin != nullptr
			? GameInfo {plugin->icon(), plugin->data()->name}
			: GameInfo::unknown(name);
	}
};
}

DClass<DemoManagerDlg> : public Ui::DemoManagerDlg
{
public:
	DemoStore demoStore;
	DemoModel *demoModel;

	QModelIndex currentIndex() const
	{
		return this->demoTable->selectionModel()->currentIndex();
	}
};

DPointered(DemoManagerDlg)

DemoManagerDlg::DemoManagerDlg()
{
	d->setupUi(this);
	CommonGUI::setupDialog(*this);
	d->demoStore = DemoStore();

	auto *menu = new QMenu(this);
	menu->addAction(tr("Export plain demo file"),
		this, SLOT(exportSelectedPlain()));
	menu->addAction(tr("Export with Doomseeker metadata ..."),
		this, SLOT(exportSelectedDoomseeker()));
	d->btnExport->setMenu(menu);

	d->demoModel = new DemoModel(this);
	d->demoTable->setModel(d->demoModel);
	d->demoTable->setColumnWidth(DemoModel::ColGame, 24);

	auto *header = d->demoTable->horizontalHeader();
	header->setSectionResizeMode(DemoModel::ColGame, QHeaderView::ResizeToContents);

	adjustDemoList();
	d->demoTable->sortByColumn(DemoModel::ColCreatedTime, Qt::DescendingOrder);

	updateUiSelectionState();
	connect(d->demoTable->selectionModel(), &QItemSelectionModel::currentChanged,
		this, &DemoManagerDlg::itemSelected);
}

DemoManagerDlg::~DemoManagerDlg()
{
}

void DemoManagerDlg::adjustDemoList()
{
	d->demoModel->removeAll();
	QStringList demos = d->demoStore.listManagedDemos();
	d->demoTable->setSortingEnabled(false);
	for (const QString &demoName : demos)
	{
		GameDemo demo = d->demoStore.loadManagedGameDemo(demoName);
		d->demoModel->addGameDemo(demo);
	}
	d->demoTable->setSortingEnabled(true);
	d->demoTable->resizeRowsToContents();
}

bool DemoManagerDlg::doRemoveDemo(const QString &file)
{
	if (!d->demoStore.removeManagedDemo(file))
		QMessageBox::critical(this, tr("Unable to remove"), tr("Could not remove the selected demo."));
	else
	{
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
		QModelIndex index = d->currentIndex();
		if (index.isValid())
		{
			const QString &demoName = d->demoModel->demoNameFromIndex(index);
			if (doRemoveDemo(demoName))
			{
				d->demoModel->removeRow(index.row());
			}
		}
	}
}

void DemoManagerDlg::exportSelectedDoomseeker()
{
	QString demoName = d->demoModel->demoNameFromIndex(d->demoTable->currentIndex());
	if (demoName.isEmpty())
		return;
	GameDemo selectedDemo = d->demoStore.loadManagedGameDemo(demoName);

	// Omit Doomseeker's path portablization here, because the feature
	// is explicitly called "export".
	QDir proposedDir = FileUtils::dirOrDir(
		gConfig.doomseeker.previousDemoExportDir,
		QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());

	const QString exportPath = QFileDialog::getSaveFileName(this,
		tr("Select demo export directory"),
		proposedDir.filePath(selectedDemo.exportedName()));
	if (exportPath.isEmpty())
		return;

	gConfig.doomseeker.previousDemoExportDir = QFileInfo(exportPath).path();

	QFile demoFile(selectedDemo.demopath);
	QFile metaFile(DemoStore::metafile(selectedDemo.demopath));

	// Check if the files exist at target location and ask to overwrite.
	QFile destDemo(exportPath);
	QFile destMeta(DemoStore::metafile(exportPath));

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
	QString demoName = d->demoModel->demoNameFromIndex(d->demoTable->currentIndex());
	if (demoName.isEmpty())
		return;
	GameDemo selectedDemo = d->demoStore.loadManagedGameDemo(demoName);

	// Omit Doomseeker's path portablization here, because the feature
	// is explicitly called "export".
	QDir dir = FileUtils::dirOrDir(
		gConfig.doomseeker.previousDemoExportDir,
		QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());

	QFileDialog saveDialog(this);
	saveDialog.setAcceptMode(QFileDialog::AcceptSave);
	saveDialog.selectFile(selectedDemo.exportedName());
	saveDialog.setDirectory(dir);
	if (saveDialog.exec() == QDialog::Accepted)
	{
		gConfig.doomseeker.previousDemoExportDir = QFileInfo(saveDialog.selectedFiles().first()).path();
		// Copy the demo to the new location.
		if (!QFile::copy(selectedDemo.demopath, saveDialog.selectedFiles().first()))
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
	QString demoName = d->demoModel->demoNameFromIndex(index);
	if (!demoName.isEmpty())
		playDemo(demoName);
}

void DemoManagerDlg::playSelected()
{
	QString demoName = d->demoModel->demoNameFromIndex(d->demoTable->currentIndex());
	if (!demoName.isEmpty())
		playDemo(demoName);
}

void DemoManagerDlg::playDemo(const QString &demoName)
{
	GameDemo demo = d->demoStore.loadManagedGameDemo(demoName);

	// Look for the plugin used to record.
	EnginePlugin *plugin = gPlugins->info(demo.game);
	if (plugin == nullptr)
	{
		QMessageBox::critical(this, tr("No plugin"),
			tr("The \"%1\" plugin does not appear to be loaded.").arg(demo.game));
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

	QList<PWad> wads = demo.wads;
	wads.prepend(PWad(demo.iwad));

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
	params.setDemoPath(QFileInfo(demo.demopath).absoluteFilePath());
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

void DemoManagerDlg::itemSelected(const QModelIndex &index)
{
	updateUiSelectionState();
}

void DemoManagerDlg::updateUiSelectionState()
{
	updatePreview();

	bool selected = d->demoTable->currentIndex().isValid();
	d->btnPlay->setEnabled(selected);
	d->btnDelete->setEnabled(selected);
	d->btnExport->setEnabled(selected);
}

void DemoManagerDlg::updatePreview()
{
	QString demoName = d->demoModel->demoNameFromIndex(d->demoTable->currentIndex());
	if (demoName.isEmpty())
	{
		d->preview->setText("");
		return;
	}
	GameDemo selectedDemo = d->demoStore.loadManagedGameDemo(demoName);

	static const QString PAR = "<p style=\"margin: 0px 0px 0px 10px\">";
	static const QString ENDPAR = "</p>";

	// Try to get the "display" name of the plugin.
	EnginePlugin *plugin = gPlugins->info(selectedDemo.game);
	QString gameName = plugin != nullptr ? plugin->data()->name : selectedDemo.game;

	QString text = "<b>" + tr("Game") + ":</b>" + PAR + gameName + ENDPAR;
	if (!selectedDemo.author.isEmpty())
	{
		text += "<b>" + tr("Author") + ":</b>" + PAR + selectedDemo.author + ENDPAR;
	}
	text += "<b>" + tr("WADs / mods") + ":</b>" + PAR;
	text += selectedDemo.iwad + "<br />";
	for (const PWad &wad : selectedDemo.wads)
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

#include "demomanager.moc"
