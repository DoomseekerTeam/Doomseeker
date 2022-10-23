//------------------------------------------------------------------------------
// polymorphism.h
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef idDB7BACD7_96AF_4A8E_9530F074031E6A19
#define idDB7BACD7_96AF_4A8E_9530F074031E6A19

#define POLYMORPHIC_SETTER_DECLARE(ret, self, name, args) \
	template<typename X> void set_##name(ret (X::*x) args) \
	{ \
		set_##name(static_cast<ret (self::*) args>(x)); \
	} \
	void set_##name(ret (self::*f) args); \

#define POLYMORPHIC_DEFINE(ret, self, name, args, callargs) \
	void self::set_##name(ret (self::*f) args) \
	{ \
		d->name = f; \
	} \
	ret self::name args \
	{ \
		return (this->*d->name)callargs; \
	}

#define POLYMORPHIC_SETTER_DECLARE_CONST(ret, self, name, args) \
	template<typename X> void set_##name(ret (X::*x) args const) \
	{ \
		set_##name(static_cast<ret (self::*) args const>(x)); \
	} \
	void set_##name(ret (self::*f) args const); \

#define POLYMORPHIC_DEFINE_CONST(ret, self, name, args, callargs) \
	void self::set_##name(ret (self::*f) args const) \
	{ \
		d->name = f; \
	} \
	ret self::name args const \
	{ \
		return (this->*d->name)callargs; \
	}

#endif
