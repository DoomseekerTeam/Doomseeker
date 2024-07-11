//------------------------------------------------------------------------------
// testircisupportparser.cpp
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
#include "testircisupportparser.h"

#include "irc/entities/ircuserprefix.h"
#include "irc/ircisupportparser.h"
#include "tests/asserts.h"

bool TestIRCISupportPrefix::executeTest()
{
	IRCISupportParser parser;
	parser.appendLine("AWAYLEN=300 CALLERID=g CASEMAPPING=rfc1459 "
		"CHANMODES=IXZbegw,k,FHJLdfjl,ABCDKMNOPQRSTcimnprstuz CHANNELLEN=64 "
		"CHANTYPES=# CHARSET=ascii ELIST=MU ESILENCE EXCEPTS=e "
		"EXTBAN=,ABCNOQRSTUcjmprsz FNC INVEX=I :are supported by this server");
	parser.appendLine("KICKLEN=420 MAP MAXBANS=60 MAXCHANNELS=75 MAXPARA=32 "
		"MAXTARGETS=20 MODES=20 NAMESX NETWORK=ExampleNetwork NICKLEN=20 OPERLOG "
		"OVERRIDE PREFIX=(ohav)@%^+ :are supported by this server");
	parser.appendLine("REMOVE SECURELIST SILENCE=32 SSL=109.74.206.218:6697 "
		"STARTTLS STATUSMSG=@%+ TOPICLEN=420 UHNAMES USERIP VBANLIST WALLCHOPS "
		"WALLVOICES WATCH=64 :are supported by this server");
	parser.parse();
	IRCUserPrefix prefixes = parser.userPrefixes();
	T_ASSERT_EQUAL(prefixes.modeForPrefix('@'), 'o');
	T_ASSERT_EQUAL(prefixes.modeForPrefix('%'), 'h');
	T_ASSERT_EQUAL(prefixes.modeForPrefix('^'), 'a');
	T_ASSERT_EQUAL(prefixes.modeForPrefix('+'), 'v');
	T_ASSERT_EQUAL(prefixes.prefixForMode('o'), '@');
	T_ASSERT_EQUAL(prefixes.prefixForMode('h'), '%');
	T_ASSERT_EQUAL(prefixes.prefixForMode('a'), '^');
	T_ASSERT_EQUAL(prefixes.prefixForMode('v'), '+');
	return true;
}

bool TestIRCISupportNoPrefix::executeTest()
{
	IRCISupportParser parser;
	parser.appendLine("AWAYLEN=300 :are supported by this server");
	parser.parse();
	IRCUserPrefix prefixes = parser.userPrefixes();
	T_ASSERT_EQUAL(prefixes.modeForPrefix('@'), 'o');
	T_ASSERT_EQUAL(prefixes.modeForPrefix('%'), 'h');
	T_ASSERT_EQUAL(prefixes.modeForPrefix('^'), '\0');
	T_ASSERT_EQUAL(prefixes.modeForPrefix('+'), 'v');
	T_ASSERT_EQUAL(prefixes.prefixForMode('o'), '@');
	T_ASSERT_EQUAL(prefixes.prefixForMode('h'), '%');
	T_ASSERT_EQUAL(prefixes.prefixForMode('a'), '\0');
	T_ASSERT_EQUAL(prefixes.prefixForMode('v'), '+');
	return true;
}
