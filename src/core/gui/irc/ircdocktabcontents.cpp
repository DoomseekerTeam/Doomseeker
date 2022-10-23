//------------------------------------------------------------------------------
// ircdocktabcontents.cpp
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
#include "application.h"
#include "gui/commongui.h"
#include "gui/irc/ircignoresmanager.h"
#include "gui/irc/ircsounds.h"
#include "gui/irc/ircuserlistmodel.h"
#include "irc/chatlogrotate.h"
#include "irc/chatlogs.h"
#include "irc/configuration/chatlogscfg.h"
#include "irc/configuration/ircconfig.h"
#include "irc/entities/ircnetworkentity.h"
#include "irc/ircchanneladapter.h"
#include "irc/ircdock.h"
#include "irc/ircglobal.h"
#include "irc/ircmessageclass.h"
#include "irc/ircnetworkadapter.h"
#include "irc/ircnicknamecompleter.h"
#include "irc/ircuserinfo.h"
#include "irc/ircuserlist.h"
#include "irc/ops/ircdelayedoperationignore.h"
#include "ircdocktabcontents.h"
#include "log.h"
#include "ui_ircdocktabcontents.h"
#include <cassert>
#include <QDateTime>
#include <QFile>
#include <QKeyEvent>
#include <QMenu>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QTimer>


const int IRCDockTabContents::BLINK_TIMER_DELAY_MS = 650;

DClass<IRCDockTabContents> : public Ui::IRCDockTabContents
{
public:
	QFile log;
	QDateTime lastMessageDate;

	/**
	 *	@brief Holds blinkTimer state.
	 *
	 *	Either text shows in usual color (false) or inverted one (true).
	 *	Change to this variable should be accompanied by emitting
	 *	titleChange() signal.
	 */
	bool bBlinkTitle;
	bool bIsDestroying;

	QTimer blinkTimer;

	IRCMessageClass *lastMessageClass;
	IRCNicknameCompleter *nicknameCompleter;
	/**
	 *	@brief This is required to properly refresh colors when
	 *	appearance is changed.
	 */
	QStringList textOutputContents;
	::IRCDockTabContents::UserListMenu *userListContextMenu;
};

DPointeredNoCopy(IRCDockTabContents)

class IRCDockTabContents::UserListMenu : public QMenu
{
public:
	UserListMenu();

	QAction *ban;
	QAction *whois;
	QAction *ctcpTime;
	QAction *ctcpPing;
	QAction *ctcpVersion;
	QAction *dehalfOp;
	QAction *deop;
	QAction *devoice;
	QAction *halfOp;
	QAction *kick;
	QAction *ignore;
	QAction *op;
	QAction *openChatWindow;
	QAction *voice;

private:
	bool bIsOperator;
};

IRCDockTabContents::IRCDockTabContents(IRCDock *pParentIRCDock)
{
	d->setupUi(this);
	d->lastMessageDate = QDateTime::currentDateTime();

	d->bBlinkTitle = false;
	d->bIsDestroying = false;
	d->lastMessageClass = nullptr;
	d->userListContextMenu = nullptr;
	this->pIrcAdapter = nullptr;

	this->pParentIRCDock = pParentIRCDock;
	d->nicknameCompleter = new IRCNicknameCompleter();

	d->txtOutputWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	setupNewUserListModel();

	// There is only one case in which we want this to be visible:
	// if we are in a channel.
	d->lvUserList->setVisible(false);

	this->connect(d->btnSend, SIGNAL(clicked()), SLOT(sendMessage()));
	this->connect(d->leCommandLine, SIGNAL(returnPressed()), SLOT(sendMessage()));
	this->connect(gApp, SIGNAL(focusChanged(QWidget*,QWidget*)),
		SLOT(onFocusChanged(QWidget*,QWidget*)));

	applyAppearanceSettings();

	d->blinkTimer.setSingleShot(false);
	this->connect(&d->blinkTimer, SIGNAL(timeout()),
		SLOT(blinkTimerSlot()));

	// Performance check line, keep commented for non-testing builds:
	//receiveMessage(Strings::createRandomAlphaNumericStringWithNewLines(80, 5000));
}

IRCDockTabContents::~IRCDockTabContents()
{
	d->bIsDestroying = true;

	if (d->lastMessageClass != nullptr)
		delete d->lastMessageClass;

	if (d->userListContextMenu != nullptr)
		delete d->userListContextMenu;

	if (pIrcAdapter != nullptr)
	{
		disconnect(pIrcAdapter, nullptr, nullptr, nullptr);
		IRCAdapterBase *pTmpAdapter = pIrcAdapter;
		pIrcAdapter = nullptr;
		delete pTmpAdapter;
	}
}

void IRCDockTabContents::adapterFocusRequest()
{
	emit focusRequest(this);
}

void IRCDockTabContents::adapterTerminating()
{
	if (pIrcAdapter != nullptr && !d->bIsDestroying)
	{
		// Disconnect the adapter from this tab.
		disconnect(pIrcAdapter, nullptr, nullptr, nullptr);
		pIrcAdapter = nullptr;

		emit chatWindowCloseRequest(this);
	}
}

void IRCDockTabContents::alertIfConfigured()
{
	if (gIRCConfig.appearance.windowAlertOnImportantChatEvent)
		QApplication::alert(gApp->mainWindowAsQWidget());
}

void IRCDockTabContents::applyAppearanceSettings()
{
	const static QString STYLE_SHEET_BASE_TEMPLATE =
		"QListView, QTextEdit, QLineEdit { background: %1; color: %2; } ";

	const IRCConfig::AppearanceCfg &appearance = gIRCConfig.appearance;

	QString qtStyleSheet = STYLE_SHEET_BASE_TEMPLATE
		.arg(appearance.backgroundColor)
		.arg(appearance.defaultTextColor);

	QColor colorSelectedText(appearance.userListSelectedTextColor);
	QColor colorSelectedBackground(appearance.userListSelectedBackgroundColor);
	qtStyleSheet += QString("QListView::item:selected { color: %1; background: %2; } ").arg(colorSelectedText.name(), colorSelectedBackground.name());

	QColor colorHoverText = colorSelectedText.lighter();
	QColor colorHoverBackground = colorSelectedBackground.lighter();
	qtStyleSheet += QString("QListView::item:hover  { color: %1; background: %2; } ").arg(colorHoverText.name(), colorHoverBackground.name());

	QString channelActionClassName = IRCMessageClass::toStyleSheetClassName(IRCMessageClass::ChannelAction);
	QString ctcpClassName = IRCMessageClass::toStyleSheetClassName(IRCMessageClass::Ctcp);
	QString errorClassName = IRCMessageClass::toStyleSheetClassName(IRCMessageClass::Error);
	QString networkActionClassName = IRCMessageClass::toStyleSheetClassName(IRCMessageClass::NetworkAction);

	QString htmlStyleSheetMessageArea = "";
	htmlStyleSheetMessageArea += "span { white-space: pre; }";
	htmlStyleSheetMessageArea += QString("a { color: %1; white-space: pre; } ").arg(appearance.urlColor);
	htmlStyleSheetMessageArea += QString("." + channelActionClassName + " { color: %1; } ").arg(appearance.channelActionColor);
	htmlStyleSheetMessageArea += QString("." + ctcpClassName + " { color: %1; } ").arg(appearance.ctcpColor);
	htmlStyleSheetMessageArea += QString("." + errorClassName + " { color: %1; } ").arg(appearance.errorColor);
	htmlStyleSheetMessageArea += QString("." + networkActionClassName + " { color: %1; } ").arg(appearance.networkActionColor);

	d->lvUserList->setStyleSheet(qtStyleSheet);
	d->lvUserList->setFont(appearance.userListFont);

	d->leCommandLine->installEventFilter(this);
	d->leCommandLine->setStyleSheet(qtStyleSheet);
	d->leCommandLine->setFont(appearance.mainFont);

	d->txtOutputWidget->setStyleSheet(qtStyleSheet);
	d->txtOutputWidget->setFont(appearance.mainFont);

	d->txtOutputWidget->document()->setDefaultStyleSheet(htmlStyleSheetMessageArea);
	d->txtOutputWidget->clear();
	d->txtOutputWidget->insertHtml(d->textOutputContents.join(""));
	d->txtOutputWidget->moveCursor(QTextCursor::End);
}

void IRCDockTabContents::blinkTimerSlot()
{
	setBlinkTitle(!d->bBlinkTitle);
}

void IRCDockTabContents::completeNickname()
{
	IRCCompletionResult result;
	if (d->nicknameCompleter->isReset())
		result = d->nicknameCompleter->complete(d->leCommandLine->text(), d->leCommandLine->cursorPosition());
	else
		result = d->nicknameCompleter->cycleNext();
	if (result.isValid())
	{
		// Prevent reset due to cursor position change.
		d->leCommandLine->blockSignals(true);
		d->leCommandLine->setText(result.textLine);
		d->leCommandLine->setCursorPosition(result.cursorPos);
		d->leCommandLine->blockSignals(false);
	}
}

bool IRCDockTabContents::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == d->leCommandLine && event->type() == QEvent::KeyPress)
	{
		auto keyEvent = static_cast<QKeyEvent *>(event);
		if (keyEvent->key() == Qt::Key_Tab)
		{
			completeNickname();
			return true;
		}
	}
	return false;
}

QStandardItem *IRCDockTabContents::findUserListItem(const QString &nickname)
{
	auto pModel = (QStandardItemModel *)d->lvUserList->model();
	IRCUserInfo userInfo(nickname, network());

	for (int i = 0; i < pModel->rowCount(); ++i)
	{
		QStandardItem *pItem = pModel->item(i);
		if (userInfo == IRCUserInfo(pItem->text(), network()))
			return pItem;
	}

	return nullptr;
}

IRCDockTabContents::UserListMenu &IRCDockTabContents::getUserListContextMenu()
{
	if (d->userListContextMenu == nullptr)
		d->userListContextMenu = new UserListMenu();

	return *d->userListContextMenu;
}

void IRCDockTabContents::grabFocus()
{
	// Make sure the tab title is not "d->blinkTimered out" anymore.
	d->blinkTimer.stop();
	setBlinkTitle(false);

	d->leCommandLine->setFocus();
}

bool IRCDockTabContents::hasTabFocus() const
{
	return this->pParentIRCDock->hasTabFocus(this);
}

QIcon IRCDockTabContents::icon() const
{
	if (pIrcAdapter == nullptr)
		return QIcon();

	switch (pIrcAdapter->adapterType())
	{
	case IRCAdapterBase::ChannelAdapter:
		return QIcon(":/icons/irc_channel.png");

	case IRCAdapterBase::NetworkAdapter:
		return QIcon(":/flags/lan-small");

	case IRCAdapterBase::PrivAdapter:
		return QIcon(":/icons/person.png");

	default:
		return QIcon();
	}
}

void IRCDockTabContents::insertMessage(const IRCMessageClass &messageClass, const QString &htmlString)
{
	if (d->lastMessageClass == nullptr)
		d->lastMessageClass = new IRCMessageClass();
	*d->lastMessageClass = messageClass;

	d->textOutputContents << htmlString;

	// Text insertion must be done this way to allow proper
	// handling of "Pause" button. Note that the cursor
	// in the widget is not affected as textCursor() creates a copy
	// of cursor object.
	QTextCursor cursor = d->txtOutputWidget->textCursor();
	cursor.movePosition(QTextCursor::End);
	cursor.insertHtml(htmlString);

	if (!d->btnPauseTextArea->isChecked())
		d->txtOutputWidget->moveCursor(QTextCursor::End);

	emit newMessagePrinted();
}

void IRCDockTabContents::markDate()
{
	QDateTime previousMessageDate = d->lastMessageDate;
	QDateTime nowDate = QDateTime::currentDateTime();
	d->lastMessageDate = nowDate;
	if (previousMessageDate.daysTo(nowDate) != 0)
	{
		receiveMessageWithClass(tr("<<<DATE>>> Date on this computer changes to %1").arg(
			nowDate.toString()), IRCMessageClass::NetworkAction);
	}
}

void IRCDockTabContents::myNicknameUsedSlot()
{
	alertIfConfigured();
	pParentIRCDock->sounds().playIfAvailable(IRCSounds::NicknameUsed);
	if (!hasTabFocus())
		d->blinkTimer.start(BLINK_TIMER_DELAY_MS);
}

void IRCDockTabContents::nameAdded(const IRCUserInfo &userInfo)
{
	auto pModel = (QStandardItemModel *)d->lvUserList->model();
	QStandardItem *pItem = new QStandardItem(userInfo.prefixedName());
	pItem->setData(userInfo.cleanNickname(), IRCUserListModel::RoleCleanNickname);

	// Try to append the nickname at the proper place in the list.
	for (int i = 0; i < pModel->rowCount(); ++i)
	{
		QStandardItem *pExistingItem = pModel->item(i);
		QString existingNickname = pExistingItem->text();

		if (userInfo <= IRCUserInfo(existingNickname, network()))
		{
			pModel->insertRow(i, pItem);
			return;
		}
	}

	// If above code didn't return then
	// this nickname should be appended to the end of the list.
	pModel->appendRow(pItem);
}

void IRCDockTabContents::nameListUpdated(const IRCUserList &userList)
{
	setupNewUserListModel();

	for (unsigned i = 0; i < userList.size(); ++i)
		nameAdded(*userList[i]);
}

void IRCDockTabContents::nameRemoved(const IRCUserInfo &userInfo)
{
	auto pModel = (QStandardItemModel *)d->lvUserList->model();
	for (int i = 0; i < pModel->rowCount(); ++i)
	{
		QStandardItem *pItem = pModel->item(i);
		if (userInfo.isSameNickname(pItem->text()))
		{
			pModel->removeRow(i);
			break;
		}
	}
}

void IRCDockTabContents::nameUpdated(const IRCUserInfo &userInfo)
{
	nameRemoved(userInfo);
	nameAdded(userInfo);
}

IRCNetworkAdapter *IRCDockTabContents::network()
{
	return ircAdapter()->network();
}

const IRCNetworkEntity &IRCDockTabContents::networkEntity() const
{
	return ircAdapter()->networkEntity();
}

void IRCDockTabContents::newChatWindowIsOpened(IRCChatAdapter *pAdapter)
{
	// Once a new chat adapter is opened we need to add it to the master
	// dock widget.
	pParentIRCDock->addIRCAdapter(pAdapter);
}

void IRCDockTabContents::onFocusChanged(QWidget *old, QWidget *now)
{
	if (old == d->lvUserList && now != d->userListContextMenu)
		d->lvUserList->clearSelection();
}

bool IRCDockTabContents::openLog()
{
	rotateOldLog();
	ChatLogs logs;
	if (!logs.mkLogDir(networkEntity()))
	{
		receiveMessageWithClass(tr("Failed to create chat log directory:\n'%1'").arg(
			logs.networkDirPath(networkEntity())), IRCMessageClass::Error);
		return false;
	}
	d->log.setFileName(ChatLogs().logFilePath(networkEntity(), recipient()));
	d->log.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
	d->log.write(tr("<<<DATE>>> Chat log started on %1\n\n").arg(QDateTime::currentDateTime().toString()).toUtf8());
	return true;
}

void IRCDockTabContents::rotateOldLog()
{
	assert(!d->log.isOpen());
	ChatLogsCfg cfg;

	ChatLogRotate logRotate;
	logRotate.setRemovalAgeDaysThreshold(
		cfg.isRestoreChatFromLogs() ? cfg.oldLogsRemovalDaysThreshold() : -1);
	logRotate.rotate(networkEntity(), recipient());
}

void IRCDockTabContents::printToSendersNetworksCurrentChatBox(const QString &text, const IRCMessageClass &msgClass)
{
	auto adapter = static_cast<IRCAdapterBase *>(sender());
	IRCDockTabContents *tab = pParentIRCDock->tabWithFocus();
	if (tab != nullptr && tab->ircAdapter()->network()->isAdapterRelated(adapter))
		tab->ircAdapter()->emitMessageWithClass(text, msgClass);
	else
		adapter->emitMessageWithClass(text, msgClass);
}

void IRCDockTabContents::receiveError(const QString &error)
{
	receiveMessageWithClass(tr("Error: %1").arg(error), IRCMessageClass::Error);
}

void IRCDockTabContents::receiveMessage(const QString &message)
{
	receiveMessageWithClass(message, IRCMessageClass::Normal);
}

void IRCDockTabContents::receiveMessageWithClass(const QString &message, const IRCMessageClass &messageClass)
{
	markDate();

	QString messageHtmlEscaped = message;

	if (gIRCConfig.appearance.timestamps)
	{
		QString timestamp = Strings::timestamp("[hh:mm:ss] ");

		messageHtmlEscaped = timestamp + messageHtmlEscaped;

		// It is also required to replace all '\n' characters with timestamp
		// markers to ensure that timestamp is in every line of text.
		messageHtmlEscaped = messageHtmlEscaped.replace("\n", "\n" + timestamp);
	}

	// As the new-line character is stripped by the lower levels we should
	// assume that each message ends with a new-line char, as specified
	// by RFC 1459.
	messageHtmlEscaped += "\n";

	writeLog(messageHtmlEscaped);

	messageHtmlEscaped = wrapTextWithMetaTags(messageHtmlEscaped, messageClass);

	// Play sound if this is Priv adapter.
	if (pIrcAdapter->adapterType() == IRCAdapterBase::PrivAdapter)
	{
		alertIfConfigured();
		pParentIRCDock->sounds().playIfAvailable(IRCSounds::PrivateMessageReceived);

		// If this tab doesn't have focus, also start d->blinkTimering the title.
		if (!hasTabFocus())
			d->blinkTimer.start(BLINK_TIMER_DELAY_MS);
	}

	this->insertMessage(messageClass, messageHtmlEscaped);
}

QString IRCDockTabContents::recipient() const
{
	return pIrcAdapter->recipient();
}

void IRCDockTabContents::resetNicknameCompletion()
{
	d->nicknameCompleter->reset();
}

bool IRCDockTabContents::restoreLog()
{
	ChatLogs logs;
	QFile file(logs.logFilePath(networkEntity(), recipient()));
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QByteArray contents = file.readAll();
		QStringList lines = QString::fromUtf8(contents, contents.size()).split("\n");
		int line = lines.size() - 1000;
		lines = lines.mid((line > 0) ? line : 0);

		insertMessage(IRCMessageClass::Normal,
			wrapTextWithMetaTags(lines.join("\n"), IRCMessageClass::Normal));

		receiveMessageWithClass(tr("---- All lines above were loaded from log ----"),
			IRCMessageClass::NetworkAction);
		return true;
	}
	return false;
}

QString IRCDockTabContents::selectedNickname()
{
	QModelIndexList selectedIndexes = d->lvUserList->selectionModel()->selectedRows();
	// There can be only one.
	if (!selectedIndexes.isEmpty())
	{
		int row = selectedIndexes[0].row();
		auto pModel = (QStandardItemModel *)d->lvUserList->model();
		QStandardItem *pItem = pModel->item(row);

		return pItem->text();
	}

	return "";
}

void IRCDockTabContents::sendCtcpPing(const QString &nickname)
{
	network()->sendCtcp(nickname, QString("PING %1").arg(QDateTime::currentMSecsSinceEpoch()));
}

void IRCDockTabContents::sendCtcpTime(const QString &nickname)
{
	network()->sendCtcp(nickname, QString("TIME"));
}

void IRCDockTabContents::sendCtcpVersion(const QString &nickname)
{
	network()->sendCtcp(nickname, QString("VERSION"));
}

void IRCDockTabContents::sendMessage()
{
	QString message = d->leCommandLine->text();
	d->leCommandLine->setText("");

	if (!message.trimmed().isEmpty())
		pIrcAdapter->sendMessage(message);
}

void IRCDockTabContents::sendWhois(const QString &nickname)
{
	network()->sendMessage(QString("/WHOIS %1").arg(nickname));
}

void IRCDockTabContents::setBlinkTitle(bool b)
{
	bool bEmit = false;
	if (d->bBlinkTitle != b)
	{
		// Delay signal emit until after we change the variable.
		bEmit = true;
	}

	d->bBlinkTitle = b;

	if (bEmit)
		emit titleBlinkRequested();
}

void IRCDockTabContents::setIRCAdapter(IRCAdapterBase *pAdapter)
{
	assert(pIrcAdapter == nullptr);
	pIrcAdapter = pAdapter;

	ChatLogsCfg cfg;
	if (cfg.isRestoreChatFromLogs())
		restoreLog();
	if (cfg.isStoreLogs())
		openLog();

	connect(pIrcAdapter, SIGNAL(error(const QString&)), SLOT(receiveError(const QString&)));
	connect(pIrcAdapter, SIGNAL(focusRequest()), SLOT(adapterFocusRequest()));
	connect(pIrcAdapter, SIGNAL(message(const QString&)), SLOT(receiveMessage(const QString&)));
	connect(pIrcAdapter, SIGNAL(messageWithClass(const QString&,const IRCMessageClass&)), SLOT(receiveMessageWithClass(const QString&,const IRCMessageClass&)));
	connect(pIrcAdapter, SIGNAL(terminating()), SLOT(adapterTerminating()));
	connect(pIrcAdapter, SIGNAL(titleChange()), SLOT(adapterTitleChange()));
	connect(pIrcAdapter, SIGNAL(messageToNetworksCurrentChatBox(QString,IRCMessageClass)),
		SLOT(printToSendersNetworksCurrentChatBox(QString,IRCMessageClass)));

	switch (pIrcAdapter->adapterType())
	{
	case IRCAdapterBase::NetworkAdapter:
	{
		auto pNetworkAdapter = (IRCNetworkAdapter *)pAdapter;
		connect(pNetworkAdapter, SIGNAL(newChatWindowIsOpened(IRCChatAdapter*)), SLOT(newChatWindowIsOpened(IRCChatAdapter*)));
		break;
	}

	case IRCAdapterBase::ChannelAdapter:
	{
		auto pChannelAdapter = (IRCChannelAdapter *)pAdapter;
		connect(pChannelAdapter, SIGNAL(myNicknameUsed()), SLOT(myNicknameUsedSlot()));
		connect(pChannelAdapter, SIGNAL(nameAdded(const IRCUserInfo&)), SLOT(nameAdded(const IRCUserInfo&)));
		connect(pChannelAdapter, SIGNAL(nameListUpdated(const IRCUserList&)), SLOT(nameListUpdated(const IRCUserList&)));
		connect(pChannelAdapter, SIGNAL(nameRemoved(const IRCUserInfo&)), SLOT(nameRemoved(const IRCUserInfo&)));
		connect(pChannelAdapter, SIGNAL(nameUpdated(const IRCUserInfo&)), SLOT(nameUpdated(const IRCUserInfo&)));

		d->lvUserList->setVisible(true);
		connect(d->lvUserList, SIGNAL(customContextMenuRequested(const QPoint&)),
			SLOT(userListCustomContextMenuRequested(const QPoint&)));

		connect(d->lvUserList, SIGNAL(doubleClicked(const QModelIndex&)),
			SLOT(userListDoubleClicked()));

		d->lvUserList->setContextMenuPolicy(Qt::CustomContextMenu);

		break;
	}

	case IRCAdapterBase::PrivAdapter:
	{
		break;
	}

	default:
	{
		receiveError("Doomseeker error: Unknown IRCAdapterBase*");
		break;
	}
	}
}

void IRCDockTabContents::setupNewUserListModel()
{
	d->lvUserList->setModel(new QStandardItemModel(d->lvUserList));
	d->nicknameCompleter->setModel(d->lvUserList->model());
}

void IRCDockTabContents::showChatContextMenu(const QPoint &pos)
{
	QMenu *menu = d->txtOutputWidget->createStandardContextMenu(pos);
	if (ircAdapter()->adapterType() == IRCAdapterBase::PrivAdapter)
	{
		menu->addSeparator();
		appendPrivChatContextMenuOptions(menu);
	}
	menu->addSeparator();
	appendGeneralChatContextMenuOptions(menu);
	menu->exec(d->txtOutputWidget->mapToGlobal(pos));
	delete menu;
}

void IRCDockTabContents::appendGeneralChatContextMenuOptions(QMenu *menu)
{
	QAction *manageIgnores = menu->addAction(tr("Manage ignores"));
	this->connect(manageIgnores, SIGNAL(triggered()), SLOT(showIgnoresManager()));
}

void IRCDockTabContents::appendPrivChatContextMenuOptions(QMenu *menu)
{
	appendPrivChatContextMenuAction(menu, tr("Whois"), PrivWhois);
	appendPrivChatContextMenuAction(menu, tr("CTCP Ping"), PrivCtcpPing);
	appendPrivChatContextMenuAction(menu, tr("CTCP Time"), PrivCtcpTime);
	appendPrivChatContextMenuAction(menu, tr("CTCP Version"), PrivCtcpVersion);
	appendPrivChatContextMenuAction(menu, tr("Ignore"), PrivIgnore);
}

void IRCDockTabContents::appendPrivChatContextMenuAction(QMenu *menu,
	const QString &text, PrivChatMenu type)
{
	QAction *action = menu->addAction(text);
	action->setData(type);
	this->connect(action, SIGNAL(triggered()), SLOT(onPrivChatActionTriggered()));
}

void IRCDockTabContents::onPrivChatActionTriggered()
{
	QString nickname = ircAdapter()->recipient();
	QString cleanNickname = IRCUserInfo(nickname, network()).cleanNickname();
	auto action = static_cast<QAction *>(sender());
	switch (action->data().toInt())
	{
	case PrivWhois:
		sendWhois(cleanNickname);
		break;
	case PrivCtcpPing:
		sendCtcpPing(cleanNickname);
		break;
	case PrivCtcpTime:
		sendCtcpTime(cleanNickname);
		break;
	case PrivCtcpVersion:
		sendCtcpVersion(cleanNickname);
		break;
	case PrivIgnore:
		startIgnoreOperation(cleanNickname);
		break;
	default:
		assert(0 && "Unsupported priv chat action");
		qDebug() << "Unsupported priv chat action: " << action->data();
		break;
	}
}

void IRCDockTabContents::showIgnoresManager()
{
	auto dialog = new IRCIgnoresManager(this, networkEntity().description());
	connect(dialog, SIGNAL(accepted()), network(), SLOT(reloadNetworkEntityFromConfig()));
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void IRCDockTabContents::startIgnoreOperation(const QString &nickname)
{
	auto op = new IRCDelayedOperationIgnore(this, network(), nickname);
	op->setShowPatternPopup(true);
	op->start();
}

QString IRCDockTabContents::title() const
{
	return pIrcAdapter->title();
}

QString IRCDockTabContents::titleColor() const
{
	if (d->lastMessageClass != nullptr && !this->hasTabFocus())
	{
		QString color;

		if (*d->lastMessageClass == IRCMessageClass::Normal)
			color = "#ff0000";
		else
			color = d->lastMessageClass->colorFromConfig();

		if (d->bBlinkTitle)
		{
			QColor c(color);

			int rInverted = 0xff - c.red();
			int gInverted = 0xff - c.green();
			int bInverted = 0xff - c.blue();

			QColor inverted(rInverted, gInverted, bInverted);

			return inverted.name();
		}
		else
			return color;
	}

	return "";
}

void IRCDockTabContents::userListCustomContextMenuRequested(const QPoint &pos)
{
	if (this->pIrcAdapter->adapterType() != IRCAdapterBase::ChannelAdapter)
	{
		// Prevent illegal calls.
		return;
	}

	QString nickname = this->selectedNickname();
	if (nickname.isEmpty())
	{
		// Prevent calls if there is no one selected.
		return;
	}
	QString cleanNickname = IRCUserInfo(nickname, network()).cleanNickname();

	auto pAdapter = (IRCChannelAdapter *) this->pIrcAdapter;
	const QString &channel = pAdapter->recipient();

	UserListMenu &menu = this->getUserListContextMenu();
	QPoint posGlobal = d->lvUserList->mapToGlobal(pos);

	QAction *pAction = menu.exec(posGlobal);

	if (pAction == nullptr)
		return;

	if (pAction == menu.ban)
	{
		bool bOk = false;

		QString reason = CommonGUI::askString(tr("Ban user"), tr("Input reason for banning user %1 from channel %2").arg(nickname, channel), &bOk);
		if (bOk)
			pAdapter->banUser(cleanNickname, reason);
	}
	else if (pAction == menu.ctcpTime)
		sendCtcpTime(cleanNickname);
	else if (pAction == menu.ctcpPing)
		sendCtcpPing(cleanNickname);
	else if (pAction == menu.ctcpVersion)
		sendCtcpVersion(cleanNickname);
	else if (pAction == menu.deop)
		pAdapter->setOp(cleanNickname, false);
	else if (pAction == menu.dehalfOp)
		pAdapter->setHalfOp(cleanNickname, false);
	else if (pAction == menu.devoice)
		pAdapter->setVoiced(cleanNickname, false);
	else if (pAction == menu.halfOp)
		pAdapter->setHalfOp(cleanNickname, true);
	else if (pAction == menu.ignore)
		startIgnoreOperation(cleanNickname);
	else if (pAction == menu.kick)
	{
		bool bOk = false;

		QString reason = CommonGUI::askString(tr("Kick user"), tr("Input reason for kicking user %1 from channel %2").arg(nickname, channel), &bOk);
		if (bOk)
			pAdapter->kickUser(cleanNickname, reason);
	}
	else if (pAction == menu.op)
		pAdapter->setOp(cleanNickname, true);
	else if (pAction == menu.openChatWindow)
		pAdapter->network()->openNewAdapter(cleanNickname);
	else if (pAction == menu.voice)
		pAdapter->setVoiced(cleanNickname, true);
	else if (pAction == menu.whois)
		sendWhois(cleanNickname);
}

void IRCDockTabContents::userListDoubleClicked()
{
	if (this->pIrcAdapter->adapterType() != IRCAdapterBase::ChannelAdapter)
	{
		// Prevent illegal calls.
		return;
	}

	QString nickname = this->selectedNickname();
	if (nickname.isEmpty())
	{
		// Prevent calls if there is no one selected.
		return;
	}
	QString cleanNickname = IRCUserInfo(nickname, network()).cleanNickname();

	this->pIrcAdapter->network()->openNewAdapter(cleanNickname);
}

QString IRCDockTabContents::wrapTextWithMetaTags(const QString &text,
	const IRCMessageClass &messageClass) const
{
	QString result = text;
	result.replace("<", "&lt;").replace(">", "&gt;");
	result = Strings::wrapUrlsWithHtmlATags(result);

	QString className = messageClass.toStyleSheetClassName();
	if (className.isEmpty())
		result = "<span>" + result + "</span>";
	else
		result = ("<span class='" + className + "'>" + result + "</span>");
	return result;
}

bool IRCDockTabContents::writeLog(const QString &text)
{
	ChatLogsCfg cfg;
	if (d->log.isOpen() && cfg.isStoreLogs())
	{
		d->log.write(text.toUtf8());
		d->log.flush();
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////
IRCDockTabContents::UserListMenu::UserListMenu()
{
	this->openChatWindow = this->addAction(tr("Open chat window"));
	this->addSeparator();
	this->whois = this->addAction(tr("Whois"));
	this->ctcpTime = this->addAction(tr("CTCP Time"));
	this->ctcpPing = this->addAction(tr("CTCP Ping"));
	this->ctcpVersion = this->addAction(tr("CTCP Version"));
	this->addSeparator();
	this->op = this->addAction(tr("Op"));
	this->deop = this->addAction(tr("Deop"));
	this->halfOp = this->addAction(tr("Half op"));
	this->dehalfOp = this->addAction(tr("De half op"));
	this->voice = this->addAction(tr("Voice"));
	this->devoice = this->addAction(tr("Devoice"));
	this->addSeparator();
	this->ignore = this->addAction(tr("Ignore"));
	this->kick = this->addAction(tr("Kick"));
	this->ban = this->addAction(tr("Ban"));

	this->bIsOperator = false;
}
