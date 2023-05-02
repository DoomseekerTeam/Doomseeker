//------------------------------------------------------------------------------
// message.cpp
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
#include "message.h"

QString StaticMessages::getMessage(unsigned messageType)
{
	switch (messageType)
	{
	case Message::Type::BANNED_FROM_MASTERSERVER:
		return tr("You have been banned from the master server.");
	default:
		return QString("%1 IS NOT A VALID ERROR MESSAGE! FIX THIS!").arg(messageType);
	}
}

DClass<Message>
{
public:
	QString content;
	unsigned timestamp;
	unsigned type;
};

DPointered(Message)

Message::Message()
{
	construct();
	d->type = Type::IGNORE_TYPE;
}

Message::Message(unsigned type)
{
	construct();
	d->type = type;
}

Message::Message(unsigned type, const QString &content)
{
	construct();
	d->content = content;
	d->type = type;
}

Message::Message(const Message &other)
{
	d = other.d;
}

Message &Message::operator=(const Message &other)
{
	d = other.d;
	return *this;
}

Message::~Message()
{
}

void Message::construct()
{
	qRegisterMetaType<Message>("Message");
	d->type = Type::IGNORE_TYPE;
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
	d->timestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
#else
	d->timestamp = QDateTime::currentDateTime().toTime_t();
#endif
}

QString Message::contents() const
{
	if (isCustom())
	{
		return d->content;
	}
	else
	{
		return StaticMessages::getMessage(type());
	}
}

bool Message::isCustom() const
{
	return type() == Type::CUSTOM_ERROR || type() == Type::CUSTOM_INFORMATION;
}

bool Message::isError() const
{
	return type() >= Type::CUSTOM_ERROR;
}

bool Message::isIgnore() const
{
	return type() == Type::IGNORE_TYPE;
}

bool Message::isInformation() const
{
	return (type() >= Type::CUSTOM_INFORMATION) && (type() < Type::CUSTOM_ERROR);
}

unsigned Message::timestamp() const
{
	return d->timestamp;
}

unsigned Message::type() const
{
	return d->type;
}
