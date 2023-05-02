//------------------------------------------------------------------------------
// ircdock.h
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
#ifndef __IRCDOCK_H_
#define __IRCDOCK_H_

#include "dptr.h"

#include <QDockWidget>

class IRCAdapterBase;
class IRCDockTabContents;
class IRCNetworkAdapter;
struct IRCNetworkConnectionInfo;
class IRCSounds;

/**
 *	@brief Dockable widget designed for IRC communication.
 */
class IRCDock : public QDockWidget
{
	Q_OBJECT

public:
	IRCDock(QWidget *parent = nullptr);
	~IRCDock() override;

	IRCDockTabContents *addIRCAdapter(IRCAdapterBase *pIRCAdapter);

	/**
	 *	@brief Applies IRC appearance settings to all open tabs.
	 */
	void applyAppearanceSettings();
	bool hasTabFocus(const IRCDockTabContents *pTab) const;

	void performNetworkAutojoins();

	IRCSounds &sounds();
	IRCDockTabContents *tabWithFocus();

private:
	DPtr<IRCDock> d;

	IRCNetworkAdapter *networkWithUiFocus();

	/**
	 * @brief Connects to new network.
	 *
	 * Opens new IRC network tab and starts the connection basing on the
	 * passed IRCNetworkConnectionInfo.
	 *
	 * @param connectionInfo
	 *      Defines connection parameters.
	 * @param bFocusOnNewTab
	 *      If <code>true</code> the new tab will gain focus when opened.
	 */
	void connectToNewNetwork(const IRCNetworkConnectionInfo &connectionInfo, bool bFocusOnNewTab);

	/**
	 *	@brief This will prefix message with network name if message
	 *	comes from a different network.
	 */
	QString prefixMessage(IRCAdapterBase *pTargetChatWindow, IRCAdapterBase *pMessageSender, const QString &message);
	void setupToolbar();

private slots:
	void chatWindowCloseRequestSlot(IRCDockTabContents *pCaller);
	void tabCloseRequestedSlot(int index);
	void tabCurrentChanged(int index);
	void tabFocusRequest(IRCDockTabContents *pCaller);
	void titleChange(IRCDockTabContents *caller);
	void titleChangeWithColorOfSenderIfNotFocused();
	void toolBarAction(QAction *pAction);
};

#endif
