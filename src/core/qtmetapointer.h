//------------------------------------------------------------------------------
// qtmetapointer.h
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __QTMETAPOINTER_H__
#define __QTMETAPOINTER_H__

#include <Qt>

class QtMetaPointer
{
private:
	void copy(const QtMetaPointer &copyin)
	{
		ptr = copyin.ptr;
	}

	void *ptr;

public:
	QtMetaPointer() {}
	QtMetaPointer(void *ptr)
	{
		this->ptr = ptr;
	}

	QtMetaPointer(const QtMetaPointer &copyin)
	{
		copy(copyin);
	}

	QtMetaPointer &operator=(const QtMetaPointer &rhs)
	{
		if (this != &rhs)
			copy(rhs);

		return *this;
	}

	QtMetaPointer &operator=(void *rhs)
	{
		this->ptr = rhs;
		return *this;
	}

	~QtMetaPointer() {}

	friend bool operator==(const void *fPtr, const QtMetaPointer &ref)
	{
		return fPtr == ref.ptr;
	}

	void *operator->()
	{
		return ptr;
	}

	operator void *()
	{
		return ptr;
	}
	operator const void *() const { return ptr; }
};

Q_DECLARE_METATYPE(QtMetaPointer)

#endif
