//------------------------------------------------------------------------------
// iwadandwadspickerdialog.h
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
#ifndef idc56cfbd4_25dc_4818_96d3_71452acc62e8
#define idc56cfbd4_25dc_4818_96d3_71452acc62e8

#include "dptr.h"
#include "global.h"
#include <QDialog>

/**
 * @ingroup group_pluginapi
 * @brief Simple dialog box where user can pick path to IWAD and
 * additional game files.
 *
 * This dialog box contains two input widgets and standard
 * accept/cancel buttons. User can pick a single IWAD from a combo
 * box, and multiple other files (PWADs, Dehacked patches, etc.) using
 * a list that also accepts drag'n'drops. Files can also be browsed
 * using OS standard file browser dialog.
 *
 * Doomseeker will attempt to find all known IWADs in "File paths"
 * that are configured by the user. It will populate the combo box
 * with all found paths. If it's desired, the dialog can be instructed
 * to select a default IWAD by name using setIwadByName() method.
 *
 * If dialog is "Accepted", results can be obtained through
 * filePaths() and iwadPath() accessors.
 */
class MAIN_EXPORT IwadAndWadsPickerDialog : public QDialog
{
	Q_OBJECT

public:
	IwadAndWadsPickerDialog(QWidget *parent);
	virtual ~IwadAndWadsPickerDialog() override;

	QString executable() const;
	void setExecutables(const QStringList &executables);

	/**
	 * @brief List of file paths (excluding IWAD) that were picked by
	 * the user.
	 */
	QStringList filePaths() const;

	/**
	 * @brief Path to IWAD file that was picked by the user.
	 */
	QString iwadPath() const;

	/**
	 * @brief Attempt to chose default IWAD by name.
	 *
	 * Attempts to set the path to the IWAD. It might be unsuccessful
	 * if IWAD cannot be found in any directory that was preconfigured
	 * by the user. In this case the method fails silently and does
	 * nothing.
	 */
	void setIwadByName(const QString &iwad);

private:
	DPtr<IwadAndWadsPickerDialog> d;

private slots:
	void browseExecutable();
};

#endif
