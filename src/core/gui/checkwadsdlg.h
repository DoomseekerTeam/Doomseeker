//------------------------------------------------------------------------------
// checkwadsdlg.h
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
// Copyright (C) 2019 Pol Marcet Sardà <polmarcetsarda@gmail.com>
//------------------------------------------------------------------------------
#ifndef __CHECKWADSDLG_H_
#define __CHECKWADSDLG_H_

#include "dptr.h"
#include "ui_checkwadsdlg.h"
#include <QDialog>

class PWad;
class PathFinder;

/**
 * @brief Contains the results of CheckWadsDlg::CheckWads(), categorized in
 * "incomplete", "missing" and "found" wads.
 */
struct CheckResult
{
	QList<PWad> incompatibleWads;
	QList<PWad> missingWads;
	QList<PWad> foundWads;
};

/**
 * @brief Manages the checking process of wads when trying to enter a server or
 * when executing the "Find missing WADs" operation. It will prompt a loading
 * bar dialog after 500 milliseconds of processing, that will also allow the
 * user to cancel the integrity check with a "cancel" button.
 */
class CheckWadsDlg : public QDialog
{
	Q_OBJECT

public:
	CheckWadsDlg(const PathFinder *pathFinder, QWidget *parent = nullptr);
	~CheckWadsDlg() override;

	/**
	 * @brief adds the wads to check.
	 */
	void addWads(const QList<PWad> &wads);
	/**
	 * @brief this function will check for the paths and integrity (if it
	 * is turned on in the config). If the process takes more than 500
	 * milliseconds, it will open a loading bar.
	 *
	 * @return returns the results stored in a CheckResult struct.
	 */
	const CheckResult checkWads();
signals:
	void finishedChecking();
private:
	DPtr<CheckWadsDlg> d;
private slots:
	void openWindow();
	void performCheckStep();
	void abortCheck();
};

#endif
