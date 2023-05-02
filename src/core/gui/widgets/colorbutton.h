//------------------------------------------------------------------------------
// colorbutton.h
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
#ifndef __COLORBUTTON_H__
#define __COLORBUTTON_H__

#include <QPushButton>

/**
 *	@brief A button widget designed to select a color.
 *
 *	This will paint itself in a color it represents. Clicking this button
 *	will bring up a standard color selection dialog box. Operating this control
 *	is as easy as it gets:
 *	Just drop it inside a container and either capture colorUpdated() signals
 *	or extract the color when ready through one of the color methods.
 */
class ColorButton : public QPushButton
{
	Q_OBJECT

public:
	ColorButton(QWidget *parent = nullptr);

	QString colorHtml() const;
	const QColor &colorQ() const
	{
		return color;
	}
	unsigned colorUnsigned() const;

	void setColor(unsigned colorValue);
	void setColorHtml(const QString &colorHtml);
	QSize sizeHint() const override;

signals:
	void colorUpdated(QColor oldColor, const QColor &newColor);

protected:
	QColor color;

	void updateAppearance();

	/**
	 *	@brief Will always emit colorUpdated() signal.
	 */
	void updateColor(const QColor &newColor);

protected slots:
	void thisClicked();
};

#endif
