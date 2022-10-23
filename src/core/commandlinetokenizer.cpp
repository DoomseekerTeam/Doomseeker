//------------------------------------------------------------------------------
// commandlinetokenizer.cpp
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
#include "commandlinetokenizer.h"

#include "scanner.h"

#ifdef Q_OS_WIN32
	#include <windows.h>
#endif

class CommandLineTokenizer::PrivData
{
public:
	#ifdef Q_OS_WIN32
	static QStringList tokenize(const QString &cmdLine)
	{
		if (cmdLine.isEmpty())
		{
			// CommandLineToArgvW() returns path to current executable
			// if lpCmdLine argument is an empty string. We don't want that
			// here.
			return QStringList();
		}
		int numArgs = 0;
		LPCWSTR winapiCmdLine = (LPCWSTR)cmdLine.utf16();
		LPWSTR *winapiTokens = CommandLineToArgvW(winapiCmdLine, &numArgs);

		if (winapiTokens == nullptr)
			return QStringList();

		QStringList result;
		for (int i = 0; i < numArgs; ++i)
		{
			// Conversion to "ushort*" seems to work for LPWSTR.
			result << QString::fromUtf16((const ushort *)winapiTokens[i]);
		}
		LocalFree(winapiTokens);
		return result;
	}
	#else
	static QStringList tokenize(const QString &cmdLine)
	{
		QStringList result;
		Scanner sc(cmdLine.toUtf8().constData(), cmdLine.length());
		while (sc.nextString())
		{
			result << sc->str();
		}
		return result;
	}
	#endif
};

QStringList CommandLineTokenizer::tokenize(const QString &cmdLine)
{
	return PrivData::tokenize(cmdLine);
}
