//------------------------------------------------------------------------------
// aboutdialog.h
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
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------

#ifndef DOOMSEEKER_GUI_ABOUTDIALOG_H
#define DOOMSEEKER_GUI_ABOUTDIALOG_H

#include "dptr.h"

#include <QDialog>

class AboutDialog : public QDialog
{
	Q_OBJECT

public:
	AboutDialog(QWidget *parent = nullptr);
	~AboutDialog() override;

public slots:
	void changePlugin(int pluginIndex);

private:
	DPtr<AboutDialog> d;

	QString copyrightVerboseNotice() const;

private slots:
	void showJsonLicense();
};

#endif /* DOOMSEEKER_GUI_ABOUTDIALOG_H */
