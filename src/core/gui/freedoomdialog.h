//------------------------------------------------------------------------------
// freedoomdialog.h
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
#ifndef idf158b01c_04e9_4734_8b12_f66ddbea5cbc
#define idf158b01c_04e9_4734_8b12_f66ddbea5cbc

#include "dptr.h"
#include <QDialog>
#include <QString>

class ModFile;
class ModSet;

class FreedoomDialog : public QDialog
{
	Q_OBJECT

public:
	FreedoomDialog(QWidget *parent);
	~FreedoomDialog() override;

public slots:
	void accept() override;

private:
	enum Column
	{
		ColName,
		ColStatus,
		ColInstall
	};

	DPtr<FreedoomDialog> d;

	void insertModFile(const ModFile &file);
	void resetProgressBar();
	ModSet selectedModFiles() const;
	void setupInstallPaths();
	void setupWadsTable();
	void showError(const QString &text);
	void showModInfo(const ModSet &modSet);
	void showStatus(const QString &text);
	void updateConfig();

private slots:
	void applyFreedoomVersionInfo();
	void fetchInfo();
	void onModInstallFinished();
	void showFileDownloadProgress(const QString &file, qint64 current, qint64 total);
};

#endif
