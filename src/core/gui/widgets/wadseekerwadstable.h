//------------------------------------------------------------------------------
// wadseekerwadstable.h
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
// Copyright (C) 2011 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __WADSEEKERWADSTABLE_H__
#define __WADSEEKERWADSTABLE_H__

#include <QMap>
#include <QMenu>
#include <QTime>

#include "gui/widgets/tablewidgetmouseaware.h"

class SpeedCalculator;
class ModFile;

class WadseekerWadsTable : public TableWidgetMouseAware
{
	Q_OBJECT

public:
	class ContextMenu : public QMenu
	{
		friend class WadseekerWadsTable;

	public:
		QAction *actionSkipCurrentSite;

	private:
		ContextMenu(QWidget *pParent = nullptr);
	};

	static const int IDX_NAME_COLUMN = 0;
	static const int IDX_URL_COLUMN = 1;
	static const int IDX_PROGRESS_COLUMN = 2;
	static const int IDX_SPEED_COLUMN = 3;
	static const int IDX_ETA_COLUMN = 4;
	static const int IDX_SIZE_COLUMN = 5;

	WadseekerWadsTable(QWidget *pParent = nullptr);
	~WadseekerWadsTable() override;

	void addFile(const QString &filename);

	ContextMenu *contextMenu(const QModelIndex &index, const QPoint &cursorPosition);

	/**
	 * @brief Total size of data in bytes for specified row.
	 *
	 * @return Amount of bytes if successful, negative value if
	 *         row is invalid.
	 */
	qint64 expectedDataSize(int row) const;

	QString fileNameAtRow(int row) const;

	/**
	 * @brief Total done percentage calculated basing on the data set by
	 *        siteFileProgress().
	 *
	 * @return A value between 0.0 and 100.0. If percentage cannot be
	 *         calculated a negative value is returned.
	 */
	double totalDonePercentage() const;

public slots:
	void setFileDownloadFinished(const ModFile &filename);
	void setFileFailed(const ModFile &filename);
	void setFileProgress(const ModFile &filename, qint64 current, qint64 total);
	void setFileSuccessful(const ModFile &filename);

	/**
	 * @brief Fired when new URL starts for specified file.
	 */
	void setFileUrl(const ModFile &filename, const QUrl &url);

protected:
	void showEvent(QShowEvent *pEvent) override;

private:
	class PrivData
	{
	public:
		bool bAlreadyShownOnce;

		/**
		 * @brief Hash Map containing SpeedCalculator objects for each
		 *        downloaded WAD.
		 *
		 * Key - filename
		 * Value - Pointer to SpeedCalculator instance.
		 */
		QMap<QString, SpeedCalculator *> speedCalculators;
		QTime updateClock;
	};

	static const int UPDATE_INTERVAL_MS = 1000;

	PrivData d;

	int findFileRow(const QString &filename);
	void updateDataInfoValues(bool bForce);
};

#endif
