//------------------------------------------------------------------------------
// wadseekermessagetype.h
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
// Copyright (C) 2011 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef WADSEEKER_WADSEEKERMESSAGETYPE_H
#define WADSEEKER_WADSEEKERMESSAGETYPE_H

// WADSEEKER_API - Include in wadseeker dev package

namespace WadseekerLib
{
/**
 * The type of a transferred message; a log level.
 *
 * Wadseeker library emits messages as strings accompanied with this
 * enum. The message type should be treated as a log level, although its
 * numerical values are meaningless and should not be used for sorting.
 */
enum MessageType
{
	/// Regular info message.
	Notice = 0,
	/// A more important information.
	NoticeImportant = 1,
	/// Something bad happened, but Wadseeker can recover and continue.
	Error = 2,
	/// Wadseeker can't continue after this message and will stop the current seek.
	CriticalError = 3,
	/// Verbose information about URL fetches and redirects.
	Navigation = 4,
};
}

#endif
