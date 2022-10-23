//------------------------------------------------------------------------------
// ircnicknamecompleter.h
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef idC00B9C6A_977E_4970_B5CD88B85BB926A9
#define idC00B9C6A_977E_4970_B5CD88B85BB926A9

#include "dptr.h"

#include <QAbstractItemModel>

class IRCCompletionResult
{
public:
	int cursorPos;
	QString textLine;

	IRCCompletionResult()
	{
		cursorPos = -1;
	}

	bool isValid() const
	{
		return cursorPos >= 0;
	}
};

class IRCNicknameCompleter
{
public:
	IRCNicknameCompleter();
	~IRCNicknameCompleter();

	/**
	 * @brief Parses current command line and returns a modified one.
	 */
	IRCCompletionResult complete(const QString &textLine, int cursorPosition);
	IRCCompletionResult cycleNext();
	bool isReset() const;
	void reset();
	void setModel(QAbstractItemModel *model);

private:
	DPtr<IRCNicknameCompleter> d;
};

#endif
