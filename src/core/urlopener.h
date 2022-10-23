//------------------------------------------------------------------------------
// urlopener.h
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
#ifndef idAF497E8F_22A5_4630_ACB761050B5AF2BF
#define idAF497E8F_22A5_4630_ACB761050B5AF2BF

#include "dptr.h"

#include <QObject>
#include <QUrl>

class UrlOpener : public QObject
{
	Q_OBJECT

public:
	static UrlOpener *instance();

	void open(const QUrl &url);

private:
	DPtr<UrlOpener> d;

	UrlOpener();
	~UrlOpener() override;

	static UrlOpener *inst;

private slots:
	void doOpen();
};

#endif
