//------------------------------------------------------------------------------
// missingwadsdialog.h
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
#ifndef id6b2989fd_5c98_4e31_ad64_cf57b7231a67
#define id6b2989fd_5c98_4e31_ad64_cf57b7231a67

#include "dptr.h"
#include <QDialog>
#include <QList>
#include <QStringList>

class PWad;

class MissingWadsDialog : public QDialog
{
	Q_OBJECT

public:
	enum MissingWadsProceed
	{
		Ignore,
		Cancel,
		Install
	};

	MissingWadsDialog(const QList<PWad> &missingWads, const QList<PWad> &incompatibleWads, QWidget *parent);
	~MissingWadsDialog() override;

	MissingWadsProceed decision() const;
	QList<PWad> filesToDownload() const;
	void setAllowIgnore(bool);

private:
	DPtr<MissingWadsDialog> d;

	QList<PWad> downloadableFiles() const;
	QList<PWad> forbiddenFiles() const;
	QList<PWad> optionalFiles() const;

	QList<PWad> incompatibleFiles() const;
	bool hasAnyAllowedFile() const;
	bool isFreedoomReplaceableOnList(const QStringList &files) const;
	QList<PWad> selectedOptionalFiles() const;
	QList<PWad> selectedIncompatibleFiles() const;
	QList<PWad> filenamesToPwads(const QStringList &names, QList<PWad> files) const;
	void setup();
	void setupForbiddenFilesArea();
	void setupDownloadableFilesArea();
	void setupIncompatibleFilesArea();
	void setupOptionalFilesArea();
	void setupWadseekerIsRunning();
	void setupWadseekerNotRunning();

private slots:
	void ignoreMissingFiles();
	void installFreedoom();
	void installMissingFiles();
	void updateStateAccordingToFileSelection();
};

#endif
