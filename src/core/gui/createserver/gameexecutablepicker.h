//------------------------------------------------------------------------------
// gameexecutablepicker.h
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
#ifndef idf35ab110_046d_4d94_b9be_b4ee43423df5
#define idf35ab110_046d_4d94_b9be_b4ee43423df5

#include "dptr.h"
#include <QString>
#include <QWidget>

class EnginePlugin;
class GameFileList;

class GameExecutablePicker : public QWidget
{
	Q_OBJECT

public:
	GameExecutablePicker(QWidget *parent);
	~GameExecutablePicker() override;

	void reloadExecutables();
	QString path() const;
	void setPath(const QString &path);
	/**
	 * @param execs
	 *     Binary OR of GameFile::ExecType.
	 */
	void setAllowedExecutables(int execs);
	void setPlugin(EnginePlugin *plugin);

private:
	DPtr<GameExecutablePicker> d;

	void add(const QString &path);
	GameFileList gameExecutables() const;
	void showWarning(const QString &msg);

private slots:
	void browse();
	void setExecutableToDefault();
};

#endif
