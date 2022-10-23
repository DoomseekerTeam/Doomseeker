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
#ifndef WADSEEKERMESSAGETYPE_H
#define WADSEEKERMESSAGETYPE_H

// WADSEEKER_API - Include in wadseeker dev package

namespace WadseekerLib
{
/**
 * Wadseeker library uses a message system that contains
 * a type of message together with a string representing it's
 * content. Types should be treated as follows:
 *
 * - Notice - this is just an information of what Wadseeker
 *            is currently doing
 * - NoticeImportant - an information on an important event
 * - Error - something bad happened but Wadseeker is able to continue
 *           by itself.
 * - CriticalError - Wadseeker stops working after such error.
 *                   Programmer should return control to the main application.
 */
enum MessageType
{
	Notice = 0,
	NoticeImportant = 1,
	Error = 2,
	CriticalError = 3
};
}

#endif
