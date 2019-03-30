//------------------------------------------------------------------------------
// odamexgameexefactory.h
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
// Copyright (C) 2019 Pol Marcet Sardà <polmarcetsarda@gmail.com>
//------------------------------------------------------------------------------
#ifndef __ODAMEXGAMEEXEFACTORY_H_
#define __ODAMEXGAMEEXEFACTORY_H_

#include "dptr.h"
#include <serverapi/gameexefactory.h>
#include <QList>

class QString;
class QDir;
class ExeFilePath;

class OdamexGameExeFactory : public GameExeFactory
{
public:
	OdamexGameExeFactory(EnginePlugin *plugin);
	~OdamexGameExeFactory();

protected:
	QList<ExeFilePath> scanSubdir(const QDir &mainDir, const QString &subdirName, int execType) const override;

private:
	QString execName() const;
	QString serverExecName() const;
};

#endif
