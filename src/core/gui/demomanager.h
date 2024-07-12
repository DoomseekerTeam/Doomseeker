//------------------------------------------------------------------------------
// demomanager.h
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
// Copyright (C) 2011 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------
#ifndef __DEMOMANAGER_H__
#define __DEMOMANAGER_H__

#include "dptr.h"
#include "global.h"

#include <QDialog>

class QAbstractButton;
class QByteArray;
class QFileInfo;
class QModelIndex;

/**
 *	@brief Dialog for managing demos recorded through Doomseeker.
 */
class DemoManagerDlg : public QDialog
{
	Q_OBJECT

public:
	DemoManagerDlg();
	~DemoManagerDlg() override;

private slots:
	void deleteSelected();
	void exportSelectedDoomseeker();
	void exportSelectedPlain();
	void importDemo();
	void itemDoubleClicked(const QModelIndex &index);
	void itemSelected(const QModelIndex &index);
	void playSelected();

private:
	void adjustDemoList();
	bool doRemoveDemo(const QString &file);
	QString labelDemoFile(QFileInfo &file, const QByteArray &md5);
	void playDemo(const QString &demoName);
	void updatePreview();
	void updateUiSelectionState();

	DPtr<DemoManagerDlg> d;
};

#endif
