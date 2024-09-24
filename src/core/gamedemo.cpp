//------------------------------------------------------------------------------
// gamedemo.cpp
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
#include "gamedemo.h"

#include "configuration/doomseekerconfig.h"
#include "datapaths.h"
#include "datetime.h"
#include "fileutils.h"
#include "ini/ini.h"
#include "ini/settingsproviderqt.h"
#include "ntfsperm.h"
#include "plugins/engineplugin.h"
#include "plugins/pluginloader.h"
#include "serverapi/serverstructs.h"
#include <cassert>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonDocument>
#include <QTimeZone>
#include <QMessageBox>
#include <QVariant>
#include <QVariantList>

class GameDemoTr : public QObject
{
	Q_OBJECT;

public:
	static QString title()
	{
		return tr("Doomseeker - Record Demo");
	}

	static QString pathDoesntExist(const QString &path, const QString &details)
	{
		QString detailsMsg;
		if (!details.isEmpty())
		{
			detailsMsg = QString("\n") + tr("Error: %1").arg(details);
		}
		return tr("The demo storage directory doesn't exist "
			"and cannot be created!%1\n\n%2").arg(detailsMsg).arg(path);
	}

	static QString pathMissingPermissions(const QString &path)
	{
		return tr("The demo storage directory exists but "
			"lacks the necessary permissions!\n\n%1").arg(path);
	}

private:
	GameDemoTr() = delete;
};

/*
  GameDemo
*/
/**
 * Parse: `Game_03.07.2024_21.54.06[_wad1_wad2...].lmp`
 */
static void loadDemoMetaDataFromFilenameV1(const QString &path, GameDemo &demo)
{
	QString metaData = QFileInfo(path).completeBaseName();
	// We need to split manually to handle escaping.
	QStringList demoData;
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
	// Should have at least 3 elements game, date, time[, iwad[, pwads]]
	if (demoData.size() < 3)
		return;

	QDate date = QDate::fromString(demoData[1], "dd.MM.yyyy");
	QTime time = QTime::fromString(demoData[2], "hh.mm.ss");

	demo.game = demoData[0];
	demo.time = QDateTime(date, time, QTimeZone::systemTimeZone());
	if (demoData.size() >= 4)
	{
		bool hasIwad = false;
		QStringList wadnames = demoData.mid(3);
		for (QString wad : wadnames)
		{
			if (!wad.isEmpty())
			{
				if (!hasIwad)
				{
					demo.iwad = wad;
					hasIwad = true;
				}
				else
				{
					demo.wads << PWad(wad);
				}
			}
		}
	}
}

/**
 * Parse: `Player/2024/Game_2024-02-01T113721Z.lmp`
 */
static void loadDemoMetaDataFromFilenameV2(const QString &path, GameDemo &demo)
{
	QFileInfo fileinfo = path;

	// Get the player info from the directory.
	QDir yearDir = fileinfo.dir();
	if (yearDir.path() != ".")
	{
		QDir playerDir = QFileInfo(yearDir.path()).dir();
		if (playerDir.path() != ".")
		{
			demo.author = playerDir.dirName();
		}
	}

	QStringList tokens = fileinfo.baseName().split("_");
	demo.time = DateTime::fromPathFriendlyUTCISO8601(tokens.takeLast());
	demo.game = tokens.join("_");
}

/**
 * Parse: `Player_Game_2024-02-01T113721.lmp`
 */
static void loadDemoMetaDataFromExportedFilename(const QString &path, GameDemo &demo)
{
	QStringList tokens = QFileInfo(path).baseName().split("_");
	demo.time = DateTime::fromPathFriendlyUTCISO8601(tokens.takeLast());
	if (tokens.size() >= 1)
		demo.game = tokens.takeLast();
	if (tokens.size() >= 1)
		demo.author = tokens.join("_");
}

QString GameDemo::exportedName() const
{
	const QString suffix = QFileInfo(this->demopath).suffix();
	return QString("%1.%2").arg(exportedNameNoExtension(), suffix);
}

QString GameDemo::exportedNameNoExtension() const
{
	static const QRegularExpression mangler("[^A-Za-z0-9-]");

	const QString mangledAuthor = !this->author.isEmpty()
		? QString(this->author)
			.replace(mangler, "")
			.left(MAX_AUTHOR_FILENAME_LEN)
		: "unknownplayer";
	const QString mangledGame = !this->game.isEmpty()
		? QString(this->game).replace(mangler, "")
		: "unknowngame";
	const QString formattedDate = this->time.isValid()
		? DateTime::toPathFriendlyUTCISO8601(this->time)
		: "unknowntime";
	return QString("%1_%2_%3")
		.arg(mangledAuthor)
		.arg(mangledGame)
		.arg(formattedDate);
}

QString GameDemo::managedName() const
{
	const QString suffix = QFileInfo(this->demopath).suffix();
	return QString("%1.%2").arg(managedNameNoExtension(), suffix);
}

QString GameDemo::managedNameNoExtension() const
{
	static const QRegularExpression mangler("[^A-Za-z0-9-]");

	QString mangledAuthor = QString(this->author)
		.replace(mangler, "")
		.left(MAX_AUTHOR_FILENAME_LEN);
	if (mangledAuthor.isEmpty())
		mangledAuthor = "_";
	const QString year = this->time.isValid() ? QString("%1").arg(this->time.date().year(), 4, 10, QChar('0')) : "_";
	const QString mangledGame = QString(this->game)
		.replace(mangler, "");
	const QString formattedDate = this->time.isValid()
		? DateTime::toPathFriendlyUTCISO8601(this->time)
		: "unknown";
	return QString("%1/%2/%3_%4")
		.arg(mangledAuthor)
		.arg(year)
		.arg(mangledGame)
		.arg(formattedDate);
}

void GameDemo::imprintPath(const QString &path)
{
	// Detect the filename format
	static const QRegularExpression detectV1(R"(\d{2}\.\d{2}.\d{4}_\d{2}\.\d{2}\.\d{2})");
	static const QRegularExpression detectV2(R"(\d{4}-\d{2}-\d{2}T\d{6}Z)");

	QString metaData = QFileInfo(path).completeBaseName();
	if (detectV2.match(metaData).hasMatch())
	{
		if (metaData.count("_") > 1)
		{
			loadDemoMetaDataFromExportedFilename(path, *this);
		}
		else
		{
			loadDemoMetaDataFromFilenameV2(path, *this);
		}
	}
	else if (detectV1.match(metaData).hasMatch())
	{
		loadDemoMetaDataFromFilenameV1(path, *this);
	}
}

/*
  DemoRecord
*/
DemoRecord::DemoRecord()
{
	d.control = NoDemo;
}

DemoRecord::DemoRecord(Control control)
{
	d.control = control;
}

DemoRecord::operator DemoRecord::Control() const
{
	return d.control;
}

/*
  DemoStore
*/
static const int DOOMSEEKER_METADATA_VERSION = 2;

/// Filename suffix without the dot, just like QFileInfo::suffix().
static const QString METAFILE_SUFFIX = "ini";
static const QString SECTION_DOOMSEEKER = "doomseeker";
static const QString SECTION_META = "meta";

DClass<DemoStore>
{
public:
	QDir root;
};

DPointered(DemoStore);

DemoStore::DemoStore()
{
	d->root = QDir(gDefaultDataPaths->demosDirectoryPath());
}

DemoStore::DemoStore(QDir root)
{
	d->root = root;
}

QStringList DemoStore::demoFileFilters()
{
	QStringList extensionFilters;
	for (const QString &ext : listDemoExtensions())
		extensionFilters << QString("*.%1").arg(ext);
	return extensionFilters;
}

bool DemoStore::ensureStorageExists(QWidget *parent)
{
	// This is not the proper place for UI code, but it's the convenient
	// one because two different UI items need to call it. Yes, this also
	// means this has the anti-pattern of knowing things about the callers.
	auto popup = [parent](const QString &message) -> bool
	{
		return QMessageBox::Ignore ==
			QMessageBox::warning(parent, GameDemoTr::title(), message,
				QMessageBox::Abort | QMessageBox::Ignore);
	};

	DirErrno mkResult = FileUtils::mkpath(d->root);
	if (mkResult.isError())
	{
		return popup(GameDemoTr::pathDoesntExist(d->root.path(), mkResult.errnoString));
	}

	QFileInfo demoDirInfo(d->root.path());
	++qt_ntfs_permission_lookup;
	bool permissions = demoDirInfo.isReadable()
		&& demoDirInfo.isWritable()
		&& demoDirInfo.isExecutable();
	--qt_ntfs_permission_lookup;
	if (!permissions)
	{
		return popup(GameDemoTr::pathMissingPermissions(d->root.path()));
	}

	return true;
}

QStringList DemoStore::listDemoExtensions()
{
	QStringList demoExtensions;
	for (unsigned i = 0; i < gPlugins->numPlugins(); ++i)
	{
		QString spExt = gPlugins->info(i)->data()->demoExtension;
		if (!demoExtensions.contains(spExt))
			demoExtensions << spExt;

		QString mpExt = gPlugins->info(i)->data()->multiplayerDemoExtension;
		if (!demoExtensions.contains(mpExt))
			demoExtensions << mpExt;
	}
	return demoExtensions;
}

QStringList DemoStore::listManagedDemos()
{
	const QString rootpath = d->root.path();
	QDirIterator dirit(rootpath, demoFileFilters(),
		QDir::Files, QDirIterator::Subdirectories);
	QStringList result;
	while (dirit.hasNext())
	{
		QString path = dirit.next();
		result << path.mid(rootpath.length() + 1);
	}
	return result;
}

bool DemoStore::importDemo(const GameDemo &demo)
{
	QFile importedDemo(demo.demopath);
	if (!importedDemo.exists())
		return false;

	QFile managedDemo(d->root.filePath(demo.managedName()));
	QFile metafile(DemoStore::metafile(managedDemo.fileName()));

	QDir managedDemoDir = QFileInfo(managedDemo).dir();
	if (FileUtils::mkpath(managedDemoDir).isError())
		return false;

	if (managedDemo.exists())
		managedDemo.remove();
	if (metafile.exists())
		metafile.remove();

	if (!importedDemo.copy(managedDemo.fileName()))
		return false;
	saveDemoMetaData(demo, metafile.fileName());
	return metafile.exists();
}

QString DemoStore::metafile(const QString &path)
{
	return path + "." + METAFILE_SUFFIX;
}

QString DemoStore::mkDemoFullPath(DemoRecord::Control control, const EnginePlugin &plugin, const bool isMultiplayer)
{
	GameDemo demo;
	demo.demopath = QString("demo.%1").arg(isMultiplayer
		? plugin.data()->multiplayerDemoExtension
		: plugin.data()->demoExtension);
	demo.author = gConfig.doomseeker.realPlayerName();
	demo.time = QDateTime::currentDateTime();
	demo.game = plugin.nameCanonical();

	const bool extensionAddedByGame = isMultiplayer
		? plugin.data()->multiplayerDemoExtensionAutomatic
		: plugin.data()->demoExtensionAutomatic;

	switch (control)
	{
	case DemoRecord::Managed:
	{
		QFileInfo demoPath(d->root.filePath(extensionAddedByGame
			? demo.managedNameNoExtension()
			: demo.managedName()));
		if (FileUtils::mkpath(demoPath.dir()).isError())
			return QString();
		return demoPath.absoluteFilePath();
	}
	case DemoRecord::Unmanaged:
		return extensionAddedByGame
			? demo.exportedNameNoExtension()
			: demo.exportedName();
	case DemoRecord::NoDemo:
		return QString();
	default:
		assert(0 && "Unknown demo control type");
		return QString();
	}
}

bool DemoStore::removeManagedDemo(const QString &name)
{
	QFile demoFile(d->root.filePath(name));
	if (demoFile.exists())
	{
		if (!demoFile.remove())
			return false;
	}

	// Remove the metadata file as well, but don't bother warning
	// if it can't be deleted for whatever reason.
	QFile metaFile(metafile(demoFile.fileName()));
	metaFile.remove();
	return true;
}

const QDir &DemoStore::root() const
{
	return d->root;
}

void DemoStore::saveDemoMetaData(const QString &demoName, const EnginePlugin &plugin,
	const QString &iwad, const QList<PWad> &pwads, const bool isMultiplayer,
	const QString &gameVersion)
{
	// If the extension is automatic we need to add it here
	bool isDemoExtensionAutomatic = isMultiplayer ?
		plugin.data()->multiplayerDemoExtensionAutomatic : plugin.data()->demoExtensionAutomatic;
	QString demoExtension = isMultiplayer ?
		plugin.data()->multiplayerDemoExtension : plugin.data()->demoExtension;

	QString demoFileName = demoName;
	if (isDemoExtensionAutomatic)
		demoFileName += "." + plugin.data()->demoExtension;
	QString metaFileName = metafile(demoFileName);

	GameDemo demo;
	demo.imprintPath(demoFileName);
	demo.game = plugin.nameCanonical();
	demo.gameVersion = gameVersion;
	demo.author = gConfig.doomseeker.realPlayerName();
	demo.demopath = demoFileName;
	demo.iwad = iwad;
	demo.wads = pwads;

	saveDemoMetaData(demo, metaFileName);
}

void DemoStore::saveDemoMetaData(const GameDemo &demo, const QString &path)
{
	QSettings settings(path, QSettings::IniFormat);
	SettingsProviderQt settingsProvider(&settings);
	Ini metaFile(&settingsProvider);

	IniSection doomseekerSection = metaFile.section(SECTION_DOOMSEEKER);
	doomseekerSection.setValue("version", DOOMSEEKER_METADATA_VERSION);

	IniSection metaSection = metaFile.section(SECTION_META);

	metaSection.setValue("author", demo.author);
	metaSection.setValue("game", demo.game);
	metaSection.setValue("gameVersion", demo.gameVersion);
	metaSection.setValue("createdtime", DateTime::toISO8601(demo.time));

	metaSection.setValue("iwad", demo.iwad.toLower());
	QVariantList wads;
	for (const PWad &wad : demo.wads)
	{
		QVariantList serializedWad;
		serializedWad << wad.name() << wad.isOptional();
		wads << QVariant(serializedWad);
	}

	// Convert the WAD objects to JSON to have them saved in a format
	// that is at least somewhat human-readable.
	metaSection.setValue("wads", QString::fromUtf8(
		QJsonDocument::fromVariant(wads).toJson(QJsonDocument::Compact)));

	// Backward-compatibility with Doomseeker <1.5 (V1).
	QStringList requiredPwads;
	QStringList optionalPwads;
	for (const PWad &wad : demo.wads)
	{
		if (wad.isOptional())
			optionalPwads << wad.name();
		else
			requiredPwads << wad.name();
	}
	metaSection.setValue("pwads", requiredPwads.join(";"));
	metaSection.setValue("optionalPwads", optionalPwads);
}

static void loadDemoMetaDataFromIniV1(Ini &metaData, GameDemo &demo)
{
	demo.iwad = metaData.retrieveSetting(SECTION_META, "iwad").valueString();
	QString pwads = metaData.retrieveSetting(SECTION_META, "pwads");
	if (pwads.length() > 0)
	{
		for (QString wad : pwads.split(";"))
		{
			if (!wad.isEmpty())
				demo.wads << PWad(wad);
		}
	}

	for (QString wad : metaData.retrieveSetting(SECTION_META, "optionalPwads").value().toStringList())
	{
		demo.wads << PWad(wad, true) ;
	}
}

static void loadDemoMetaDataFromIniV2(Ini &metaData, GameDemo &demo)
{
	demo.author = metaData.retrieveSetting(SECTION_META, "author").valueString();
	demo.game = metaData.retrieveSetting(SECTION_META, "game").valueString();
	demo.gameVersion = metaData.retrieveSetting(SECTION_META, "gameVersion").valueString();
	demo.time = QDateTime::fromString(metaData.retrieveSetting(SECTION_META, "createdtime").valueString(), Qt::ISODate);
	demo.iwad = metaData.retrieveSetting(SECTION_META, "iwad").valueString();
	QVariantList wads = QJsonDocument::fromJson(metaData.retrieveSetting(SECTION_META, "wads").valueString().toUtf8()).toVariant().toList();
	for (const QVariant &wad : wads)
	{
		QVariantList wadTokens = wad.toList();
		if (wadTokens.size() >= 2)
		{
			QString name = wadTokens[0].toString();
			bool optional = wadTokens[1].toBool();
			demo.wads << PWad(name, optional);
		}
	}
}

static void loadDemoMetaDataFromIni(const QString &path, GameDemo &demo)
{
	QFileInfo iniFile(path + ".ini");
	if (!iniFile.exists())
		return;

	QSettings settings(iniFile.filePath(), QSettings::IniFormat);
	SettingsProviderQt settingsProvider(&settings);
	Ini metaData(&settingsProvider);
	int version = metaData.section(SECTION_DOOMSEEKER)["version"].value().toInt();

	if (version >= 2) {
		// Try to load any version greater than V2 as V2, assuming that
		// future versions will maintain backward-compatibility with the
		// most recent previous version.
		loadDemoMetaDataFromIniV2(metaData, demo);
	} else {
		loadDemoMetaDataFromIniV1(metaData, demo);
	}
}

GameDemo DemoStore::loadGameDemo(const QString &path)
{
	GameDemo demo;
	demo.demopath = path.endsWith("." + METAFILE_SUFFIX)
		? path.left(path.length() - (1 + METAFILE_SUFFIX.length()))
		: path;
	demo.imprintPath(demo.demopath);
	loadDemoMetaDataFromIni(demo.demopath, demo);
	return demo;
}

GameDemo DemoStore::loadManagedGameDemo(const QString &name)
{
	GameDemo demo;
	QFile demoFile(d->root.filePath(name));
	if (demoFile.exists())
		demo = loadGameDemo(demoFile.fileName());
	return demo;
}

#include "gamedemo.moc"
