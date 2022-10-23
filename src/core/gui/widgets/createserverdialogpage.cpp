//------------------------------------------------------------------------------
// createserverdialogpage.cpp
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
#include "createserverdialogpage.h"

#include "gui/createserverdialog.h"

DClass<CreateServerDialogPage>
{
public:
	QString name;
	CreateServerDialog *pParentDialog;
};

DPointered(CreateServerDialogPage)

CreateServerDialogPage::CreateServerDialogPage(
	CreateServerDialog *pParentDialog,
	const QString &name)
	: QWidget(pParentDialog)
{
	d->pParentDialog = pParentDialog;
	d->name = name;
}

CreateServerDialogPage::~CreateServerDialogPage()
{
}

const QString &CreateServerDialogPage::name() const
{
	return d->name;
}

QDialog *CreateServerDialogPage::parentDialog()
{
	return d->pParentDialog;
}

bool CreateServerDialogPage::validate()
{
	return true;
}
