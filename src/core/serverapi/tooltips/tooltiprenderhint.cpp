//------------------------------------------------------------------------------
// tooltiprenderhint.cpp
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
#include "tooltiprenderhint.h"

#include <QFont>
#include <QRect>

DClass<TooltipRenderHint>
{
public:
	QRect boundingRect;
	QFont font;
};

DPointered(TooltipRenderHint)

TooltipRenderHint::TooltipRenderHint() {}
TooltipRenderHint::~TooltipRenderHint() = default;

const QRect &TooltipRenderHint::boundingRect() const
{
	return d->boundingRect;
}

void TooltipRenderHint::setBoundingRect(const QRect &rect)
{
	d->boundingRect = rect;
}

const QFont &TooltipRenderHint::font() const
{
	return d->font;
}

void TooltipRenderHint::setFont(const QFont &font)
{
	d->font = font;
}
