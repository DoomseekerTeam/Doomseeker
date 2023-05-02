//------------------------------------------------------------------------------
// updaterinfoparser.cpp
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "updaterinfoparser.h"

#include "log.h"
#include "updater/autoupdater.h"
#include "updater/updatepackage.h"

#include <QJsonDocument>

DClass<UpdaterInfoParser>
{
public:
	QList<UpdatePackage> packages;

	bool hasMainProgramName(const QVariantMap &metaData) const
	{
		return metaData.contains(AutoUpdater::MAIN_PROGRAM_PACKAGE_NAME);
	}
};

DPointered(UpdaterInfoParser)
//////////////////////////////////////////////////////////////////////////////
UpdaterInfoParser::UpdaterInfoParser()
{
}

UpdaterInfoParser::~UpdaterInfoParser()
{
}

const QList<UpdatePackage> &UpdaterInfoParser::packages() const
{
	return d->packages;
}

int UpdaterInfoParser::parse(const QByteArray &json)
{
	d->packages.clear();
	auto var = QJsonDocument::fromJson(json).toVariant();
	if (var.isValid())
	{
		QVariantMap metaData = var.toMap();
		if (d->hasMainProgramName(metaData))
		{
			for (const QString &package : metaData.keys())
			{
				int result = parsePackageNode(package, metaData[package].toMap());
				if (result != AutoUpdater::EC_Ok)
				{
					return result;
				}
			}
		}
		else
		{
			return AutoUpdater::EC_UpdaterInfoMissingMainProgramNode;
		}
	}
	else
	{
		return AutoUpdater::EC_UpdaterInfoCannotParse;
	}
	return AutoUpdater::EC_Ok;
}

int UpdaterInfoParser::parsePackageNode(const QString &packageName, const QVariantMap &map)
{
	UpdatePackage package;
	package.name = packageName;

	if (map.contains("revision"))
	{
		package.revision = map["revision"].toString();
	}
	else
	{
		gLog << tr("Missing update revision info for package %1.").arg(packageName);
		return AutoUpdater::EC_MissingRevisionInfo;
	}

	if (map.contains("display-version"))
	{
		package.displayVersion = map["display-version"].toString();
	}
	else
	{
		package.displayVersion = package.revision;
	}

	if (map.contains("display-name"))
	{
		package.displayName = map["display-name"].toString();
	}
	else
	{
		package.displayName = packageName;
	}

	if (map.contains("URL"))
	{
		QString strUrl = map["URL"].toString();
		package.downloadUrl = strUrl;
		if (!package.downloadUrl.isValid() || package.downloadUrl.isRelative())
		{
			gLog << tr("Invalid update download URL for package %1: %2")
				.arg(packageName, strUrl);
			return AutoUpdater::EC_InvalidDownloadUrl;
		}
	}
	else
	{
		gLog << tr("Missing update download URL for package %1.").arg(packageName);
		return AutoUpdater::EC_MissingDownloadUrl;
	}

	if (map.contains("URL-script"))
	{
		QString strUrl = map["URL-script"].toString();
		package.downloadScriptUrl = strUrl;
		if (!package.downloadScriptUrl.isValid() || package.downloadScriptUrl.isRelative())
		{
			gLog << tr("Invalid update script download URL for package %1, %2")
				.arg(packageName, strUrl);
			return AutoUpdater::EC_InvalidDownloadUrl;
		}
	}
	else
	{
		package.downloadScriptUrl = package.downloadUrl.toString() + ".xml";
	}

	d->packages << package;
	return AutoUpdater::EC_Ok;
}
