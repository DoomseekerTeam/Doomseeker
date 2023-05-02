//------------------------------------------------------------------------------
// logdirectorypicker.h
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
#ifndef id05098fac_12b9_432d_b5ce_c2a7292c8358
#define id05098fac_12b9_432d_b5ce_c2a7292c8358

#include "dptr.h"

#include <QWidget>

class LogDirectoryPicker : public QWidget
{
	Q_OBJECT

public:
	LogDirectoryPicker(QWidget *parent);
	~LogDirectoryPicker() override;
	/**
	 * @brief returns a sanitized path. In case logging is disabled by the user,
	 * or the path inserted does not exist, returns an empty string.
	 *
	 * @return QString path where the log has to be placed.
	 */
	const QString validatedCurrentPath() const;
	const QString &currentPath() const;
	/**
	 * @brief Sets the Path, and calls updateLoggingEnabled.
	 */
	void setPathAndUpdate(const QString &path);

	const bool &isLoggingEnabled() const;
	void setLoggingEnabled(const bool &enabled);
private:
	/**
	 * @brief in case that the path does not exist, it will disable the checkbox
	 * and show a warning sign. If not, or the path is empty, it enables the
	 * checkbox and hides the sign.
	 */
	void updateLoggingStatus(const QString &path);
	DPtr<LogDirectoryPicker> d;

private slots:
	void browse();
	void cbLoggingEnabledChecked(bool isChecked);
	void textEdited();
};

#endif
