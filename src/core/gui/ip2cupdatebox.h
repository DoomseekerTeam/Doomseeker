//------------------------------------------------------------------------------
// ip2cupdatebox.h
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
#ifndef __IP2CUPDATEBOX_H__
#define __IP2CUPDATEBOX_H__

#include "dptr.h"

#include <QDialog>

class IP2CUpdateBox : public QDialog
{
	Q_OBJECT

public:
	IP2CUpdateBox(QWidget *parent = nullptr);
	~IP2CUpdateBox() override;

private:
	DPtr<IP2CUpdateBox> d;

	void start();

private slots:
	void updateInfo(int status);
};

#endif
