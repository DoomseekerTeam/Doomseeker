//------------------------------------------------------------------------------
// odamexcvar.h
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
// Copyright (C) 2026 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_PLUGINS_ODAMEX_ODAMEXCVAR_H
#define DOOMSEEKER_PLUGINS_ODAMEX_ODAMEXCVAR_H

#include <QString>
#include <QVariant>
#include <QtGlobal>

class DataStreamOperatorWrapper;

namespace OdamexCvar
{

enum class CvarType : quint8
{
	Boolean = 1,
	Integer8 = 2,
	Integer16 = 3,
	Integer32 = 4,
	Float = 5,
	String = 6
};

enum class IntegerInterpretation
{
	Signed,
	Unsigned
};

/**
 * Selects the interpretation for an exact name/type pair.
 */
IntegerInterpretation integerInterpretationFor(const QString &name, CvarType type);

/**
 * Reads one value without knowing its CVar name.
 *
 * Returns bool, int, uint, or QString. Byte/word integers are promoted to int
 * or uint; float-as-string fields retain their original text. Non-integer
 * values ignore the integer interpretation.
 */
QVariant readCvarValue(DataStreamOperatorWrapper &in, CvarType type,
	IntegerInterpretation interpretation);

/** Checked integer conversions; invalid or out-of-range values produce zero. */
int valueToInt(const QVariant &value);
uint valueToUInt(const QVariant &value);
}

#endif
