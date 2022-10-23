//------------------------------------------------------------------------------
// passworddlg.cpp
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
#include "passworddlg.h"

#include "ui_passworddlg.h"

#include "configuration/passwordscfg.h"
#include "configuration/serverpassword.h"
#include "gui/commongui.h"
#include "gui/helpers/comboboxex.h"
#include "serverapi/server.h"

#include <QLineEdit>

DClass<PasswordDlg> : public Ui::PasswordDlg
{
public:
	ServerCPtr server;
	ComboBoxEx *cboConnectPassEx;
	ComboBoxEx *cboIngamePassEx;
};

DPointered(PasswordDlg)

PasswordDlg::PasswordDlg(ServerCPtr server, QWidget *parent)
	: QDialog(parent)
{
	d->setupUi(this);
	CommonGUI::setupDialog(*this);
	d->cboConnectPassEx = new ComboBoxEx(*d->cboConnectPassword);
	d->cboIngamePassEx = new ComboBoxEx(*d->cboIngamePassword);
	d->server = server;

	applyInputsVisibility();

	// Adjust the size and prevent resizing.
	adjustSize();
	setMinimumHeight(height());
	setMaximumHeight(height());

	loadConfiguration();
}

PasswordDlg::~PasswordDlg()
{
	delete d->cboConnectPassEx;
}

void PasswordDlg::accept()
{
	saveConfiguration();
	QDialog::accept();
}

void PasswordDlg::applyInputsVisibility()
{
	d->connectPasswordWidget->setVisible(d->server->isLocked());
	d->ingamePasswordWidget->setVisible(d->server->isLockedInGame());
}

QString PasswordDlg::connectPassword() const
{
	return d->cboConnectPassword->currentText();
}

QString PasswordDlg::inGamePassword() const
{
	return d->cboIngamePassword->currentText();
}

void PasswordDlg::loadConfiguration()
{
	PasswordsCfg cfg;
	if (cfg.isHidingPasswords())
	{
		d->cboConnectPassword->lineEdit()->setEchoMode(QLineEdit::Password);
		d->cboIngamePassword->lineEdit()->setEchoMode(QLineEdit::Password);
	}
	d->remember->setChecked(cfg.isRememberingConnectPhrase());
	setPasswords(cfg.serverPhrases());
	setCurrentConnectPassword(cfg.suggestPassword(
		d->server.data(), ServerPasswordType::CONNECT).phrase());
	setCurrentIngamePassword(cfg.suggestPassword(
		d->server.data(), ServerPasswordType::INGAME).phrase());
}

void PasswordDlg::removeCurrentConnectPassword()
{
	PasswordsCfg cfg;
	QString phrase = d->cboConnectPassword->currentText();
	cfg.removeServerPhrase(d->cboConnectPassword->currentText());

	d->cboIngamePassEx->removeItem(phrase);
	if (!d->cboConnectPassEx->removeCurrentItem())
	{
		d->cboConnectPassword->clearEditText();
		d->cboConnectPassword->setFocus();
	}
}

void PasswordDlg::removeCurrentIngamePassword()
{
	PasswordsCfg cfg;
	QString phrase = d->cboIngamePassword->currentText();
	cfg.removeServerPhrase(d->cboIngamePassword->currentText());

	d->cboConnectPassEx->removeItem(phrase);
	if (!d->cboIngamePassEx->removeCurrentItem())
	{
		d->cboIngamePassword->clearEditText();
		d->cboIngamePassword->setFocus();
	}
}

void PasswordDlg::saveConfiguration()
{
	PasswordsCfg cfg;
	cfg.setRememberConnectPhrase(d->remember->isChecked());
	if (d->remember->isChecked())
	{
		cfg.saveServerPhrase(connectPassword(), d->server.data(), ServerPasswordType::CONNECT);
		cfg.saveServerPhrase(inGamePassword(), d->server.data(), ServerPasswordType::INGAME);
	}
}

void PasswordDlg::setCurrentConnectPassword(const QString &password)
{
	d->cboConnectPassEx->setCurrentOrAddNewAndSelect(password);
}

void PasswordDlg::setCurrentIngamePassword(const QString &password)
{
	d->cboIngamePassEx->setCurrentOrAddNewAndSelect(password);
}

void PasswordDlg::setPasswords(const QStringList &passwords)
{
	d->cboConnectPassEx->setItemsSorted(passwords);
	d->cboIngamePassEx->setItemsSorted(passwords);
}
