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

#include "datapaths.h"
#include "fileutils.h"
#include "ini/ini.h"
#include "ini/settingsproviderqt.h"
#include "ntfsperm.h"
#include "templatedpathresolver.h"
#include "plugins/engineplugin.h"
#include "serverapi/serverstructs.h"
#include <cassert>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
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

static const int DOOMSEEKER_METADATA_VERSION = 2;

static const QString SECTION_DOOMSEEKER = "doomseeker";
static const QString SECTION_META = "meta";

DemoRecord::DemoRecord()
{
	d.control = NoDemo;
}

DemoRecord::DemoRecord(Control control)
{
	d.control = control;
}

bool DemoRecord::ensureStorageExists(QWidget *parent)
{
	auto popup = [parent](const QString &message) -> bool
	{
		return QMessageBox::Ignore ==
			QMessageBox::warning(parent, GameDemoTr::title(), message,
				QMessageBox::Abort | QMessageBox::Ignore);
	};

	QDir demoDir(gDefaultDataPaths->demosDirectoryPath());
	DirErrno mkResult = FileUtils::mkpath(demoDir);
	if (mkResult.isError())
	{
		return popup(GameDemoTr::pathDoesntExist(demoDir.path(), mkResult.errnoString));
	}

	QFileInfo demoDirInfo(demoDir.path());
	++qt_ntfs_permission_lookup;
	bool permissions = demoDirInfo.isReadable()
		&& demoDirInfo.isWritable()
		&& demoDirInfo.isExecutable();
	--qt_ntfs_permission_lookup;
	if (!permissions)
	{
		return popup(GameDemoTr::pathMissingPermissions(demoDir.path()));
	}

	return true;
}

QString DemoRecord::mkDemoFullPath(Control control, const EnginePlugin &plugin, const bool isMultiplayer)
{
	switch (control)
	{
	case Managed:
		return QFileInfo(gDoomseekerTemplatedPathResolver().resolve(
			gDefaultDataPaths->demosDirectoryPath()) + QDir::separator()
			+ mkDemoName(plugin, isMultiplayer)).absoluteFilePath();
	case Unmanaged:
		return mkDemoName(plugin, isMultiplayer);
	case NoDemo:
		return QString();
	default:
		assert(0 && "Unknown demo control type");
		return QString();
	}
}

QString DemoRecord::mkDemoName(const EnginePlugin &plugin, const bool isMultiplayer)
{
	QString demoName = "";
	demoName += QString("%1_%2").
		arg(plugin.data()->name).
		arg(QDateTime::currentDateTime().toString("dd.MM.yyyy_hh.mm.ss"));
	if (isMultiplayer)
	{
		if (!plugin.data()->multiplayerDemoExtensionAutomatic)
			demoName += QString(".%1").arg(plugin.data()->multiplayerDemoExtension);
	}
	else
	{
		if (!plugin.data()->demoExtensionAutomatic)
			demoName += QString(".%1").arg(plugin.data()->demoExtension);
	}
	return demoName;
}

void DemoRecord::saveDemoMetaData(const QString &demoName, const EnginePlugin &plugin,
	const QString &iwad, const QList<PWad> &pwads, const bool isMultiplayer)
{
	QString metaFileName;
	// If the extension is automatic we need to add it here
	bool isDemoExtensionAutomatic = isMultiplayer ?
		plugin.data()->multiplayerDemoExtensionAutomatic : plugin.data()->demoExtensionAutomatic;
	QString demoExtension = isMultiplayer ?
		plugin.data()->multiplayerDemoExtension : plugin.data()->demoExtension;
	if (isDemoExtensionAutomatic)
	{
		metaFileName = QString("%1.%2.ini").arg(demoName)
			.arg(plugin.data()->demoExtension);
	}
	else
		metaFileName = demoName + ".ini";

	QSettings settings(metaFileName, QSettings::IniFormat);
	SettingsProviderQt settingsProvider(&settings);
	Ini metaFile(&settingsProvider);
	IniSection doomseekerSection = metaFile.section(SECTION_DOOMSEEKER);
	doomseekerSection.createSetting("version", DOOMSEEKER_METADATA_VERSION);

	IniSection metaSection = metaFile.section(SECTION_META);

	metaSection.createSetting("iwad", iwad.toLower());
	QVariantList modfiles;
	for (const PWad &wad : pwads)
	{
		QVariantList serializedModfile;
		serializedModfile << wad.name() << wad.isOptional();
		modfiles << QVariant(serializedModfile);
	}
	metaSection.createSetting("modFiles", QVariant(modfiles));

	// Backward-compatibility with Doomseeker <1.5 (V1).
	QStringList requiredPwads;
	QStringList optionalPwads;
	for (const PWad &wad : pwads)
	{
		if (wad.isOptional())
			optionalPwads << wad.name();
		else
			requiredPwads << wad.name();
	}
	metaSection.createSetting("pwads", requiredPwads.join(";"));
	metaSection.createSetting("optionalPwads", optionalPwads);
}

static void loadDemoMetadataFromFilename(const QString &path, GameDemo &demo)
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
	// Should have at least 3 elements port, date, time[, iwad[, pwads]]
	if (demoData.size() < 3)
		return;

	QDate date = QDate::fromString(demoData[1], "dd.MM.yyyy");
	QTime time = QTime::fromString(demoData[2], "hh.mm.ss");

	demo.port = demoData[0];
	demo.time = QDateTime(date, time);
	if (demoData.size() >= 4)
	{
		for (QString wad : demoData.mid(3))
		{
			if (!wad.isEmpty())
				demo.wads << PWad(wad);
		}
	}
}

static void loadDemoMetadataFromIniV1(Ini &metaData, GameDemo &demo)
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

static void loadDemoMetadataFromIniV2(Ini &metaData, GameDemo &demo)
{
	demo.iwad = metaData.retrieveSetting(SECTION_META, "iwad").valueString();
	QVariantList modfiles = metaData.retrieveSetting(SECTION_META, "modFiles").value().toList();
	for (const QVariant &modfile : modfiles)
	{
		QVariantList modfileTokens = modfile.toList();
		if (modfileTokens.size() >= 2)
		{
			QString name = modfileTokens[0].toString();
			bool optional = modfileTokens[1].toBool();
			demo.wads << PWad(name, optional);
		}
	}
}

static void loadDemoMetadataFromIni(const QString &path, GameDemo &demo)
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
		loadDemoMetadataFromIniV2(metaData, demo);
	} else {
		loadDemoMetadataFromIniV1(metaData, demo);
	}
}

GameDemo DemoRecord::loadGameDemo(const QString &path)
{
	GameDemo demo;
	demo.filepath = path;
	loadDemoMetadataFromFilename(path, demo);
	loadDemoMetadataFromIni(path, demo);
	return demo;
}

DemoRecord::operator DemoRecord::Control() const
{
	return d.control;
}

#include "gamedemo.moc"
