//------------------------------------------------------------------------------
// taskbarprogress.h
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
#ifndef id3efbc508_293e_46ed_8518_73e88b9ba584
#define id3efbc508_293e_46ed_8518_73e88b9ba584

#include "dptr.h"
#include <QObject>

class QWinTaskbarProgress;

/**
 * @brief Platform-agnostic wrapper for QWinTaskbarProgress.
 *
 * Actually, the platform wrapping is only meant to avoid having to
 * run #ifdef checks in the code that wishes to use the
 * TaskbarProgress. On Windows platform this is a proxy design
 * pattern. On all other platforms objects of this class are no-ops.
 */
class TaskbarProgress : public QObject
{
	Q_OBJECT

	friend class TaskbarButton;

public:
	TaskbarProgress(QObject *parent = nullptr);

	bool isPaused() const;
	bool isStopped() const;
	bool isVisible() const;
	int maximum() const;
	int minimum() const;
	int value() const;

public slots:
	void hide();
	void pause();
	void reset();
	void resume();
	void setMaximum(int maximum);
	void setMinimum(int minimum);
	void setPaused(bool paused);
	void setRange(int minimum, int maximum);
	void setValue(int value);
	void setVisible(bool visible);
	void show();
	void stop();

signals:
	void maximumChanged(int maximum);
	void minimumChanged(int minimum);
	void valueChanged(int value);
	void visibilityChanged(bool visible);

private:
	DPtr<TaskbarProgress> d;

	TaskbarProgress(QWinTaskbarProgress *progress, QObject *parent = nullptr);
	void construct();
};

#endif
