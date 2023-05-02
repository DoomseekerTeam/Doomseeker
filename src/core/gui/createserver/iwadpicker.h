//------------------------------------------------------------------------------
// iwadpicker.h
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
#ifndef id6733139c_96d4_4ad7_bc7a_04bf8a85f5bf
#define id6733139c_96d4_4ad7_bc7a_04bf8a85f5bf

#include "dptr.h"

#include <QWidget>

class GameCreateParams;
class Ini;

class IwadPicker : public QWidget
{
	Q_OBJECT

public:
	IwadPicker(QWidget *parent);
	~IwadPicker() override;

	void addIwad(const QString &path);
	QString currentIwad() const;
	void loadIwads();
	void setIwadByName(const QString &iwad);

private:
	DPtr<IwadPicker> d;

private slots:
	void browse();
};

#endif
