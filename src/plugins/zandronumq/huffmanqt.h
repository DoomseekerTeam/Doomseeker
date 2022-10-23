//------------------------------------------------------------------------------
// huffmanqt.h
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
#ifndef id1c3588fd_6e92_42a0_8128_4daabe456f30
#define id1c3588fd_6e92_42a0_8128_4daabe456f30

#include <QByteArray>

class HuffmanQt
{
public:
	static QByteArray decode(const QByteArray &in);
	static QByteArray decode(const char *in, int size);
	static QByteArray encode(const QByteArray &in);
	static QByteArray encode(const char *in, int size);
};

#endif
