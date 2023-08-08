//------------------------------------------------------------------------------
// tooltiprenderhint.h
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
// Copyright (C) 2023 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_SERVERAPI_TOOLTIPS_TOOLTIPRENDERHINT_H
#define DOOMSEEKER_SERVERAPI_TOOLTIPS_TOOLTIPRENDERHINT_H

#include "dptr.h"
#include "global.h"

class QFont;
class QRect;

/**
 * @ingroup group_pluginapi
 */
class MAIN_EXPORT TooltipRenderHint
{
public:
	TooltipRenderHint();
	virtual ~TooltipRenderHint();

	const QRect &boundingRect() const;
	void setBoundingRect(const QRect &rect);

	const QFont &font() const;
	void setFont(const QFont &font);

private:
	DPtr<TooltipRenderHint> d;
};

#endif
