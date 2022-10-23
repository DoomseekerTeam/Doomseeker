//------------------------------------------------------------------------------
// rconpassworddialog.h
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
#ifndef id254128BC_CE43_41C3_A9C0DD520BECAB15
#define id254128BC_CE43_41C3_A9C0DD520BECAB15

#include "dptr.h"

#include <QDialog>

class EnginePlugin;

class RconPasswordDialog : public QDialog
{
	Q_OBJECT

public:
	RconPasswordDialog(QWidget *parent = nullptr, bool connection = false);
	~RconPasswordDialog() override;

	QString connectPassword() const;
	const EnginePlugin *selectedEngine() const;
	QString serverAddress() const;

private:
	DPtr<RconPasswordDialog> d;

private slots:
	void setHidePassword(bool hide);
};

#endif
