//------------------------------------------------------------------------------
// cfgwadalias.cpp
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
#include "cfgwadalias.h"
#include "ui_cfgwadalias.h"

#include "configuration/doomseekerconfig.h"
#include "gui/commongui.h"
#include "pathfinder/filealias.h"
#include <QTimer>

DClass<CFGWadAlias> : public Ui::CFGWadAlias
{
public:
	enum Columns
	{
		ColWad,
		ColAliases,
		ColMatchType
	};

	QString matchTypeHelp;
	QTimer resizeTimer;
};

DPointeredNoCopy(CFGWadAlias)

CFGWadAlias::CFGWadAlias(QWidget *parent)
	: ConfigPage(parent)
{
	d->setupUi(this);

	d->resizeTimer.setSingleShot(true);
	d->resizeTimer.setInterval(0);
	connect(&d->resizeTimer, SIGNAL(timeout()), d->table, SLOT(resizeRowsToContents()));

	QHeaderView *header = d->table->horizontalHeader();
	connect(header, SIGNAL(sectionResized(int,int,int)), &d->resizeTimer, SLOT(start()));

	header->resizeSection(PrivData<CFGWadAlias>::ColWad, 150);
	header->setSectionResizeMode(PrivData<CFGWadAlias>::ColAliases, QHeaderView::Stretch);
	header->resizeSection(PrivData<CFGWadAlias>::ColMatchType, 110);

	d->table->sortByColumn(PrivData<CFGWadAlias>::ColWad, Qt::AscendingOrder);

	d->matchTypeHelp = CFGWadAlias::tr("Left-to-Right will use the alias files instead of "
		"the main file but not vice-versa.");
	d->matchTypeHelp += "\n\n";
	d->matchTypeHelp += CFGWadAlias::tr("All Equal will treat all files as equal and try to "
		"match them in any combination.");
}

CFGWadAlias::~CFGWadAlias()
{
}

void CFGWadAlias::addAliasToTable(const FileAlias &alias)
{
	bool wasSortingEnabled = d->table->isSortingEnabled();
	d->table->setSortingEnabled(false);

	int row = findRowWithWad(alias.name());
	if (row < 0)
	{
		row = d->table->rowCount();
		addNewEntry();
		applyAliasToRow(row, alias);
	}
	else
	{
		// Merge.
		FileAlias existingAlias = aliasFromRow(row);
		existingAlias.addAliases(alias.aliases());
		applyAliasToRow(row, existingAlias);
	}

	d->table->setSortingEnabled(wasSortingEnabled);
}

void CFGWadAlias::addDefaults()
{
	QList<FileAlias> aliases = FileAlias::standardWadAliases();
	for (const FileAlias &alias : aliases)
	{
		addAliasToTable(alias);
	}
}

void CFGWadAlias::addNewEntry()
{
	bool wasSortingEnabled = d->table->isSortingEnabled();
	d->table->setSortingEnabled(false);

	int row = d->table->rowCount();
	d->table->insertRow(row);
	d->table->setItem(row, PrivData<CFGWadAlias>::ColWad, new QTableWidgetItem());
	d->table->setItem(row, PrivData<CFGWadAlias>::ColAliases, new QTableWidgetItem());
	mkMatchTypeComboBox(row);

	d->table->setSortingEnabled(wasSortingEnabled);
	resizeRowToContents(row);
}

QList<FileAlias> CFGWadAlias::aliases() const
{
	QList<FileAlias> aliases;
	for (int row = 0; row < d->table->rowCount(); ++row)
	{
		FileAlias alias = aliasFromRow(row);
		if (alias.isValid())
			aliases << alias;
	}
	return FileAliasList::mergeDuplicates(aliases);
}

FileAlias CFGWadAlias::aliasFromRow(int row) const
{
	FileAlias alias;
	alias.setName(d->table->item(row, PrivData<CFGWadAlias>::ColWad)->text().trimmed());
	QStringList candidateAliases = d->table->item(row, PrivData<CFGWadAlias>::ColAliases)->text().split(";");
	for (const QString &candidateAlias : candidateAliases)
	{
		if (!candidateAlias.trimmed().isEmpty())
			alias.addAlias(candidateAlias.trimmed());
	}
	QComboBox *cboMatchType = qobject_cast<QComboBox *>(d->table->cellWidget(
		row, PrivData<CFGWadAlias>::ColMatchType));
	alias.setMatchType(static_cast<FileAlias::MatchType>(
		cboMatchType->itemData(cboMatchType->currentIndex()).toInt()));
	return alias;
}

void CFGWadAlias::applyAliasToRow(int row, const FileAlias &alias)
{
	d->table->setItem(row, PrivData<CFGWadAlias>::ColWad, toolTipItem(alias.name()));
	d->table->setItem(row, PrivData<CFGWadAlias>::ColAliases, toolTipItem(alias.aliases().join("; ")));

	QComboBox *cboMatchType = qobject_cast<QComboBox *>(
		d->table->cellWidget(row, PrivData<CFGWadAlias>::ColMatchType));
	int matchTypeIdx = qMax(0, cboMatchType->findData(alias.matchType()));
	cboMatchType->setCurrentIndex(matchTypeIdx);
}

int CFGWadAlias::findRowWithWad(const QString &wadName)
{
	for (int row = 0; row < d->table->rowCount(); ++row)
	{
		if (d->table->item(row, PrivData<CFGWadAlias>::ColWad)->text().trimmed().compare(
			wadName.trimmed(), Qt::CaseInsensitive) == 0)
		{
			return row;
		}
	}
	return -1;
}

QIcon CFGWadAlias::icon() const
{
	return QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon);
}

QComboBox *CFGWadAlias::mkMatchTypeComboBox(int row)
{
	QComboBox *cboMatchType = new QComboBox();
	cboMatchType->setToolTip(d->matchTypeHelp);
	cboMatchType->addItem(tr("Left-to-Right"), FileAlias::LeftToRight);
	cboMatchType->addItem(tr("All Equal"), FileAlias::AllEqual);
	d->table->setCellWidget(row, PrivData<CFGWadAlias>::ColMatchType, cboMatchType);
	return cboMatchType;
}

void CFGWadAlias::readSettings()
{
	bool wasSortingEnabled = d->table->isSortingEnabled();
	d->table->setSortingEnabled(false);

	while (d->table->rowCount() > 0)
	{
		d->table->removeRow(0);
	}
	// Aliases from configuration are guaranteed to be unique.
	QList<FileAlias> aliases = gConfig.doomseeker.wadAliases();
	for (const FileAlias &alias : aliases)
	{
		if (alias.isValid())
			addAliasToTable(alias);
	}

	d->table->setSortingEnabled(wasSortingEnabled);
}

void CFGWadAlias::removeSelected()
{
	CommonGUI::removeSelectedRowsFromQTableWidget(d->table);
}

void CFGWadAlias::resizeRowsToContents()
{
	#ifdef Q_OS_WIN32
	d->resizeTimer.start();
	#else
	d->table->resizeRowsToContents();
	#endif
}

void CFGWadAlias::resizeRowToContents(int row)
{
	#ifdef Q_OS_WIN32
	resizeRowsToContents(); // resizeRowToContents is not friendly on Windows.
	#else
	d->table->resizeRowToContents(row);
	#endif
}

void CFGWadAlias::saveSettings()
{
	gConfig.doomseeker.setWadAliases(aliases());
}

void CFGWadAlias::showEvent(QShowEvent *event)
{
	resizeRowsToContents();
}

QTableWidgetItem *CFGWadAlias::toolTipItem(const QString &contents)
{
	QTableWidgetItem *item = new QTableWidgetItem(contents);
	item->setToolTip(contents);
	return item;
}
