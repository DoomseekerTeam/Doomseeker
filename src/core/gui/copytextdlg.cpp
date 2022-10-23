//------------------------------------------------------------------------------
// copytextdlg.cpp
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
// Copyright (C) 2009 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "clipboard.h"
#include "copytextdlg.h"
#include "gui/commongui.h"

DClass<CopyTextDlg> : public Ui::CopyTextDlg
{
};

DPointered(CopyTextDlg)

CopyTextDlg::CopyTextDlg(const QString &content, const QString &description, QWidget *parent) : QDialog(parent)
{
	d->setupUi(this);
	CommonGUI::setupDialog(*this);

	connect(d->btnCopy, SIGNAL(clicked()), SLOT(copyContent()));

	if (!description.isNull())
		d->lblDescription->setText(description);

	d->teContent->document()->setPlainText(content);

	// Ensure that the text is displayed from the top.
	d->teContent->moveCursor(QTextCursor::Start);
	d->teContent->ensureCursorVisible();
}

CopyTextDlg::~CopyTextDlg()
{
}

void CopyTextDlg::copyContent()
{
	Clipboard::setText(d->teContent->document()->toPlainText());
}
