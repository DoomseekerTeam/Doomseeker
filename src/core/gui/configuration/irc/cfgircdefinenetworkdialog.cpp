//------------------------------------------------------------------------------
// cfgircdefinenetworkdialog.cpp
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
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "cfgircdefinenetworkdialog.h"
#include "ui_cfgircdefinenetworkdialog.h"

#include "irc/chatnetworknamer.h"
#include "irc/configuration/chatnetworkscfg.h"
#include "irc/entities/ircnetworkentity.h"
#include <cassert>
#include <QMessageBox>


DClass<CFGIRCDefineNetworkDialog> : public Ui::CFGIRCDefineNetworkDialog
{
public:
	static const int MAX_IRC_COMMAND_LENGTH = 512;

	QList<IRCNetworkEntity> existingNetworks;
	QString originalDescription;
};

DPointered(CFGIRCDefineNetworkDialog)


CFGIRCDefineNetworkDialog::CFGIRCDefineNetworkDialog(const IRCNetworkEntity &initValuesEntity, QWidget *parent)
	: QDialog(parent)
{
	construct();

	initFrom(initValuesEntity);
}

CFGIRCDefineNetworkDialog::CFGIRCDefineNetworkDialog(QWidget *parent)
	: QDialog(parent)
{
	construct();
}

CFGIRCDefineNetworkDialog::~CFGIRCDefineNetworkDialog()
{
}

void CFGIRCDefineNetworkDialog::accept()
{
	if (!validateDescription())
		return;
	QStringList offenders = validateAutojoinCommands();
	if (!offenders.isEmpty())
	{
		if (!askToAcceptAnywayWhenCommandsBad(offenders))
			return;
	}
	QDialog::accept();
}

bool CFGIRCDefineNetworkDialog::askToAcceptAnywayWhenCommandsBad(const QStringList &offenders)
{
	assert(!offenders.isEmpty() && "no offenders");
	QString header = tr("Following commands have violated the IRC maximum byte "
		"number limit (%1):\n\n").arg(PrivData<CFGIRCDefineNetworkDialog>::MAX_IRC_COMMAND_LENGTH);
	QString footer = tr("\n\nIf saved, the script may not run properly.\n\n"
		"Do you wish to save the script anyway?");
	QStringList formattedOffenders = formatOffenders(offenders.mid(0, 10));
	QString body = formattedOffenders.join("\n\n");
	if (formattedOffenders.size() < offenders.size())
		body += tr("\n\n... and %n more ...", "", offenders.size() - formattedOffenders.size());
	QString msg = header + body + footer;
	QMessageBox::StandardButton result = QMessageBox::warning(
		this, tr("Doomseeker - IRC Commands Problem"), msg,
		QMessageBox::Yes | QMessageBox::Cancel);
	return result == QMessageBox::Yes;
}

QStringList CFGIRCDefineNetworkDialog::autojoinCommands() const
{
	return d->teAutojoinCommands->toPlainText().split("\n");
}

void CFGIRCDefineNetworkDialog::buttonClicked(QAbstractButton *button)
{
	auto pButton = (QPushButton *)button;
	if (pButton == d->buttonBox->button(QDialogButtonBox::Ok))
		this->accept();
	else
		this->reject();
}

void CFGIRCDefineNetworkDialog::construct()
{
	d->setupUi(this);

	connect(d->buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(buttonClicked(QAbstractButton*)));
}

QStringList CFGIRCDefineNetworkDialog::formatOffenders(const QStringList &offenders) const
{
	QStringList offendersFormatted;
	for (const QString &offender : offenders)
	{
		offendersFormatted << tr("\t%1 (...)").arg(offender.left(40));
	}
	return offendersFormatted;
}

IRCNetworkEntity CFGIRCDefineNetworkDialog::getNetworkEntity() const
{
	IRCNetworkEntity entity;

	QString autojoinChannels = d->teAutojoinChannels->toPlainText();
	autojoinChannels.remove('\r').replace('\n', ' ');

	entity.setAddress(d->leAddress->text().trimmed());
	entity.setAutojoinChannels(autojoinChannels.split(" ", QString::SkipEmptyParts));
	entity.setAutojoinCommands(autojoinCommands());
	entity.setDescription(d->leDescription->text().trimmed());
	entity.setNickservCommand(d->leNickservCommand->text().trimmed());
	entity.setNickservPassword(d->leNickservPassword->text());
	entity.setPassword(d->leServerPassword->text());
	entity.setPort(d->spinPort->value());

	return entity;
}

void CFGIRCDefineNetworkDialog::initFrom(const IRCNetworkEntity &networkEntity)
{
	d->originalDescription = networkEntity.description();
	d->leAddress->setText(networkEntity.address());
	d->teAutojoinChannels->setPlainText(networkEntity.autojoinChannels().join(" "));
	d->teAutojoinCommands->setPlainText(networkEntity.autojoinCommands().join("\n"));
	d->leDescription->setText(networkEntity.description());
	d->leNickservCommand->setText(networkEntity.nickservCommand());
	d->leNickservPassword->setText(networkEntity.nickservPassword());
	d->leServerPassword->setText(networkEntity.password());
	d->spinPort->setValue(networkEntity.port());
}

bool CFGIRCDefineNetworkDialog::isDescriptionUnique() const
{
	QString current = d->leDescription->text().trimmed().toLower();
	if (d->originalDescription.trimmed().toLower() == current)
	{
		// Network is being edited and its name hasn't been changed.
		return true;
	}
	for (const IRCNetworkEntity &network : listExistingNetworks())
	{
		if (network.description().trimmed().toLower() == current)
			return false;
	}
	return true;
}

bool CFGIRCDefineNetworkDialog::isValidDescription() const
{
	return ChatNetworkNamer::isValidName(d->leDescription->text());
}

QList<IRCNetworkEntity> CFGIRCDefineNetworkDialog::listExistingNetworks() const
{
	if (!d->existingNetworks.isEmpty())
		return d->existingNetworks;
	else
		return ChatNetworksCfg().networks();
}

QStringList CFGIRCDefineNetworkDialog::validateAutojoinCommands() const
{
	QStringList offenders;
	for (const QString &command : autojoinCommands())
	{
		if (command.toUtf8().length() > PrivData<CFGIRCDefineNetworkDialog>::MAX_IRC_COMMAND_LENGTH)
			offenders << command;
	}
	return offenders;
}

void CFGIRCDefineNetworkDialog::setExistingNetworks(const QList<IRCNetworkEntity> &networks)
{
	d->existingNetworks = networks;
}

bool CFGIRCDefineNetworkDialog::validateDescription()
{
	if (d->leDescription->text().trimmed().isEmpty())
	{
		QMessageBox::critical(this, tr("Invalid IRC network description"),
			tr("Network description cannot be empty."));
		return false;
	}
	if (!isDescriptionUnique())
	{
		QMessageBox::critical(this, tr("Invalid IRC network description"),
			tr("There already is a network with such description."));
		return false;
	}
	if (!isValidDescription())
	{
		QString msg = tr("Network description is invalid.\n\n"
			"Only letters, digits, spaces and \"%1\" are allowed.")
				.arg(ChatNetworkNamer::additionalAllowedChars());
		QMessageBox::critical(this, tr("Invalid IRC network description"), msg);
		return false;
	}
	return true;
}
