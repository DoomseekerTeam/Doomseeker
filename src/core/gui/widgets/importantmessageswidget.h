//------------------------------------------------------------------------------
// importantmessageswidget.h
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
// Copyright (C) 2011 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __IMPORTANTMESSAGESWIDGET_H__
#define __IMPORTANTMESSAGESWIDGET_H__

#include "dptr.h"

#include <QWidget>

class QDateTime;
class QLabel;
class QVBoxLayout;

/**
 * @brief A self-scaling widget that displays messages as separate instances of
 * QLabel.
 *
 * This widget adds new messages by wrapping them in a QLabel widgets and
 * inserting those widgets into a vertical layout. Number of QLabel widgets
 * is limited by _maxMessages field. Old messages will be removed once their
 * time of life exceeds MAX_MSG_KEEP_TIME_SEC . Widget will auto-hide and
 * auto-show
 */
class ImportantMessagesWidget : public QWidget
{
	Q_OBJECT

public:
	ImportantMessagesWidget(QWidget *pParent = nullptr);
	~ImportantMessagesWidget() override;

	/**
	 * @brief Amount of messages that can be displayed in the widget at
	 * the same time.
	 */
	unsigned maxMessages() const;
	void setMaxMessages(unsigned num);

public slots:
	/**
	 * @brief Adds a new message without a timestamp.
	 */
	void addMessage(const QString &message);

	/**
	 * @brief Adds a message with a timestamp.
	 *
	 * @param message
	 *      Message to add.
	 * @param timestamp
	 *      QDateTime object representing time.
	 */
	void addMessage(const QString &message, const QDateTime &dateTime);

	/**
	 * @brief Adds a message with a timestamp.
	 *
	 * @param message
	 *      Message to add.
	 * @param timestamp
	 *      Time in seconds since the epoch.
	 */
	void addMessage(const QString &message, unsigned timestamp);

	/**
	 * @brief Clears all messages from the widghet.
	 *
	 * Widget will shrink itself in a way that will make it invisible.
	 */
	void clear();

	/**
	 * @brief Removes a number of oldest messages from the widget.
	 *
	 * Widget will shrink with each message removed.
	 *
	 * @param num
	 *      Amount of messages to remove. If higher than the actual count
	 *      of messages, will work the same as clear().
	 */
	void removeOldest(int num);

	/**
	 * @brief Will remove one oldest message.
	 */
	void removeOneOldest();

private:
	DPtr<ImportantMessagesWidget> d;

private slots:
	/**
	 * @brief Removes old label widgets.
	 *
	 * Removes old label widgets only if they are both beyond maximum amount
	 * of widgets limit AND beyond the MAX_MSG_KEEP_TIME_SEC time limit.
	 */
	void dropOldWidgetsIfBeyondLimit();
};

#endif
