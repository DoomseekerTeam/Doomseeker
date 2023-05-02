//------------------------------------------------------------------------------
// modinstall.h
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
#ifndef id33bf644f_e0e7_4b53_8c65_742992226eed
#define id33bf644f_e0e7_4b53_8c65_742992226eed

#include "dptr.h"
#include "wadseekerexportinfo.h"
#include <QObject>
#include <QString>

class ModSet;
class WadDownloadInfo;

class WADSEEKER_API ModInstall : public QObject
{
	Q_OBJECT

public:
	ModInstall(QObject *parent = nullptr);
	~ModInstall() override;

	const QString &error() const;
	bool isError() const;
	void install(const QString &targetDir, const ModSet &modSet);

signals:
	void fileDownloadProgress(const QString &filename, qint64 current, qint64 total);
	void finished();

private:
	DPtr<ModInstall> d;

	void errorOut(const QString &msg);

private slots:
	void failInstallation();
	void onWadDownloadProgress(const WadDownloadInfo &wadDownloadInfo,
		qint64 current, qint64 total);
};

#endif
