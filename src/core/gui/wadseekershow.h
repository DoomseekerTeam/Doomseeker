//------------------------------------------------------------------------------
// wadseekershow.h
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
#ifndef id61C47D80_80D8_4824_974F9E00211D3FD1
#define id61C47D80_80D8_4824_974F9E00211D3FD1

#include "dptr.h"
#include "serverapi/serverptr.h"
#include <QObject>

#define gWadseekerShow (WadseekerShow::instance())

class WadseekerInterface;

class WadseekerShow : public QObject
{
	Q_OBJECT

public:
	static WadseekerShow *instance();

	bool checkWadseekerValidity(QWidget *parent);

private:
	static WadseekerShow *staticInstance;
	DPtr<WadseekerShow> d;

	WadseekerShow();
};

#endif
