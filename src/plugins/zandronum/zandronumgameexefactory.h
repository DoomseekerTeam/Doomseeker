//------------------------------------------------------------------------------
// zandronumgameexefactory.h
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
#ifndef id11524123_87af_4cee_82b2_c7f39783c02c
#define id11524123_87af_4cee_82b2_c7f39783c02c

#include "dptr.h"
#include <QDir>
#include <QList>
#include <QString>
#include <serverapi/gameexefactory.h>

class ExeFilePath;

class ZandronumGameExeFactory : public GameExeFactory
{
public:
	ZandronumGameExeFactory(EnginePlugin *plugin);
	~ZandronumGameExeFactory() override;

private:
	DPtr<ZandronumGameExeFactory> d;

	QList<ExeFilePath> additionalExecutables(int execType) const;
	QString execName() const;
	QList<ExeFilePath> scanSubdir(const QDir &mainDir, const QString &subdirName, int execType) const;
	QString serverExecName() const;
};

#endif
