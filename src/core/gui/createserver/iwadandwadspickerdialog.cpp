//------------------------------------------------------------------------------
// iwadandwadspickerdialog.cpp
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
// Copyright (C) 2015 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "iwadandwadspickerdialog.h"

#include "filefilter.h"
#include "gui/commongui.h"
#include "ui_iwadandwadspickerdialog.h"
#include <QFileDialog>

DClass<IwadAndWadsPickerDialog> : public Ui::IwadAndWadsPickerDialog
{
};

DPointered(IwadAndWadsPickerDialog)

IwadAndWadsPickerDialog::IwadAndWadsPickerDialog(QWidget *parent)
	: QDialog(parent)
{
	d->setupUi(this);
}

IwadAndWadsPickerDialog::~IwadAndWadsPickerDialog()
{
}

QString IwadAndWadsPickerDialog::executable() const
{
	return d->executableInput->currentText();
}

void IwadAndWadsPickerDialog::setExecutables(const QStringList &paths)
{
	for (const QString &path : paths)
	{
		if (d->executableInput->findText(path) < 0)
			d->executableInput->addItem(path);
	}
}

void IwadAndWadsPickerDialog::browseExecutable()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Doomseeker - Browse executable"),
		executable(), FileFilter::executableFilesFilter());
	if (!path.isEmpty())
	{
		d->executableInput->setCurrentText(path);
		if (d->executableInput->findText(path) < 0)
			d->executableInput->addItem(path);
	}
}

QStringList IwadAndWadsPickerDialog::filePaths() const
{
	return d->wadsPicker->filePaths();
}

QString IwadAndWadsPickerDialog::iwadPath() const
{
	return d->iwadPicker->currentIwad();
}

void IwadAndWadsPickerDialog::setIwadByName(const QString &iwad)
{
	d->iwadPicker->setIwadByName(iwad);
}
