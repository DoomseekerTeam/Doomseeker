//------------------------------------------------------------------------------
// ircignoresmanager.cpp
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
#include "ircignoresmanager.h"
#include "ui_ircignoresmanager.h"

#include "gui/commongui.h"
#include "irc/configuration/chatnetworkscfg.h"
#include "irc/entities/ircnetworkentity.h"
#include "patternlist.h"
#include <QKeyEvent>

DClass<IRCIgnoresManager> : public Ui::IRCIgnoresManager
{
public:
	QString networkDescription;
};

DPointered(IRCIgnoresManager)


IRCIgnoresManager::IRCIgnoresManager(QWidget *parent, const QString &networkDescription)
	: QDialog(parent)
{
	d->networkDescription = networkDescription;
	d->setupUi(this);
	CommonGUI::setupDialog(*this);

	loadItems();
}

IRCIgnoresManager::~IRCIgnoresManager()
{
}

void IRCIgnoresManager::done(int result)
{
	if (result == Accepted)
		saveItems();
	QDialog::done(result);
}

void IRCIgnoresManager::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Delete)
		deleteSelected();
	QDialog::keyPressEvent(event);
}

void IRCIgnoresManager::loadItems()
{
	ChatNetworksCfg cfg;
	IRCNetworkEntity network = cfg.network(d->networkDescription);
	for (const auto &pattern : network.ignoredUsers())
	{
		QListWidgetItem *item = new QListWidgetItem(pattern.userPattern(), d->list);
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		d->list->addItem(item);
	}
}

void IRCIgnoresManager::saveItems()
{
	ChatNetworksCfg cfg;
	IRCNetworkEntity network = cfg.network(d->networkDescription);
	network.setIgnoredUsers(patterns());
	cfg.replaceNetwork(network.description(), network, this);
}

PatternList IRCIgnoresManager::patterns() const
{
	PatternList result;
	for (int row = 0; row < d->list->count(); ++row)
	{
		QString text = d->list->item(row)->text();
		if (!text.trimmed().isEmpty())
			result << Pattern(text, QRegularExpression::CaseInsensitiveOption, Pattern::Wildcard);
	}
	return result;
}

void IRCIgnoresManager::deleteSelected()
{
	QList<QListWidgetItem *> selection = d->list->selectedItems();
	for (QListWidgetItem *item : selection)
	{
		d->list->removeItemWidget(item);
	}
	qDeleteAll(selection);
}
