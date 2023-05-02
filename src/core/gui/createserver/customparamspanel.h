//------------------------------------------------------------------------------
// customparamspanel.h
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
#ifndef idb5bd7932_a986_44a8_8c28_935cc89c64f2
#define idb5bd7932_a986_44a8_8c28_935cc89c64f2

#include "dptr.h"

#include <QWidget>

class GameCreateParams;
class Ini;

class CustomParamsPanel : public QWidget
{
	Q_OBJECT

public:
	CustomParamsPanel(QWidget *parent);
	~CustomParamsPanel() override;

	void fillInParams(GameCreateParams &params);
	void loadConfig(Ini &config);
	void saveConfig(Ini &config);

private:
	DPtr<CustomParamsPanel> d;
};

#endif
