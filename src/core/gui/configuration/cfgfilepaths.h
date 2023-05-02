//------------------------------------------------------------------------------
// cfgfilepaths.h
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
// Copyright (C) 2009 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------

#ifndef __CFG_FILE_PATHS_H_
#define __CFG_FILE_PATHS_H_

#include "dptr.h"
#include "gui/configuration/configpage.h"

class FileSearchPath;
class QFileInfo;

class CFGFilePaths : public ConfigPage
{
	Q_OBJECT

public:
	CFGFilePaths(QWidget *parent = nullptr);
	~CFGFilePaths() override;

	void readSettings() override;
	Validation validate() override;

protected:
	void addPath(const FileSearchPath &fileSearchPath);

	QIcon icon() const override;

	bool isPathAlreadyDefined(const QString &path);

	QString name() const override
	{
		return tr("File Paths");
	}

	void saveSettings() override;

protected slots:
	void btnAddWadPath_Click();
	void btnRemoveWadPath_Click();

private:
	DPtr<CFGFilePaths> d;

	QString validatePath(const QString &path) const;
};

#endif
