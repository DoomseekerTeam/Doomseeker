//------------------------------------------------------------------------------
// cfgserverpasswords.cpp
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
#include "cfgserverpasswords.h"
#include "ui_cfgserverpasswords.h"

#include "configuration/passwordscfg.h"
#include "configuration/serverpassword.h"
#include "gui/commongui.h"
#include "gui/helpers/datetablewidgetitem.h"
#include <QMap>

const int COL_PASS_PASSWORD = 0;
const int COL_PASS_LAST_GAME = 1;
const int COL_PASS_LAST_SERVER = 2;
const int COL_PASS_LAST_TIME = 3;

const int COL_SERV_GAME = 0;
const int COL_SERV_NAME = 1;
const int COL_SERV_ADDRESS = 2;
const int COL_SERV_LAST_TIME = 3;

const QString HIDDEN_PASS = "***";

DClass<CFGServerPasswords> : public Ui::CFGServerPasswords
{
public:
	bool bHidingPasswordsMode;
};

DPointered(CFGServerPasswords)

CFGServerPasswords::CFGServerPasswords(QWidget *parent)
	: ConfigPage(parent)
{
	d->setupUi(this);
	d->bHidingPasswordsMode = true;
	hidePasswords();
	d->tablePasswords->sortItems(COL_PASS_LAST_TIME, Qt::DescendingOrder);
	d->tablePasswords->setColumnWidth(COL_PASS_PASSWORD, 90);
	d->tableServers->sortItems(COL_SERV_GAME, Qt::AscendingOrder);
	d->tableServers->setColumnWidth(COL_SERV_GAME, 90);
	d->lblServerLossWarning->setVisible(false);
}

CFGServerPasswords::~CFGServerPasswords()
{
}

void CFGServerPasswords::addPasswordFromLineEdit()
{
	QString phrase = d->lePassword->text();
	d->lePassword->clear();
	if (!phrase.isEmpty() && !isPassphraseInTable(phrase))
	{
		ServerPassword password;
		password.setPhrase(phrase);
		addServerPasswordToTable(password);
	}
}

void CFGServerPasswords::addServerPasswordToTable(const ServerPassword &password)
{
	int rowIndex = d->tablePasswords->rowCount();
	d->tablePasswords->insertRow(rowIndex);
	setPasswordInRow(rowIndex, password);
}

void CFGServerPasswords::clearTable(QTableWidget *table)
{
	while (table->rowCount() > 0)
	{
		table->removeRow(0);
	}
}

int CFGServerPasswords::findPassphraseInTable(const QString &phrase)
{
	for (int i = 0; i < d->tablePasswords->rowCount(); ++i)
	{
		ServerPassword password = serverPasswordFromRow(i);
		if (password.phrase() == phrase)
			return i;
	}
	return -1;
}

void CFGServerPasswords::hidePasswords()
{
	d->btnRevealHideToggle->setText(tr("Reveal"));
	d->bHidingPasswordsMode = true;
	d->lePassword->setEchoMode(QLineEdit::Password);
	for (int i = 0; i < d->tablePasswords->rowCount(); ++i)
	{
		QTableWidgetItem *item = d->tablePasswords->item(i, COL_PASS_PASSWORD);
		item->setText(HIDDEN_PASS);
		item->setToolTip(HIDDEN_PASS);
	}
}

bool CFGServerPasswords::isPassphraseInTable(const QString &phrase)
{
	return findPassphraseInTable(phrase) >= 0;
}

void CFGServerPasswords::onPasswordTableCellChange(int currentRow, int currentColumn,
	int previousRow, int previousColumn)
{
	Q_UNUSED(currentColumn);
	Q_UNUSED(previousColumn);
	if (currentRow != previousRow)
	{
		// Setting an invalid password will clear the table which is
		// what we want.
		setServersInTable(serverPasswordFromRow(currentRow));
	}
}

void CFGServerPasswords::saveSettings()
{
	PasswordsCfg cfg;
	QList<ServerPassword> passwords;
	for (int i = 0; i < d->tablePasswords->rowCount(); ++i)
		passwords << serverPasswordFromRow(i);
	cfg.setServerPasswords(passwords);
	cfg.setMaxNumberOfServersPerPassword(d->spinMaxPasswords->value());
}

ServerPassword CFGServerPasswords::serverPasswordFromRow(int row)
{
	if (row < 0 || row >= d->tablePasswords->rowCount())
		return ServerPassword();
	return ServerPassword::deserializeQVariant(
		d->tablePasswords->item(row, COL_PASS_PASSWORD)->data(Qt::UserRole));
}

void CFGServerPasswords::setPasswordInRow(int row, const ServerPassword &password)
{
	// Disable sorting or bad things may happen.
	bool wasSortingEnabled = d->tablePasswords->isSortingEnabled();
	d->tablePasswords->setSortingEnabled(false);

	QString phrase;
	if (d->bHidingPasswordsMode)
		phrase = HIDDEN_PASS;
	else
		phrase = password.phrase();

	QTableWidgetItem *phraseItem = toolTipItem(phrase);
	phraseItem->setData(Qt::UserRole, password.serializeQVariant());
	d->tablePasswords->setItem(row, COL_PASS_PASSWORD, phraseItem);

	d->tablePasswords->setItem(row, COL_PASS_LAST_GAME, toolTipItem(password.lastGame()));
	d->tablePasswords->setItem(row, COL_PASS_LAST_SERVER, toolTipItem(password.lastServerName()));

	DateTableWidgetItem *dateItem = new DateTableWidgetItem(password.lastTime());
	dateItem->setToolTip(dateItem->displayedText());
	d->tablePasswords->setItem(row, COL_PASS_LAST_TIME, dateItem);

	// Re-enable sorting if was enabled before.
	d->tablePasswords->setSortingEnabled(wasSortingEnabled);
	d->tablePasswords->resizeRowsToContents();
}

void CFGServerPasswords::setServersInTable(const ServerPassword &password)
{
	clearTable(d->tableServers);
	// Disable sorting or bad things may happen.
	d->tableServers->setSortingEnabled(false);
	for (const ServerPasswordSummary &server : password.servers())
	{
		int rowIndex = d->tableServers->rowCount();
		d->tableServers->insertRow(rowIndex);

		QTableWidgetItem *gameItem = toolTipItem(server.game());
		gameItem->setData(Qt::UserRole, server.serializeQVariant());
		d->tableServers->setItem(rowIndex, COL_SERV_GAME, gameItem);

		d->tableServers->setItem(rowIndex, COL_SERV_NAME, toolTipItem(server.name()));
		QString address = QString("%1:%2").arg(server.address()).arg(server.port());
		d->tableServers->setItem(rowIndex, COL_SERV_ADDRESS, toolTipItem(address));

		DateTableWidgetItem *dateItem = new DateTableWidgetItem(server.time());
		dateItem->setToolTip(dateItem->displayedText());
		d->tableServers->setItem(rowIndex, COL_SERV_LAST_TIME, dateItem);
	}
	// Re-enable sorting.
	d->tableServers->setSortingEnabled(true);
	d->tableServers->resizeRowsToContents();
}

void CFGServerPasswords::showServerLossWarningIfNecessary()
{
	PasswordsCfg cfg;
	d->lblServerLossWarning->setVisible(d->spinMaxPasswords->value() <
		cfg.maxNumberOfServersPerPassword());
}

void CFGServerPasswords::readSettings()
{
	clearTable(d->tablePasswords);
	clearTable(d->tableServers);

	PasswordsCfg cfg;
	for (const ServerPassword &pass : cfg.serverPasswords())
	{
		addServerPasswordToTable(pass);
	}
	d->spinMaxPasswords->setValue(cfg.maxNumberOfServersPerPassword());
}

void CFGServerPasswords::removeSelectedPasswords()
{
	CommonGUI::removeSelectedRowsFromQTableWidget(d->tablePasswords);
}

void CFGServerPasswords::removeSelectedServers()
{
	QList<ServerPasswordSummary> servers;
	for (QTableWidgetItem *item : d->tableServers->selectedItems())
	{
		if (item->column() == COL_SERV_GAME)
			servers << ServerPasswordSummary::deserializeQVariant(item->data(Qt::UserRole));
	}
	ServerPassword currentPassword = serverPasswordFromRow(d->tablePasswords->currentRow());
	for (const ServerPasswordSummary &server : servers)
	{
		currentPassword.removeServer(server.game(), server.address(), server.port());
	}
	updatePassword(currentPassword);
	CommonGUI::removeSelectedRowsFromQTableWidget(d->tableServers);
}

void CFGServerPasswords::revealPasswords()
{
	d->btnRevealHideToggle->setText(tr("Hide"));
	d->lePassword->setEchoMode(QLineEdit::Normal);
	d->bHidingPasswordsMode = false;
	for (int i = 0; i < d->tablePasswords->rowCount(); ++i)
	{
		QTableWidgetItem *item = d->tablePasswords->item(i, COL_PASS_PASSWORD);
		ServerPassword password = serverPasswordFromRow(i);
		item->setText(password.phrase());
		item->setToolTip(password.phrase());
	}
}

void CFGServerPasswords::toggleRevealHide()
{
	if (d->bHidingPasswordsMode)
		revealPasswords();
	else
		hidePasswords();
}

QTableWidgetItem *CFGServerPasswords::toolTipItem(const QString &contents)
{
	auto item = new QTableWidgetItem(contents);
	item->setToolTip(contents);
	return item;
}

void CFGServerPasswords::updatePassword(const ServerPassword &password)
{
	int row = findPassphraseInTable(password.phrase());
	if (row >= 0)
		setPasswordInRow(row, password);
	else
		addServerPasswordToTable(password);
}
