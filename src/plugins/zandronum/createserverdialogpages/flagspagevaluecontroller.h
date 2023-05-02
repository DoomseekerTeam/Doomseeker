//------------------------------------------------------------------------------
// flagspagevaluecontroller.h
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
// Copyright (C) 2016 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_PLUGIN_ZANDRONUM_CREATESERVERDIALOGPAGES_FLAGSPAGEVALUECONTROLLER_H
#define DOOMSEEKER_PLUGIN_ZANDRONUM_CREATESERVERDIALOGPAGES_FLAGSPAGEVALUECONTROLLER_H

/**
 * @brief Converts numerical flags values to widget representation
 *        and vice-versa.
 */
class FlagsPageValueController
{
public:
	FlagsPageValueController() {}
	virtual ~FlagsPageValueController() {}

	virtual void convertNumericalToWidgets() = 0;
	virtual void convertWidgetsToNumerical() = 0;
	virtual void setVisible(bool visible)
	{
		Q_UNUSED(visible);
	}

private:
	FlagsPageValueController(const FlagsPageValueController &other) = delete;
};

#endif
