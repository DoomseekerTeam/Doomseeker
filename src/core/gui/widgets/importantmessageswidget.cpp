//------------------------------------------------------------------------------
// importantmessageswidget.cpp
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
#include "importantmessageswidget.h"

#include "ui_importantmessageswidget.h"
#include <QDateTime>
#include <QLabel>
#include <QPointer>
#include <QVBoxLayout>
#include <functional>

DClass<ImportantMessagesWidget> : public Ui::ImportantMessagesWidget
{
public:
	class MessageLabel
	{
	public:
		MessageLabel(QLabel *pLabel)
		{
			this->pLabel = pLabel;
			this->timeCreated = QDateTime::currentDateTime();
		}

		QLabel *pLabel;
		QDateTime timeCreated;
	};

	class Handler final : public ::ImportantMessagesWidget::Handler
	{
	public:
		Handler(QPointer<::ImportantMessagesWidget> container, QPointer<QLabel> label,
			std::function<void(QLabel *label)> remover)
			: container(container), label(label), remover(remover)
		{
		}

		void remove() override
		{
			if (container != nullptr && label != nullptr)
			{
				remover(label.data());
			}
		}

	private:
		QPointer<::ImportantMessagesWidget> container;
		QPointer<QLabel> label;
		std::function<void(QLabel *label)> remover;
	};

	static const unsigned DEFAULT_MAX_MESSAGES = 0;
	static const int MAX_MSG_KEEP_TIME_SEC = 10;

	QList<MessageLabel> labelWidgets;
	int maxMessages;
};

DPointered(ImportantMessagesWidget)

ImportantMessagesWidget::ImportantMessagesWidget(QWidget *pParent)
	: QWidget(pParent)
{
	d->maxMessages = PrivData<ImportantMessagesWidget>::DEFAULT_MAX_MESSAGES;
	d->setupUi(this);

	this->hide();
}

ImportantMessagesWidget::~ImportantMessagesWidget()
{
}

QSharedPointer<ImportantMessagesWidget::Handler> ImportantMessagesWidget::addMessage(
	const QString &message)
{
	return addMessage(message, QDateTime());
}

QSharedPointer<ImportantMessagesWidget::Handler> ImportantMessagesWidget::addMessage(
	const QString &message, const QDateTime &dateTime)
{
	QString strTimestamp = "";
	if (dateTime.isValid())
		strTimestamp = dateTime.toString("[hh:mm:ss]") + " ";

	QString formattedMessage = "<p>" + strTimestamp + message + "</p>";

	QLabel *pNewLabel = new QLabel(this);
	pNewLabel->setText(formattedMessage);
	pNewLabel->setTextInteractionFlags(pNewLabel->textInteractionFlags()
		| Qt::TextSelectableByMouse);
	pNewLabel->setCursor(Qt::IBeamCursor);
	pNewLabel->setWordWrap(true);

	auto handler = QSharedPointer<PrivData<ImportantMessagesWidget>::Handler>::create(
		QPointer<ImportantMessagesWidget>(this), QPointer<QLabel>(pNewLabel),
		[this](QLabel *label){ remove(label); });

	d->labelWidgets << PrivData<ImportantMessagesWidget>::MessageLabel(pNewLabel);
	d->messageLayout->addWidget(pNewLabel);

	// Remember that widget may be auto-hidden.
	this->show();

	// Do not forget co clear oldest widgets if necessary.
	dropOldWidgetsIfBeyondLimit();
	return handler;
}

QSharedPointer<ImportantMessagesWidget::Handler> ImportantMessagesWidget::addMessage(
	const QString &message, unsigned timestamp)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
	return addMessage(message, QDateTime::fromSecsSinceEpoch(timestamp));
#else
	return addMessage(message, QDateTime::fromTime_t(timestamp));
#endif
}

void ImportantMessagesWidget::clear()
{
	while (!d->labelWidgets.isEmpty())
	{
		removeOneOldest();
	}
}

void ImportantMessagesWidget::dropOldWidgetsIfBeyondLimit()
{
	while (d->labelWidgets.size() > d->maxMessages)
	{
		PrivData<ImportantMessagesWidget>::MessageLabel &oldestLabel = d->labelWidgets.first();
		int timeDifference = oldestLabel.timeCreated.secsTo(QDateTime::currentDateTime());

		if (timeDifference > PrivData<ImportantMessagesWidget>::MAX_MSG_KEEP_TIME_SEC)
			removeOneOldest();
		else
		{
			// Exit the loop as no message is old enough to be dropped.
			// Messages are ordered in chronological order.
			break;
		}
	}
}

void ImportantMessagesWidget::removeOldest(int num)
{
	if (num >= d->labelWidgets.size())
		clear();
	else
	{
		for (; num > 0; --num)
			removeOneOldest();
	}
}

void ImportantMessagesWidget::removeOneOldest()
{
	if (!d->labelWidgets.isEmpty())
	{
		remove(d->labelWidgets.first().pLabel);
	}
}

void ImportantMessagesWidget::remove(QLabel *label)
{
	if (label != nullptr)
	{
		for (auto it = d->labelWidgets.begin(); it != d->labelWidgets.end(); ++it)
		{
			if (it->pLabel == label)
			{
				delete it->pLabel;
				d->labelWidgets.erase(it);
				break;
			}
		}
	}
	if (d->labelWidgets.isEmpty())
	{
		// Auto-hide when there is nothing to show.
		hide();
	}
}

unsigned ImportantMessagesWidget::maxMessages() const
{
	return d->maxMessages;
}

void ImportantMessagesWidget::setMaxMessages(unsigned num)
{
	d->maxMessages = num;
}
