//------------------------------------------------------------------------------
// ircdocktabcontents.h
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
#ifndef __IRCDOCK_TAB_CONTENTS_H_
#define __IRCDOCK_TAB_CONTENTS_H_

#include "dptr.h"

#include <QWidget>

class IRCAdapterBase;
class IRCChatAdapter;
class IRCDock;
class IRCMessageClass;
class IRCNetworkAdapter;
class IRCNetworkEntity;
class IRCNicknameCompleter;
class IRCUserInfo;
class IRCUserList;
class QMenu;
class QModelIndex;
class QStandardItem;

/**
 * @brief Dockable widget designed for IRC communication.
 */
class IRCDockTabContents : public QWidget
{
	Q_OBJECT

public:
	IRCDockTabContents(IRCDock *pParentIRCDock);
	~IRCDockTabContents() override;

	/**
	 * @brief Applies current appearance settings from the IRC config.
	 */
	void applyAppearanceSettings();

	/**
	 * @brief Called when tab becomes active.
	 *
	 * Informs the tab that it should grab keyboard focus.
	 * Text input widget will be selected.
	 */
	void grabFocus();
	bool hasTabFocus() const;

	QIcon icon() const;

	IRCAdapterBase *ircAdapter() const
	{
		return pIrcAdapter;
	}

	/**
	 * @brief Calling this multiple times on the same object will cause
	 * memory leaks.
	 */
	void setIRCAdapter(IRCAdapterBase *pAdapter);

	QString title() const;
	QString titleColor() const;

public slots:
	void receiveMessage(const QString &message);
	void receiveMessageWithClass(const QString &message, const IRCMessageClass &messageClass);

signals:
	/**
	 * @brief Emitted when the IRCAdapterBase that is associated
	 * with this widget is no longer valid - possibly even deleted.
	 *
	 * Capture this to close this widget.
	 */
	void chatWindowCloseRequest(IRCDockTabContents *);

	void titleBlinkRequested();
	/**
	 * @brief Emitted when the variable returned by
	 * IRCAdapterBase::title() might have changed and the
	 * application should be notified of this fact.
	 */
	void titleChange(IRCDockTabContents *pCaller);

	/**
	 * @brief Emitted when network adapter for this dock emits
	 * its focusRequest() signal.
	 */
	void focusRequest(IRCDockTabContents *pCaller);

	void newMessagePrinted();

protected slots:
	void adapterFocusRequest();
	void adapterTerminating();

	void adapterTitleChange()
	{
		emit titleChange(this);
	}

	void nameAdded(const IRCUserInfo &userInfo);
	void nameListUpdated(const IRCUserList &userList);
	void nameRemoved(const IRCUserInfo &userInfo);
	void nameUpdated(const IRCUserInfo &userInfo);

	/**
	 * @brief Captures signals from IRC Networks which indicate that a new
	 * chat window is being opened.
	 */
	void newChatWindowIsOpened(IRCChatAdapter *pAdapter);

	void myNicknameUsedSlot();

	void receiveError(const QString &error);
	void sendMessage();
	void userListCustomContextMenuRequested(const QPoint &pos);
	void userListDoubleClicked();

protected:
	IRCAdapterBase *pIrcAdapter;
	IRCDock *pParentIRCDock;

	bool eventFilter(QObject *watched, QEvent *event) override;

private:
	class UserListMenu;

	enum PrivChatMenu
	{
		PrivWhois,
		PrivCtcpPing,
		PrivCtcpTime,
		PrivCtcpVersion,
		PrivIgnore
	};

	DPtr<IRCDockTabContents> d;
	friend class PrivData<IRCDockTabContents>;

	static const int BLINK_TIMER_DELAY_MS;

	void alertIfConfigured();
	void appendGeneralChatContextMenuOptions(QMenu *menu);
	void appendPrivChatContextMenuOptions(QMenu *menu);
	void appendPrivChatContextMenuAction(QMenu *menu, const QString &text, PrivChatMenu type);

	void completeNickname();
	QStandardItem *findUserListItem(const QString &nickname);
	UserListMenu &getUserListContextMenu();
	void insertMessage(const IRCMessageClass &messageClass, const QString &htmlString);
	void markDate();
	IRCNetworkAdapter *network();
	const IRCNetworkEntity &networkEntity() const;
	bool openLog();
	QString recipient() const;
	bool restoreLog();
	void rotateOldLog();
	QString selectedNickname();

	void sendCtcpPing(const QString &nickname);
	void sendCtcpTime(const QString &nickname);
	void sendCtcpVersion(const QString &nickname);
	void sendWhois(const QString &nickname);

	/**
	 * Sets bBlinkTitle to specified value and emits
	 * titleChange() signal if new value was different than the
	 * previous one.
	 */
	void setBlinkTitle(bool b);
	/**
	 * @brief Deletes current model, applies a new, empty one.
	 */
	void setupNewUserListModel();
	void startIgnoreOperation(const QString &nickname);

	QString wrapTextWithMetaTags(const QString &text,
		const IRCMessageClass &messageClass) const;

	bool writeLog(const QString &text);

private slots:
	void blinkTimerSlot();
	void onFocusChanged(QWidget *old, QWidget *now);
	void onPrivChatActionTriggered();
	void printToSendersNetworksCurrentChatBox(const QString &text, const IRCMessageClass &msgClass);
	void resetNicknameCompletion();
	void showChatContextMenu(const QPoint &pos);
	void showIgnoresManager();
};

#endif
