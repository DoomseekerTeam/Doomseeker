//------------------------------------------------------------------------------
// ircnetworkselectionbox.cpp
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
#include "gui/configuration/irc/cfgircdefinenetworkdialog.h"
#include "irc/configuration/chatnetworkscfg.h"
#include "irc/configuration/ircconfig.h"
#include "irc/ircnetworkconnectioninfo.h"
#include "ircnetworkselectionbox.h"
#include "qtmetapointer.h"
#include "ui_ircnetworkselectionbox.h"
#include <QMessageBox>

DClass<IRCNetworkSelectionBox> : public Ui::IRCNetworkSelectionBox
{
};

DPointered(IRCNetworkSelectionBox)

IRCNetworkSelectionBox::IRCNetworkSelectionBox(QWidget *parent)
	: QDialog(parent)
{
	d->setupUi(this);

	connect(d->cboNetwork, SIGNAL(currentIndexChanged(int)), SLOT(networkChanged(int)));

	initWidgets();
}

IRCNetworkSelectionBox::~IRCNetworkSelectionBox()
{
}

void IRCNetworkSelectionBox::accept()
{
	if (validate())
		QDialog::accept();
}

void IRCNetworkSelectionBox::addNetworkToComboBox(const IRCNetworkEntity &network)
{
	d->cboNetwork->addItem(buildTitle(network), network.serializeQVariant());
}

QString IRCNetworkSelectionBox::buildTitle(const IRCNetworkEntity &network) const
{
	return QString("%1 [%2:%3]").arg(network.description()).arg(network.address()).arg(network.port());
}

void IRCNetworkSelectionBox::createNewNetwork()
{
	CFGIRCDefineNetworkDialog dialog(this);
	if (dialog.exec() == QDialog::Accepted)
	{
		ChatNetworksCfg cfg;
		QList<IRCNetworkEntity> networks = cfg.networks();
		networks << dialog.getNetworkEntity();
		cfg.setNetworks(networks);

		fetchNetworks();
	}
}

void IRCNetworkSelectionBox::editCurrentNetwork()
{
	IRCNetworkEntity network = networkCurrent();
	if (!network.isValid())
	{
		QMessageBox::critical(this, tr("Doomseeker - edit IRC network"),
			tr("Cannot edit as no valid network is selected."));
		return;
	}
	CFGIRCDefineNetworkDialog dialog(network, this);
	if (dialog.exec() == QDialog::Accepted)
	{
		IRCNetworkEntity editedNetwork = dialog.getNetworkEntity();
		if (replaceNetworkInConfig(network, editedNetwork))
			updateCurrentNetwork(editedNetwork);
	}
}

void IRCNetworkSelectionBox::fetchNetworks()
{
	ChatNetworksCfg cfg;
	QList<IRCNetworkEntity> networks = cfg.networks();
	std::sort(networks.begin(), networks.end());
	d->cboNetwork->blockSignals(true);
	d->cboNetwork->clear();

	for (const IRCNetworkEntity &network : networks)
	{
		addNetworkToComboBox(network);
	}

	IRCNetworkEntity lastUsedNetwork = cfg.lastUsedNetwork();
	if (lastUsedNetwork.isValid())
		setNetworkMatchingDescriptionAsCurrent(lastUsedNetwork.description());

	updateNetworkInfo();
	d->cboNetwork->blockSignals(false);
}

void IRCNetworkSelectionBox::initWidgets()
{
	d->leAlternateNick->setText(gIRCConfig.personal.alternativeNickname);
	d->leNick->setText(gIRCConfig.personal.nickname);
	d->leRealName->setText(gIRCConfig.personal.fullName);
	d->leUserName->setText(gIRCConfig.personal.userName);

	fetchNetworks();
}

IRCNetworkEntity IRCNetworkSelectionBox::network() const
{
	IRCNetworkEntity networkEntity = networkCurrent();
	networkEntity.setPassword(d->lePassword->text());
	return networkEntity;
}

void IRCNetworkSelectionBox::networkChanged(int index)
{
	if (index >= 0)
		updateNetworkInfo();
}

IRCNetworkEntity IRCNetworkSelectionBox::networkCurrent() const
{
	return networkAtRow(d->cboNetwork->currentIndex());
}

IRCNetworkEntity IRCNetworkSelectionBox::networkAtRow(int row) const
{
	if (row < 0 || row >= d->cboNetwork->count())
		return IRCNetworkEntity();
	return IRCNetworkEntity::deserializeQVariant(d->cboNetwork->itemData(row));
}

IRCNetworkConnectionInfo IRCNetworkSelectionBox::networkConnectionInfo() const
{
	IRCNetworkConnectionInfo outInfo;

	outInfo.alternateNick = d->leAlternateNick->text();
	outInfo.nick = d->leNick->text();
	outInfo.realName = d->leRealName->text();
	outInfo.userName = d->leUserName->text();

	outInfo.networkEntity = this->network();

	return outInfo;
}

void IRCNetworkSelectionBox::setNetworkMatchingDescriptionAsCurrent(const QString &description)
{
	for (int row = 0; row < d->cboNetwork->count(); ++row)
	{
		IRCNetworkEntity candidate = networkAtRow(row);
		if (candidate.description() == description)
		{
			d->cboNetwork->setCurrentIndex(row);
			break;
		}
	}
}

void IRCNetworkSelectionBox::updateCurrentNetwork(const IRCNetworkEntity &network)
{
	d->cboNetwork->setItemText(d->cboNetwork->currentIndex(), buildTitle(network));
	d->cboNetwork->setItemData(d->cboNetwork->currentIndex(), network.serializeQVariant());
	updateNetworkInfo();
}

void IRCNetworkSelectionBox::removeCurrentNetwork()
{
	IRCNetworkEntity network = networkCurrent();
	if (!network.isValid())
	{
		QMessageBox::critical(this, tr("Doomseeker - remove IRC network"),
			tr("Cannot remove as no valid network is selected."));
		return;
	}
	if (QMessageBox::question(this, tr("Doomseeker - remove IRC network"),
		tr("Are you sure you wish to remove network '%1'?").arg(network.description()),
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		ChatNetworksCfg cfg;
		cfg.removeNetwork(network);
		fetchNetworks();
	}
}

bool IRCNetworkSelectionBox::replaceNetworkInConfig(const IRCNetworkEntity &oldNetwork, const IRCNetworkEntity &newNetwork)
{
	ChatNetworksCfg cfg;
	return cfg.replaceNetwork(oldNetwork.description(), newNetwork, this);
}

void IRCNetworkSelectionBox::updateNetworkInfo()
{
	IRCNetworkEntity network = networkCurrent();

	d->leServerAddress->setText(network.address());
	d->spinPort->setValue(network.port());
	d->lePassword->setText(network.password());
}

bool IRCNetworkSelectionBox::validate()
{
	const static QString ERROR_TITLE = tr("IRC connection error");
	IRCNetworkConnectionInfo connectionInfo = this->networkConnectionInfo();

	if (connectionInfo.nick.isEmpty())
	{
		QMessageBox::warning(nullptr, ERROR_TITLE, tr("You must specify a nick."));
		return false;
	}

	if (connectionInfo.networkEntity.address().isEmpty())
	{
		QMessageBox::warning(nullptr, ERROR_TITLE, tr("You must specify a network address."));
		return false;
	}

	return true;
}
