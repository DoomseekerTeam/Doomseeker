//-----------------------------------------------------------------------------
// global.h
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//-----------------------------------------------------------------------------
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <QtGlobal>
#include <QThread>

static inline quint32 READINT32(const char *pointer)
{
	return (quint32((quint8)(*pointer))) | (quint32(quint8(*(pointer + 1))) << 8) | (quint32(quint8(*(pointer + 2))) << 16) | (quint32(quint8(*(pointer + 3))) << 24);
}
static inline quint16 READINT16(const char *pointer)
{
	return (quint16((quint8)(*pointer))) | (quint16(quint8(*(pointer + 1))) << 8);
}
static inline quint16 READBIGINT16(const char *pointer)
{
	return (quint16((quint8)(*pointer)) << 8) | (quint16(quint8(*(pointer + 1))));
}
static inline quint8 READINT8(const char *pointer)
{
	return quint8(*pointer);
}

#define WRITEINT8_DIRECT(type, integer) (type)(quint8)((integer) & 0xFF)
#define WRITEINT16_DIRECT(type, integer) WRITEINT8_DIRECT(type, integer), WRITEINT8_DIRECT(type, integer >> 8)
#define WRITEINT32_DIRECT(type, integer) WRITEINT8_DIRECT(type, integer), WRITEINT8_DIRECT(type, integer >> 8), WRITEINT8_DIRECT(type, integer >> 16), WRITEINT8_DIRECT(type, integer >> 24)

static inline void WRITEINT32(char *pointer, const quint32 integer)
{
	*pointer = (quint8)(integer & 0xFF);
	*(pointer + 1) = (quint8)((integer >> 8) & 0xFF);
	*(pointer + 2) = (quint8)((integer >> 16) & 0xFF);
	*(pointer + 3) = (quint8)((integer >> 24) & 0xFF);
}
static inline void WRITEINT16(char *pointer, const quint16 integer)
{
	*pointer = (quint8)(integer & 0xFF);
	*(pointer + 1) = (quint8)((integer >> 8) & 0xFF);
}
static inline void WRITEINT8(char *pointer, const quint8 integer)
{
	*pointer = (quint8)(integer & 0xFF);
}

static inline quint32 MAKEID(quint8 a, quint8 b, quint8 c, quint8 d)
{
	return quint32(a) | (quint32(b) << 8) | (quint32(c) << 16) | (quint32(d) << 24);
}

// Now we set it up so symbols are properly exported/imported on Windows
#ifdef Q_OS_WIN32
#ifdef doomseeker_EXPORTS
#define MAIN_EXPORT __declspec(dllexport)
#define PLUGIN_EXPORT __declspec(dllimport)
#else
#define MAIN_EXPORT __declspec(dllimport)
#define PLUGIN_EXPORT __declspec(dllexport)
#endif
#else
#define MAIN_EXPORT
#define PLUGIN_EXPORT
#endif

class Sleep : private QThread
{
public:
	static void sleep(int time)
	{
		QThread::sleep(time);
	}
};

#endif /* __GLOBAL_H__ */
