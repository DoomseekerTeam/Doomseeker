//------------------------------------------------------------------------------
// fontbutton.h
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
#ifndef __FONTBUTTON_H__
#define __FONTBUTTON_H__

#include <QPushButton>

/**
 *	@brief A button widget designed to select a font.
 *
 *	The principal of working is the same as ColorButton.
 */
class FontButton : public QPushButton
{
	Q_OBJECT

public:
	FontButton(QWidget *parent = nullptr);

	const QFont &selectedFont() const
	{
		return currentFont;
	}

	void setSelectedFont(const QFont &font);

signals:
	void fontUpdated(QFont oldFont, const QFont &newFont);

protected:
	QFont currentFont;

	void updateAppearance();

	/**
	 *	@brief Will always emit fontUpdated() signal.
	 */
	void updateFont(const QFont &newFont);

protected slots:
	void thisClicked();
};

#endif
