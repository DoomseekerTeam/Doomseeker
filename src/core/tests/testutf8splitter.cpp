//------------------------------------------------------------------------------
// testutf8splitter.cpp
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
#include "testutf8splitter.h"

#include "utf8splitter.h"
#include <QDebug>

#define T_ASSERT_EQUAL(a, b) \
	if ((a) != (b)) \
	{ \
		qDebug() << "a:" << (a) << ", b:" << (b); \
		return false; \
	}

bool TestUtf8Splitter::executeTest()
{
	{
		QString in;
		for (int i = 0; i < 250; ++i)
		{
			in.append(QChar(0x0105)); // a with 'ogonek'
		}
		QList<QByteArray> result = Utf8Splitter().split(in.toUtf8(), 385);
		T_ASSERT_EQUAL(result.size(), 2);
		T_ASSERT_EQUAL(result[0].size(), 384);
		T_ASSERT_EQUAL(result[1].size(), 116);
		T_ASSERT_EQUAL(static_cast<unsigned char>(result[0][382]), 0xc4);
		T_ASSERT_EQUAL(static_cast<unsigned char>(result[0][383]), 0x85);
		T_ASSERT_EQUAL(static_cast<unsigned char>(result[1][0]), 0xc4);
		T_ASSERT_EQUAL(static_cast<unsigned char>(result[1][1]), 0x85);
	}
	return true;
}
