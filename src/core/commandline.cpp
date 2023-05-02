//------------------------------------------------------------------------------
// commandline.cpp
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "commandline.h"

#include "apprunner.h"
#include "strings.hpp"
#include <QRegExp>

void CommandLine::escapeArgs(QStringList &args)
{
	QStringList::iterator it;
	for (it = args.begin(); it != args.end(); ++it)
	{
		QString &str = *it;
		escapeArg(str);
	}
}

static bool needsQuoteWrap(const QString &arg)
{
	if (arg.isEmpty())
		return true;
	QRegExp reallySafestCharsIHope = QRegExp("[^a-z0-9/\\_-+]", Qt::CaseInsensitive);
	return arg.contains(reallySafestCharsIHope);
}

#if defined Q_OS_WIN
void CommandLine::escapeArg(QString &arg)
{
	// Note: this may be game specific (oh, dear...)
	arg.replace('"', "\\\"");
	if (needsQuoteWrap(arg))
	{
		arg.prepend('"');
		arg += '"';
	}
}

#else
// Since most other operating systems are Unix like we might as well make this a default.
void CommandLine::escapeArg(QString &arg)
{
	arg.replace('\'', "'\\''"); // This does: ' -> '\''
	if (needsQuoteWrap(arg))
	{
		arg.prepend('\'');
		arg += '\'';
	}
}
#endif

void CommandLine::escapeExecutable(QString &arg)
{
	#ifdef Q_OS_DARWIN
	QFileInfo binary = arg;
	if (binary.isBundle())
		arg += AppRunner::findBundleBinary(binary);
	#endif
	return escapeArg(arg);
}
