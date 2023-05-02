//------------------------------------------------------------------------------
// ip2cloader.h
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
// Copyright (C) 2013 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef idBE97C916_2BFB_4C8C_B530CA666606FA7D
#define idBE97C916_2BFB_4C8C_B530CA666606FA7D

#include "dptr.h"

#include <QObject>

class IP2CLoader : public QObject
{
	Q_OBJECT

public:
	IP2CLoader(QObject *parent = nullptr);
	~IP2CLoader() override;

	void load();

public slots:
	void update();

signals:
	void downloadProgress(qint64 current, qint64 total);
	void finished();

private:
	DPtr<IP2CLoader> d;

	void ip2cJobsFinished();
	void ip2cParseDatabase();

private slots:
	void ip2cFinishUpdate(const QByteArray &downloadedData);
	void ip2cFinishedParsing(bool bSuccess);
	void onUpdateNeeded(int status);
};

#endif
