//------------------------------------------------------------------------------
// ircmessageclass.h
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
#ifndef __IRCMESSAGECLASS_H__
#define __IRCMESSAGECLASS_H__

#include <QString>

class IRCMessageClass
{
public:
	enum ClassName
	{
		ChannelAction,
		Ctcp,
		Error,
		/**
		 * @brief Normal has no representation in string, ie.
		 *        it represents a global style for the widget.
		 */
		Normal,
		NetworkAction
	};

	static QString toStyleSheetClassName(ClassName className);

	IRCMessageClass()
	{
		this->className = Normal;
	}

	IRCMessageClass(const IRCMessageClass &other)
	{
		this->className = other.className;
	}

	IRCMessageClass(const ClassName &className)
	{
		this->className = className;
	}

	bool operator==(const IRCMessageClass &other) const
	{
		return this->className == other.className;
	}

	bool operator==(const ClassName &className) const
	{
		return this->className == className;
	}

	bool operator!=(const IRCMessageClass &other) const
	{
		return this->className != other.className;
	}

	bool operator!=(const ClassName &className) const
	{
		return this->className != className;
	}

	IRCMessageClass &operator=(const IRCMessageClass &other)
	{
		className = other.className;
		return *this;
	}

	operator ClassName() const
	{
		return this->className;
	}

	/**
	 * @brief Obtains HTML color from config for this message class.
	 */
	QString colorFromConfig() const;

	QString toStyleSheetClassName() const
	{
		return toStyleSheetClassName(this->className);
	}

private:
	ClassName className;
};

#endif
