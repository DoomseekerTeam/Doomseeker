//------------------------------------------------------------------------------
// ircdock.cpp
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
#include "gui/irc/ircdocktabcontents.h"
#include "gui/irc/ircnetworkselectionbox.h"
#include "gui/irc/ircsounds.h"
#include "irc/configuration/chatnetworkscfg.h"
#include "irc/configuration/ircconfig.h"
#include "irc/ircnetworkadapter.h"
#include "ircdock.h"
#include "ui_ircdock.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QTabBar>
#include <QToolBar>

DClass<IRCDock> : public Ui::IRCDock
{
public:
	IRCSounds pSounds;
	QAction *toolBarConnect;
	QAction *toolBarOpenChatWindow;
};

DPointered(IRCDock)

IRCDock::IRCDock(QWidget *parent)
	: QDockWidget(parent)
{
	d->setupUi(this);
	toggleViewAction()->setIcon(QIcon(":/icons/chat.png"));

	d->pSounds.loadFromConfig();

	setupToolbar();

	connect(d->tabWidget, SIGNAL(currentChanged(int)),
		SLOT(tabCurrentChanged(int)));

	connect(d->tabWidget, SIGNAL(tabCloseRequested(int)),
		SLOT(tabCloseRequestedSlot(int)));
}

IRCDock::~IRCDock()
{
}

IRCDockTabContents *IRCDock::addIRCAdapter(IRCAdapterBase *pIRCAdapter)
{
	auto pNewAdapterWidget = new IRCDockTabContents(this);

	connect(pNewAdapterWidget, SIGNAL(chatWindowCloseRequest(IRCDockTabContents*)), SLOT(chatWindowCloseRequestSlot(IRCDockTabContents*)));
	connect(pNewAdapterWidget, SIGNAL(focusRequest(IRCDockTabContents*)), SLOT(tabFocusRequest(IRCDockTabContents*)));
	connect(pNewAdapterWidget, SIGNAL(titleChange(IRCDockTabContents*)), SLOT(titleChange(IRCDockTabContents*)));
	connect(pNewAdapterWidget, SIGNAL(newMessagePrinted()),
		SLOT(titleChangeWithColorOfSenderIfNotFocused()));
	connect(pNewAdapterWidget, SIGNAL(titleBlinkRequested()),
		SLOT(titleChangeWithColorOfSenderIfNotFocused()));

	pNewAdapterWidget->setIRCAdapter(pIRCAdapter);
	d->tabWidget->addTab(pNewAdapterWidget, pNewAdapterWidget->icon(), pNewAdapterWidget->title());
	this->titleChange(pNewAdapterWidget);

	return pNewAdapterWidget;
}

void IRCDock::applyAppearanceSettings()
{
	for (int i = 0; i < d->tabWidget->count(); ++i)
	{
		auto pWidget = (IRCDockTabContents *)d->tabWidget->widget(i);
		pWidget->applyAppearanceSettings();
	}
}

void IRCDock::chatWindowCloseRequestSlot(IRCDockTabContents *pCaller)
{
	int tabIndex = d->tabWidget->indexOf(pCaller);
	if (tabIndex >= 0)
		tabCloseRequestedSlot(tabIndex);
}

void IRCDock::connectToNewNetwork(const IRCNetworkConnectionInfo &connectionInfo, bool bFocusOnNewTab)
{
	IRCNetworkAdapter *pIRCNetworkAdapter = new IRCNetworkAdapter(connectionInfo);

	// Switch this to true only for debug.
	pIRCNetworkAdapter->setEmitAllIRCMessagesEnabled(false);

	// Setup the UI tab for the new network.
	IRCDockTabContents *pTab = addIRCAdapter(pIRCNetworkAdapter);

	// Connect to the network.
	pIRCNetworkAdapter->connect();

	if (bFocusOnNewTab)
		tabFocusRequest(pTab);
}

bool IRCDock::hasTabFocus(const IRCDockTabContents *pTab) const
{
	return this->d->tabWidget->currentWidget() == pTab;
}

IRCNetworkAdapter *IRCDock::networkWithUiFocus()
{
	auto pWidget = (IRCDockTabContents *)d->tabWidget->currentWidget();
	if (pWidget == nullptr)
		return nullptr;

	return pWidget->ircAdapter()->network();
}

void IRCDock::performNetworkAutojoins()
{
	IRCNetworkConnectionInfo connectionInfo;
	connectionInfo.alternateNick = gIRCConfig.personal.alternativeNickname;
	connectionInfo.nick = gIRCConfig.personal.nickname;
	connectionInfo.realName = gIRCConfig.personal.fullName;
	connectionInfo.userName = gIRCConfig.personal.userName;

	QList<IRCNetworkEntity> autojoinNetworks = ChatNetworksCfg().autoJoinNetworks();
	for (const IRCNetworkEntity &network : autojoinNetworks)
	{
		connectionInfo.networkEntity = network;

		connectToNewNetwork(connectionInfo.autoFilled(), false);
	}
}

QString IRCDock::prefixMessage(IRCAdapterBase *pTargetChatWindow, IRCAdapterBase *pMessageSender, const QString &message)
{
	if (pMessageSender != nullptr)
	{
		IRCNetworkAdapter *pTargetNetwork = pTargetChatWindow->network();
		if (pTargetNetwork !=  pMessageSender)
			return QString("%1: %2").arg(pMessageSender->title(), message);
	}

	return message;
}

void IRCDock::setupToolbar()
{
	auto pToolBar = new QToolBar(this);
	pToolBar->setMovable(false);
	pToolBar->setOrientation(Qt::Vertical);

	d->toolBarConnect = new QAction(QIcon(":/icons/network-connect-3.png"), tr("Connect"), pToolBar);
	d->toolBarOpenChatWindow = new QAction(QIcon(":/icons/irc_channel.png"), tr("Open chat window"), pToolBar);

	pToolBar->addAction(d->toolBarConnect);
	pToolBar->addAction(d->toolBarOpenChatWindow);

	d->horizontalLayout->insertWidget(0, pToolBar);
	connect(pToolBar, SIGNAL(actionTriggered(QAction*)), this, SLOT(toolBarAction(QAction*)));
}

IRCSounds &IRCDock::sounds()
{
	return d->pSounds;
}

void IRCDock::tabCloseRequestedSlot(int index)
{
	QWidget *pPageWidget = d->tabWidget->widget(index);
	d->tabWidget->removeTab(index);

	delete pPageWidget;
}

void IRCDock::tabCurrentChanged(int index)
{
	if (index >= 0)
	{
		d->tabWidget->tabBarPublic()->setTabTextColor(index, "");
		auto pTab = (IRCDockTabContents *) d->tabWidget->widget(index);
		pTab->grabFocus();
	}
}

void IRCDock::tabFocusRequest(IRCDockTabContents *pCaller)
{
	d->tabWidget->setCurrentWidget(pCaller);
}

IRCDockTabContents *IRCDock::tabWithFocus()
{
	if (d->tabWidget->currentWidget() != nullptr)
		return static_cast<IRCDockTabContents *>(d->tabWidget->currentWidget());
	return nullptr;
}

void IRCDock::titleChange(IRCDockTabContents *caller)
{
	int tabIndex = d->tabWidget->indexOf(caller);
	if (tabIndex >= 0)
		d->tabWidget->setTabText(tabIndex, caller->title());
}

void IRCDock::titleChangeWithColorOfSenderIfNotFocused()
{
	auto caller = static_cast<IRCDockTabContents *>(sender());
	int tabIndex = d->tabWidget->indexOf(caller);
	if (tabIndex >= 0)
	{
		d->tabWidget->setTabText(tabIndex, caller->title());
		d->tabWidget->tabBarPublic()->setTabTextColor(tabIndex, caller->titleColor());
	}
}

void IRCDock::toolBarAction(QAction *pAction)
{
	if (pAction == d->toolBarConnect)
	{
		IRCNetworkSelectionBox networkSelection(this);
		if (networkSelection.exec() == QDialog::Accepted)
		{
			IRCNetworkConnectionInfo connectionInfo = networkSelection.networkConnectionInfo();
			ChatNetworksCfg().setLastUsedNetwork(connectionInfo.networkEntity);

			// We will attempt to remember user credentials for further use.
			gIRCConfig.personal.alternativeNickname = connectionInfo.alternateNick;
			gIRCConfig.personal.nickname = connectionInfo.nick;
			gIRCConfig.personal.fullName = connectionInfo.realName;
			gIRCConfig.personal.userName = connectionInfo.userName;

			connectToNewNetwork(connectionInfo.autoFilled(), true);
		}
	}
	else if (pAction == d->toolBarOpenChatWindow)
	{
		IRCNetworkAdapter *pNetwork = networkWithUiFocus();
		if (pNetwork == nullptr)
			QMessageBox::warning(nullptr, tr("Doomseeker IRC - Open chat window"), tr("Cannot obtain network connection adapter."));
		else if (!pNetwork->isConnected())
			QMessageBox::warning(nullptr, tr("Doomseeker IRC - Open chat window"), tr("You are not connected to this network."));
		else
		{
			QString recipientName = QInputDialog::getText(nullptr, tr("Open chat window"), tr("Specify a channel or user name:"));
			if (!recipientName.isEmpty())
				pNetwork->openNewAdapter(recipientName);
		}
	}
}
