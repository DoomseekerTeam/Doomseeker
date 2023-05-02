//------------------------------------------------------------------------------
// dockBuddiesList.cpp
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
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------
#include "configuration/doomseekerconfig.h"
#include "dockBuddiesList.h"
#include "patternlist.h"
#include "serverapi/mastermanager.h"
#include "serverapi/playerslist.h"
#include "serverapi/server.h"
#include "strings.hpp"
#include "ui_addBuddyDlg.h"
#include "ui_dockBuddiesList.h"
#include <QMenu>
#include <QMessageBox>
#include <QRegExp>
#include <QStandardItemModel>

class BuddyLocationInfo
{
public:
	BuddyLocationInfo(const Player &buddy, ServerPtr location);
	~BuddyLocationInfo();

	const Player &buddy() const;
	ServerPtr location() const;

	bool operator==(const BuddyLocationInfo &other) const;

private:
	DPtr<BuddyLocationInfo> d;
};

// [BL] This was moved earlier in the file in order to prevent double specialization
// with some compilers
DClass<BuddyLocationInfo>
{
public:
	Player buddy;
	ServerPtr server;
};

DPointered(BuddyLocationInfo)

DClass<DockBuddiesList> : public Ui::DockBuddiesList
{
public:
	QList<BuddyLocationInfo> buddies;
	QStandardItemModel *buddiesTableModel;
	PatternList patterns;
};

DPointered(DockBuddiesList)

DockBuddiesList::DockBuddiesList(QWidget *parent)
	: QDockWidget(parent), masterClient(nullptr), save(false)
{
	d->setupUi(this);
	this->toggleViewAction()->setIcon(QIcon(":/icons/buddies.png"));

	// Set up the model
	d->buddiesTableModel = new QStandardItemModel();

	// Not working yet.
	QStringList columns;
	columns << DockBuddiesList::tr("ID");
	columns << DockBuddiesList::tr("Buddy");
	columns << DockBuddiesList::tr("Location");
	d->buddiesTableModel->setHorizontalHeaderLabels(columns);

	d->buddiesTable->setModel(d->buddiesTableModel);
	d->buddiesTable->setColumnHidden(0, true); // Hide the ID

	// Read config
	d->patterns = gConfig.doomseeker.buddies;

	for (const QRegExp &pattern : d->patterns)
	{
		d->patternsList->addItem(pattern.pattern());
	}

	connect(d->addButton, SIGNAL(clicked()), this, SLOT(addBuddy()));
	connect(d->buddiesTable, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(followBuddy(const QModelIndex&)));
	connect(d->deleteButton, SIGNAL(clicked()), this, SLOT(deleteBuddy()));
	connect(d->patternsList, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(patternsListContextMenu(const QPoint&)));
}

DockBuddiesList::~DockBuddiesList()
{
	// See if we made any modification since modifying the setting will cause a
	// write cycle.
	if (save)
		gConfig.doomseeker.buddies = d->patterns;
}

void DockBuddiesList::addBuddy()
{
	AddBuddyDlg dlg;
	int result = dlg.exec();
	if (result != QDialog::Accepted)
		return;

	QRegExp pattern = dlg.pattern();
	d->patterns << pattern;
	d->patternsList->addItem(pattern.pattern());

	scan(masterClient);

	save = true;
}

void DockBuddiesList::deleteBuddy()
{
	// Do nothing if nothing is selected.
	if (d->patternsList->selectionModel()->selectedIndexes().size() <= 0)
		return;

	if (QMessageBox::warning(this, tr("Delete Buddy"), tr("Are you sure you want to delete this pattern?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
		!= QMessageBox::Yes)
		return;

	QModelIndexList selection = d->patternsList->selectionModel()->selectedIndexes();
	// We have to remove the bottom most row until they are all gone.
	while (selection.size() > 0)
	{
		int largestIndex = 0;
		for (int i = 1; i < selection.size(); i++)
		{
			if (selection[i].row() > selection[largestIndex].row())
				largestIndex = i;
		}

		d->patterns.removeAt(selection[largestIndex].row());
		delete d->patternsList->item(selection[largestIndex].row());
		selection.removeAt(largestIndex); // remove index
	}

	scan(masterClient);

	save = true;
}

void DockBuddiesList::followBuddy(const QModelIndex &index)
{
	// Folow the buddy into the server.
	QString error;

	ServerPtr server = d->buddies[d->buddiesTableModel->item(index.row(), BLCID_ID)->data().toInt()].location();
	emit joinServer(server);
}

bool DockBuddiesList::hasBuddy(const ServerPtr &server)
{
	for (const BuddyLocationInfo &location : d->buddies)
	{
		if (location.location() == server)
			return true;
	}
	return false;
}

void DockBuddiesList::patternsListContextMenu(const QPoint &pos) const
{
	// First lets get the selection since that will determine the menu.
	QModelIndexList selection = d->patternsList->selectionModel()->selectedIndexes();

	QMenu context;

	// Always have the add function.
	QAction *addAction = context.addAction(tr("Add"));
	connect(addAction, SIGNAL(triggered()), this, SLOT(addBuddy()));

	// if anything is selected allow the user to delete them.
	if (selection.size() > 0)
	{
		QAction *deleteAction = context.addAction(tr("Delete"));
		connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteBuddy()));
	}

	// Finally show our menu.
	context.exec(d->patternsList->mapToGlobal(pos));
}

void DockBuddiesList::scan(const MasterManager *master)
{
	if (master == nullptr && masterClient == nullptr)
		return;
	else if (master != masterClient && master != nullptr) // If the master is new
		masterClient = master;

	d->buddies.clear(); //empty list
	for (ServerPtr server : masterClient->allServers())
	{
		if (!server->isKnown())
			continue;
		for (int i = 0; i < server->players().numClients(); ++i)
		{
			const Player player = server->player(i);
			if (d->patterns.isExactMatchAny(player.nameColorTagsStripped()))
			{
				BuddyLocationInfo candidate(player, server);
				if (!d->buddies.contains(candidate))
					d->buddies << BuddyLocationInfo(player, server);
			}
		}
	}

	// Populate list
	d->buddiesTableModel->removeRows(0, d->buddiesTableModel->rowCount());
	for (int i = 0; i < d->buddies.size(); i++)
	{
		const BuddyLocationInfo &info = d->buddies[i];
		QList<QStandardItem *> columns;

		for (int j = 0; j < HOW_MANY_BUDDIESLIST_COLUMNS; j++)
		{
			switch (j)
			{
			case BLCID_ID:
			{
				auto item = new QStandardItem();
				item->setData(i);
				columns << item;
				break;
			}
			case BLCID_BUDDY:
				columns << new QStandardItem(info.buddy().nameColorTagsStripped());
				break;
			case BLCID_LOCATION:
				columns << new QStandardItem(info.location()->name());
				break;
			default:
				columns << new QStandardItem();
				break;
			}
		}

		d->buddiesTableModel->appendRow(columns);
	}

	// Fits rows to contents
	d->buddiesTable->resizeRowsToContents();
	emit scanCompleted();
}

////////////////////////////////////////////////////////////////////////////////

DClass<AddBuddyDlg> : public Ui::AddBuddyDlg
{
};

DPointered(AddBuddyDlg)

AddBuddyDlg::AddBuddyDlg(QWidget *parent) : QDialog(parent)
{
	d->setupUi(this);

	connect(d->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonBoxClicked(QAbstractButton*)));
}

AddBuddyDlg::~AddBuddyDlg()
{
}

void AddBuddyDlg::buttonBoxClicked(QAbstractButton *button)
{
	if (d->buttonBox->standardButton(button) == QDialogButtonBox::Ok)
	{
		if (!pattern().isValid())
		{
			QMessageBox::information(this, tr("Invalid Pattern"), tr("The pattern you have specified is invalid."));
			return;
		}
		accept();
	}
	else
		reject();
}

QRegExp AddBuddyDlg::pattern() const
{
	QRegExp::PatternSyntax syntax = d->basicPattern->isChecked() ? QRegExp::Wildcard : QRegExp::RegExp;
	return QRegExp(d->patternBox->text(), Qt::CaseInsensitive, syntax);
}

////////////////////////////////////////////////////////////////////////////////

BuddyLocationInfo::BuddyLocationInfo(const Player &buddy, ServerPtr location)
{
	d->buddy = buddy;
	d->server = location;
}

BuddyLocationInfo::~BuddyLocationInfo()
{
}

const Player &BuddyLocationInfo::buddy() const
{
	return d->buddy;
}

ServerPtr BuddyLocationInfo::location() const
{
	return d->server;
}

bool BuddyLocationInfo::operator==(const BuddyLocationInfo &other) const
{
	return d->buddy == other.d->buddy
		&& d->server->address() == other.d->server->address()
		&& d->server->port() == other.d->server->port()
		&& d->server->plugin() == other.d->server->plugin();
}
