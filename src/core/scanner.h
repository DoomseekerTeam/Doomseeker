//------------------------------------------------------------------------------
// scanner.hpp
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
// Copyright (C) 2010 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------

#ifndef __SCANNER_HPP__
#define __SCANNER_HPP__

#include <QObject>
#include <QString>

#include "dptr.h"
#include "global.h"

enum ETokenType
{
	TK_Identifier, // Ex: SomeIdentifier
	TK_StringConst, // Ex: "Some String"
	TK_IntConst, // Ex: 27
	TK_FloatConst, // Ex: 1.5
	TK_BoolConst, // Ex: true
	TK_AndAnd, // &&
	TK_OrOr, // ||
	TK_EqEq, // ==
	TK_NotEq, // !=
	TK_GtrEq, // >=
	TK_LessEq, // <=
	TK_ShiftLeft, // <<
	TK_ShiftRight, // >>
	TK_Increment, // ++
	TK_Decrement, // --
	TK_PointerMember, // ->
	TK_ScopeResolution, // ::
	TK_MacroConcat, // ##
	TK_AddEq, // +=
	TK_SubEq, // -=
	TK_MulEq, // *=
	TK_DivEq, // /=
	TK_ModEq, // %=
	TK_ShiftLeftEq, // <<=
	TK_ShiftRightEq, // >>=
	TK_AndEq, // &=
	TK_OrEq, // |=
	TK_XorEq, // ^=
	TK_Ellipsis, // ...

	TK_NumSpecialTokens,

	TK_NoToken = -1
};

/**
 * @ingroup group_pluginapi
 * @brief Scanner reads scripts by checking individual tokens.
 * @author Blzut3
 */
class MAIN_EXPORT Scanner
{
private:
	Scanner(const Scanner &other);

public:
	class ParserState
	{
	public:
		ParserState();
		virtual ~ParserState();

		const QString &str() const;
		void setStr(const QString &v);

		unsigned int number() const;
		void setNumber(unsigned int v);

		double decimal() const;
		void setDecimal(double v);

		bool boolean() const;
		void setBoolean(bool v);

		char token() const;
		void setToken(char v);

		unsigned int tokenLine() const;
		void setTokenLine(unsigned int v);

		unsigned int tokenLinePosition() const;
		void setTokenLinePosition(unsigned int v);

		unsigned int scanPos() const;
		void setScanPos(unsigned int v);

	private:
		DPtr<ParserState> d;
	};

	enum MessageLevel
	{
		ML_ERROR,
		ML_WARNING,
		ML_NOTICE
	};

	Scanner(const char *data, int length = -1);
	virtual ~Scanner();

	void checkForMeta();
	/**
	 * Moves the position ahead any whitespace that there might be from the
	 * current position.
	 */
	void checkForWhitespace();
	/**
	 * This function will scan the next token and return if it is of the
	 * specified type.  If it returns true, then the position will be moved
	 * to after the token, otherwise it will be reset.
	 */
	bool checkToken(char token);
	/**
	 * Transfers nextState over for use.
	 */
	void expandState();
	int currentLine() const;
	int currentLinePos() const;
	int currentPos() const;
	unsigned int currentScanPos() const;
	bool nextString();
	/**
	 * Gets whatever token is next returning true on success.
	 * @param expandState Used by checkToken, leave as true.
	 */
	bool nextToken(bool autoExpandState = true);
	void mustGetToken(unsigned char token);
	void rewind(); /// Only can rewind one step.
	const char *scriptData() const;
	void scriptMessage(MessageLevel level, const char *error, ...) const;
	void setScriptIdentifier(const QString &ident);
	int skipLine();
	ParserState &state();
	const ParserState &state() const;

	/**
	 * Returns true if there is still more to read.
	 */
	bool tokensLeft() const;

	const ParserState &operator*() const
	{
		return state();
	}
	const ParserState *operator->() const
	{
		return &state();
	}

	static const QString &escape(QString &str);
	static const QString &unescape(QString &str);

	static void setMessageHandler(void (*handler)(MessageLevel, const char *, va_list))
	{
		messageHandler = handler;
	}

protected:
	/**
	 * Changes the values in line and lineStart, does not change the actual
	 * scanning position in the file.
	 */
	void incrementLine();

private:
	DPtr<Scanner> d;

	static void (*messageHandler)(MessageLevel, const char *, va_list);
};

#endif /* __SCANNER_HPP__ */
