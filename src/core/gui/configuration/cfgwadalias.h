//------------------------------------------------------------------------------
// cfgwadalias.h
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
#ifndef idBE57B8CD_234F_4EEE_AC1B1FAAFB11B1CD
#define idBE57B8CD_234F_4EEE_AC1B1FAAFB11B1CD

#include "dptr.h"
#include "gui/configuration/configpage.h"
#include <QComboBox>

class FileAlias;
class QTableWidgetItem;

class CFGWadAlias : public ConfigPage
{
	Q_OBJECT

public:
	CFGWadAlias(QWidget *parent = nullptr);
	~CFGWadAlias() override;

	QIcon icon() const override;
	QString name() const override
	{
		return tr("WAD Aliases");
	}

	void readSettings() override;

protected:
	void saveSettings() override;
	void showEvent(QShowEvent *event) override;

private:
	DPtr<CFGWadAlias> d;

	void addAliasToTable(const FileAlias &alias);
	FileAlias aliasFromRow(int row) const;
	void applyAliasToRow(int row, const FileAlias &alias);
	QList<FileAlias> aliases() const;
	int findRowWithWad(const QString &wadName);
	QComboBox *mkMatchTypeComboBox(int row);
	void resizeRowsToContents();
	QTableWidgetItem *toolTipItem(const QString &contents);

private slots:
	void addDefaults();
	void addNewEntry();
	void removeSelected();
	void resizeRowToContents(int row);
};

#endif
