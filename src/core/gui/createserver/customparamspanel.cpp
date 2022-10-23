//------------------------------------------------------------------------------
// customparamspanel.cpp
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
#include "customparamspanel.h"
#include "ui_customparamspanel.h"

#include "ini/ini.h"
#include "scanner.h"
#include "serverapi/gamecreateparams.h"

DClass<CustomParamsPanel> : public Ui::CustomParamsPanel
{
};

DPointered(CustomParamsPanel)

CustomParamsPanel::CustomParamsPanel(QWidget *parent)
	: QWidget(parent)
{
	d->setupUi(this);
}

CustomParamsPanel::~CustomParamsPanel()
{
}

void CustomParamsPanel::fillInParams(GameCreateParams &params)
{
	QString customParams = d->paramsArea->toPlainText();
	QByteArray utf8 = customParams.toUtf8();
	Scanner sc(utf8.constData(), utf8.length());
	while (sc.nextString())
	{
		params.customParameters() << sc->str();
	}
}

void CustomParamsPanel::loadConfig(Ini &config)
{
	IniSection misc = config.section("Misc");
	d->paramsArea->document()->setPlainText(misc["CustomParams"]);
}

void CustomParamsPanel::saveConfig(Ini &config)
{
	IniSection misc = config.section("Misc");
	misc["CustomParams"] = d->paramsArea->toPlainText();
}
