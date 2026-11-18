//------------------------------------------------------------------------------
// odamexcvar.cpp
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
#include "odamexcvar.h"

#include "odamexqueryreader.h"
#include <limits>

namespace OdamexCvar
{
namespace
{
struct IntegerCvarRule
{
	const char *name;
	CvarType type;
	IntegerInterpretation interpretation;
};

// The protocol labels integer CVars "(Un)signed" without a signedness flag.
// We interpret the signedness based on Odamex source-code evidence.
// The source paths below refer to Odamex release tag 12.3.0.
const IntegerCvarRule integerCvarRules[] = {
	// common/c_cvarlist.cpp declares sv_scorelimit as 0..255, and
	// server/src/sv_sqp.cpp writes it as one byte: 0x80 means 128, not -128.
	// This is not a general byte rule; e.g. g_thingfilter permits -1.
	{"sv_scorelimit", CvarType::Integer8, IntegerInterpretation::Unsigned},
	// server/src/sv_cvarlist.cpp declares this byte as 0..255.
	{"sv_maxplayersperteam", CvarType::Integer8, IntegerInterpretation::Unsigned},
	// common/c_cvarlist.cpp declares a nonnegative word limit. The
	// launcher reads it as uint16_t (odalpapi/net_packet.cpp). The declared
	// maximum of 65536 is not representable in the two-byte query field.
	{"sv_fraglimit", CvarType::Integer16, IntegerInterpretation::Unsigned},
};

template<typename T>
T checkedIntegerValue(const QVariant &value)
{
	bool ok;
	const qlonglong number = value.toLongLong(&ok);
	if (!ok || number < qlonglong(std::numeric_limits<T>::min())
		|| number > qlonglong(std::numeric_limits<T>::max()))
		return 0;
	return static_cast<T>(number);
}
}

IntegerInterpretation integerInterpretationFor(const QString &name, CvarType type)
{
	for (const IntegerCvarRule &rule : integerCvarRules)
	{
		if (rule.type == type && name == QLatin1String(rule.name))
			return rule.interpretation;
	}
	return IntegerInterpretation::Signed;
}

QVariant readCvarValue(DataStreamOperatorWrapper &in, CvarType type,
	IntegerInterpretation interpretation)
{
	switch (type)
	{
	case CvarType::Boolean:
		return true;
	case CvarType::Integer8:
		if (interpretation == IntegerInterpretation::Unsigned)
			return uint(in.readQUInt8());
		return int(in.readQInt8());
	case CvarType::Integer16:
		if (interpretation == IntegerInterpretation::Unsigned)
			return uint(in.readQUInt16());
		return int(in.readQInt16());
	case CvarType::Integer32:
		if (interpretation == IntegerInterpretation::Unsigned)
			return in.readQUInt32();
		return in.readQInt32();
	case CvarType::Float:
	case CvarType::String:
	default:
		return OdamexQuery::readCString(in);
	}
}

int valueToInt(const QVariant &value)
{
	return checkedIntegerValue<int>(value);
}

uint valueToUInt(const QVariant &value)
{
	return checkedIntegerValue<uint>(value);
}
}
