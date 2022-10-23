//------------------------------------------------------------------------------
// generalinfotip.h
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
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __GENERAL_INFO_TIP_H_
#define __GENERAL_INFO_TIP_H_

#include "dptr.h"
#include "global.h"
#include "serverapi/serverptr.h"
#include <QObject>
#include <QString>

class GeneralInfoTip : public QObject
{
	Q_OBJECT

public:
	GeneralInfoTip(const ServerCPtr &server);
	~GeneralInfoTip() override;

	QString generateHTML();

private:
	DPtr<GeneralInfoTip> d;

	/**
	 * Generates output in format
	 * @code
	 * <label>: <valueString> + '\\n'
	 * @endcode
	 *
	 * @return Empty string if valueString is also empty. Otherwise
	 * output in format mentioned above is generated.
	 */
	QString labelString(QString label, QString valueString);
};

#endif
