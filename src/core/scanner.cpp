//------------------------------------------------------------------------------
// scanner.cpp
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

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "scanner.h"

DClass<Scanner>
{
public:
	Scanner::ParserState nextState, prevState, state;

	char *data;
	unsigned int length;

	unsigned int line;
	unsigned int lineStart;
	unsigned int logicalPosition;
	unsigned int scanPos;

	bool needNext; // If checkToken returns false this will be false.

	QString scriptIdentifier;
};

DClass<Scanner::ParserState>
{
public:
	QString str;
	unsigned int number;
	double decimal;
	bool boolean;
	char token;
	unsigned int tokenLine;
	unsigned int tokenLinePosition;
	unsigned int scanPos;
};

DPointered(Scanner::ParserState)
DPointered(Scanner)

void (*Scanner::messageHandler)(MessageLevel, const char *, va_list) = nullptr;

static const char *const TokenNames[TK_NumSpecialTokens] =
{
	"Identifier",
	"String Constant",
	"Integer Constant",
	"Float Constant",
	"Boolean Constant",
	"Logical And",
	"Logical Or",
	"Equals",
	"Not Equals",
	"Greater Than or Equals"
	"Less Than or Equals",
	"Left Shift",
	"Right Shift",
	"Increment",
	"Decrement",
	"Pointer Member",
	"Scope Resolution",
	"Macro Concatenation",
	"Assign Sum",
	"Assign Difference",
	"Assign Product",
	"Assign Quotient",
	"Assign Modulus",
	"Assign Left Shift",
	"Assign Right Shift",
	"Assign Bitwise And",
	"Assign Bitwise Or",
	"Assign Exclusive Or",
	"Ellipsis"
};

////////////////////////////////////////////////////////////////////////////////

Scanner::Scanner(const char *data, int length)
{
	d->line = 1;
	d->lineStart = 0;
	d->logicalPosition = 0;
	d->scanPos = 0;
	d->needNext = true;
	if (length == -1)
		length = strlen(data);
	d->length = length;
	d->data = new char[length];
	memcpy(d->data, data, length);

	checkForWhitespace();

	d->state.setScanPos(d->scanPos);
}

Scanner::~Scanner()
{
	delete[] d->data;
}

// Here's my answer to the preprocessor screwing up line numbers. What we do is
// after a new line in CheckForWhitespace, look for a comment in the form of
// "/*meta:filename:line*/"
void Scanner::checkForMeta()
{
	if (d->scanPos + 10 < d->length)
	{
		char metaCheck[8];
		memcpy(metaCheck, d->data + d->scanPos, 7);
		metaCheck[7] = 0;
		if (strcmp(metaCheck, "/*meta:") == 0)
		{
			d->scanPos += 7;
			int metaStart = d->scanPos;
			int fileLength = 0;
			int lineLength = 0;
			while (d->scanPos < d->length)
			{
				char thisChar = d->data[d->scanPos];
				char nextChar = d->scanPos + 1 < d->length ? d->data[d->scanPos + 1] : 0;
				if (thisChar == '*' && nextChar == '/')
				{
					lineLength = d->scanPos - metaStart - 1 - fileLength;
					d->scanPos += 2;
					break;
				}
				if (thisChar == ':' && fileLength == 0)
					fileLength = d->scanPos - metaStart;
				d->scanPos++;
			}
			if (fileLength > 0 && lineLength > 0)
			{
				setScriptIdentifier(QString::fromUtf8(d->data + metaStart, fileLength));
				QString lineNumber = QString::fromUtf8(d->data + metaStart + fileLength + 1, lineLength);
				d->line = atoi(lineNumber.toUtf8().constData());
				d->lineStart = d->scanPos;
			}
		}
	}
}

void Scanner::checkForWhitespace()
{
	int comment = 0; // 1 = till next new line, 2 = till end block
	while (d->scanPos < d->length)
	{
		char cur = d->data[d->scanPos];
		char next = d->scanPos + 1 < d->length ? d->data[d->scanPos + 1] : 0;
		if (comment == 2)
		{
			if (cur != '*' || next != '/')
			{
				if (cur == '\n' || cur == '\r')
				{
					d->scanPos++;
					if (comment == 1)
						comment = 0;

					// Do a quick check for Windows style new line
					if (cur == '\r' && next == '\n')
						d->scanPos++;
					incrementLine();
				}
				else
					d->scanPos++;
			}
			else
			{
				comment = 0;
				d->scanPos += 2;
			}
			continue;
		}

		if (cur == ' ' || cur == '\t' || cur == 0)
			d->scanPos++;
		else if (cur == '\n' || cur == '\r')
		{
			d->scanPos++;
			if (comment == 1)
				comment = 0;

			// Do a quick check for Windows style new line
			if (cur == '\r' && next == '\n')
				d->scanPos++;
			incrementLine();
			checkForMeta();
		}
		else if (cur == '/' && comment == 0)
		{
			switch (next)
			{
			case '/':
				comment = 1;
				break;
			case '*':
				comment = 2;
				break;
			default:
				return;
			}
			d->scanPos += 2;
		}
		else
		{
			if (comment == 0)
				return;
			else
				d->scanPos++;
		}
	}
}

bool Scanner::checkToken(char token)
{
	if (d->needNext)
	{
		if (!nextToken(false))
			return false;
	}

	// An int can also be a float.
	if (d->nextState.token() == token || (d->nextState.token() == TK_IntConst && token == TK_FloatConst))
	{
		d->needNext = true;
		expandState();
		return true;
	}
	d->needNext = false;
	return false;
}

int Scanner::currentLine() const
{
	return d->state.tokenLine();
}

int Scanner::currentLinePos() const
{
	return d->state.tokenLinePosition();
}

int Scanner::currentPos() const
{
	return d->logicalPosition;
}

unsigned int Scanner::currentScanPos() const
{
	return d->scanPos;
}

void Scanner::expandState()
{
	d->scanPos = d->nextState.scanPos();
	d->logicalPosition = d->scanPos;
	checkForWhitespace();

	d->prevState = d->state;
	d->state = d->nextState;
}

void Scanner::incrementLine()
{
	d->line++;
	d->lineStart = d->scanPos;
}

bool Scanner::nextString()
{
	d->nextState.setTokenLine(d->line);
	d->nextState.setTokenLinePosition(d->scanPos - d->lineStart);
	d->nextState.setToken(TK_NoToken);
	if (!d->needNext)
		d->scanPos = d->state.scanPos();
	checkForWhitespace();
	if (d->scanPos >= d->length)
		return false;

	int start = d->scanPos;
	int end = d->scanPos;
	bool quoted = d->data[d->scanPos] == '"';
	if (quoted) // String Constant
	{
		end = ++start; // Remove starting quote
		d->scanPos++;
		while (d->scanPos < d->length)
		{
			char cur = d->data[d->scanPos];
			if (cur == '"')
				end = d->scanPos;
			else if (cur == '\\')
			{
				d->scanPos += 2;
				continue;
			}
			d->scanPos++;
			if (start != end)
				break;
		}
	}
	else // Unquoted string
	{
		while (d->scanPos < d->length)
		{
			char cur = d->data[d->scanPos];
			switch (cur)
			{
			default:
				break;
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				end = d->scanPos;
				break;
			}
			if (start != end)
				break;
			d->scanPos++;
		}
		if (d->scanPos == d->length)
			end = d->scanPos;
	}
	if (end - start > 0)
	{
		d->nextState.setScanPos(d->scanPos);
		QString thisString = QString::fromUtf8(d->data + start, end - start);
		if (quoted)
			unescape(thisString);
		d->nextState.setStr(thisString);
		d->nextState.setToken(TK_StringConst);
		expandState();
		d->needNext = true;
		return true;
	}
	checkForWhitespace();
	return false;
}

bool Scanner::nextToken(bool autoExpandState)
{
	if (!d->needNext)
	{
		d->needNext = true;
		if (autoExpandState)
			expandState();
		return true;
	}

	d->nextState.setTokenLine(d->line);
	d->nextState.setTokenLinePosition(d->scanPos - d->lineStart);
	d->nextState.setToken(TK_NoToken);
	if (d->scanPos >= d->length)
	{
		if (autoExpandState)
			expandState();
		return false;
	}

	unsigned int start = d->scanPos;
	unsigned int end = d->scanPos;
	int integerBase = 10;
	bool floatHasDecimal = false;
	bool floatHasExponent = false;
	bool stringFinished = false; // Strings are the only things that can have 0 length tokens.

	char cur = d->data[d->scanPos++];
	// Determine by first character
	if (cur == '_' || (cur >= 'A' && cur <= 'Z') || (cur >= 'a' && cur <= 'z'))
		d->nextState.setToken(TK_Identifier);
	else if (cur >= '0' && cur <= '9')
	{
		if (cur == '0')
			integerBase = 8;
		d->nextState.setToken(TK_IntConst);
	}
	else if (cur == '.' && d->scanPos < d->length && d->data[d->scanPos] != '.')
	{
		floatHasDecimal = true;
		d->nextState.setToken(TK_FloatConst);
	}
	else if (cur == '"')
	{
		end = ++start; // Move the start up one character so we don't have to trim it later.
		d->nextState.setToken(TK_StringConst);
	}
	else
	{
		end = d->scanPos;
		d->nextState.setToken(cur);

		// Now check for operator tokens
		if (d->scanPos < d->length)
		{
			char next = d->data[d->scanPos];
			if (cur == '&' && next == '&')
				d->nextState.setToken(TK_AndAnd);
			else if (cur == '|' && next == '|')
				d->nextState.setToken(TK_OrOr);
			else if (
				(cur == '<' && next == '<') ||
				(cur == '>' && next == '>')
			)
			{
				// Next for 3 character tokens
				if (d->scanPos + 1 > d->length && d->data[d->scanPos + 1] == '=')
				{
					d->scanPos++;
					d->nextState.setToken(cur == '<' ? TK_ShiftLeftEq : TK_ShiftRightEq);
				}
				else
					d->nextState.setToken(cur == '<' ? TK_ShiftLeft : TK_ShiftRight);
			}
			else if (cur == '#' && next == '#')
				d->nextState.setToken(TK_MacroConcat);
			else if (cur == ':' && next == ':')
				d->nextState.setToken(TK_ScopeResolution);
			else if (cur == '+' && next == '+')
				d->nextState.setToken(TK_Increment);
			else if (cur == '-')
			{
				if (next == '-')
					d->nextState.setToken(TK_Decrement);
				else if (next == '>')
					d->nextState.setToken(TK_PointerMember);
			}
			else if (cur == '.' && next == '.' &&
				d->scanPos + 1 < d->length && d->data[d->scanPos + 1] == '.')
			{
				d->nextState.setToken(TK_Ellipsis);
				++d->scanPos;
			}
			else if (next == '=')
			{
				switch (cur)
				{
				case '=':
					d->nextState.setToken(TK_EqEq);
					break;
				case '!':
					d->nextState.setToken(TK_NotEq);
					break;
				case '>':
					d->nextState.setToken(TK_GtrEq);
					break;
				case '<':
					d->nextState.setToken(TK_LessEq);
					break;
				case '+':
					d->nextState.setToken(TK_AddEq);
					break;
				case '-':
					d->nextState.setToken(TK_SubEq);
					break;
				case '*':
					d->nextState.setToken(TK_MulEq);
					break;
				case '/':
					d->nextState.setToken(TK_DivEq);
					break;
				case '%':
					d->nextState.setToken(TK_ModEq);
					break;
				case '&':
					d->nextState.setToken(TK_AndEq);
					break;
				case '|':
					d->nextState.setToken(TK_OrEq);
					break;
				case '^':
					d->nextState.setToken(TK_XorEq);
					break;
				default:
					break;
				}
			}

			if (d->nextState.token() != cur)
			{
				d->scanPos++;
				end = d->scanPos;
			}
		}
	}

	if (start == end)
	{
		while (d->scanPos < d->length)
		{
			cur = d->data[d->scanPos];
			switch (d->nextState.token())
			{
			default:
				break;
			case TK_Identifier:
				if (cur != '_' && (cur < 'A' || cur > 'Z') && (cur < 'a' || cur > 'z') && (cur < '0' || cur > '9'))
					end = d->scanPos;
				break;
			case TK_IntConst:
				if (cur == '.' || (d->scanPos - 1 != start && cur == 'e'))
					d->nextState.setToken(TK_FloatConst);
				else if ((cur == 'x' || cur == 'X') && d->scanPos - 1 == start)
				{
					integerBase = 16;
					break;
				}
				else
				{
					switch (integerBase)
					{
					default:
						if (cur < '0' || cur > '9')
							end = d->scanPos;
						break;
					case 8:
						if (cur < '0' || cur > '7')
							end = d->scanPos;
						break;
					case 16:
						if ((cur < '0' || cur > '9') && (cur < 'A' || cur > 'F') && (cur < 'a' || cur > 'f'))
							end = d->scanPos;
						break;
					}
					break;
				}
				[[gnu::fallthrough]];
			case TK_FloatConst:
				if (cur < '0' || cur > '9')
				{
					if (!floatHasDecimal && cur == '.')
					{
						floatHasDecimal = true;
						break;
					}
					else if (!floatHasExponent && cur == 'e')
					{
						floatHasDecimal = true;
						floatHasExponent = true;
						if (d->scanPos + 1 < d->length)
						{
							char next = d->data[d->scanPos + 1];
							if ((next < '0' || next > '9') && next != '+' && next != '-')
								end = d->scanPos;
							else
								d->scanPos++;
						}
						break;
					}
					end = d->scanPos;
				}
				break;
			case TK_StringConst:
				if (cur == '"')
				{
					stringFinished = true;
					end = d->scanPos;
					d->scanPos++;
				}
				else if (cur == '\\')
					d->scanPos++; // Will add two since the loop automatically adds one
				break;
			}
			if (start == end && !stringFinished)
				d->scanPos++;
			else
				break;
		}
		// Handle small tokens at the end of a file.
		if (d->scanPos == d->length && !stringFinished)
			end = d->scanPos;
	}

	d->nextState.setScanPos(d->scanPos);
	if (end - start > 0 || stringFinished)
	{
		d->nextState.setStr(QByteArray(d->data + start, end - start));
		if (d->nextState.token() == TK_FloatConst)
		{
			if (floatHasDecimal && d->nextState.str().length() == 1)
			{
				// Don't treat a lone '.' as a decimal.
				d->nextState.setToken('.');
			}
			else
			{
				d->nextState.setDecimal(d->nextState.str().toDouble(nullptr));
				d->nextState.setNumber(static_cast<int>(d->nextState.decimal()));
				d->nextState.setBoolean(d->nextState.number() != 0);
			}
		}
		else if (d->nextState.token() == TK_IntConst)
		{
			d->nextState.setNumber(d->nextState.str().toUInt(nullptr, integerBase));
			d->nextState.setDecimal(d->nextState.number());
			d->nextState.setBoolean(d->nextState.number() != 0);
		}
		else if (d->nextState.token() == TK_Identifier)
		{
			// Check for a boolean constant.
			if (d->nextState.str().compare("true") == 0)
			{
				d->nextState.setToken(TK_BoolConst);
				d->nextState.setBoolean(true);
			}
			else if (d->nextState.str().compare("false") == 0)
			{
				d->nextState.setToken(TK_BoolConst);
				d->nextState.setBoolean(false);
			}
		}
		else if (d->nextState.token() == TK_StringConst)
		{
			QString str = d->nextState.str();
			d->nextState.setStr(unescape(str));
		}
		if (autoExpandState)
			expandState();
		return true;
	}
	d->nextState.setToken(TK_NoToken);
	if (autoExpandState)
		expandState();
	return false;
}

void Scanner::mustGetToken(unsigned char token)
{
	if (!checkToken(token))
	{
		expandState();
		if (token < TK_NumSpecialTokens && d->state.token() < TK_NumSpecialTokens)
			scriptMessage(Scanner::ML_ERROR, "Expected '%s' but got '%s' instead.", TokenNames[token], TokenNames[static_cast<unsigned>(d->state.token())]);
		else if (token < TK_NumSpecialTokens && d->state.token() >= TK_NumSpecialTokens)
			scriptMessage(Scanner::ML_ERROR, "Expected '%s' but got '%c' instead.", TokenNames[token], d->state.token());
		else if (token >= TK_NumSpecialTokens && d->state.token() < TK_NumSpecialTokens)
			scriptMessage(Scanner::ML_ERROR, "Expected '%c' but got '%s' instead.", token, TokenNames[static_cast<unsigned>(d->state.token())]);
		else
			scriptMessage(Scanner::ML_ERROR, "Expected '%c' but got '%c' instead.", token, d->state.token());
	}
}

void Scanner::rewind()
{
	d->needNext = false;
	d->nextState = d->state;
	d->state = d->prevState;
	d->scanPos = d->state.scanPos();

	d->line = d->prevState.tokenLine();
	d->logicalPosition = d->prevState.tokenLinePosition();
}

const char *Scanner::scriptData() const
{
	return d->data;
}

void Scanner::scriptMessage(MessageLevel level, const char *error, ...) const
{
	const char *messageLevel;
	switch (level)
	{
	default:
		messageLevel = "Notice";
		break;
	case ML_WARNING:
		messageLevel = "Warning";
		break;
	case ML_ERROR:
		messageLevel = "Error";
		break;
	}

	char *newMessage = new char[strlen(error) + d->scriptIdentifier.length() + 25];
	sprintf(newMessage, "%s:%d:%d:%s: %s\n", d->scriptIdentifier.toUtf8().constData(), currentLine(), currentLinePos(), messageLevel, error);
	va_list list;
	va_start(list, error);
	if (messageHandler)
		messageHandler(level, newMessage, list);
	else
		vfprintf(stderr, newMessage, list);
	va_end(list);
	delete[] newMessage;

	if (!messageHandler && level == ML_ERROR)
		exit(0);
}

void Scanner::setScriptIdentifier(const QString &ident)
{
	d->scriptIdentifier = ident;
}

int Scanner::skipLine()
{
	int ret = currentPos();
	while (d->logicalPosition < d->length)
	{
		char thisChar = d->data[d->logicalPosition];
		char nextChar = d->logicalPosition + 1 < d->length ? d->data[d->logicalPosition + 1] : 0;
		if (thisChar == '\n' || thisChar == '\r')
		{
			ret = d->logicalPosition++; // Return the first newline character we see.
			if (nextChar == '\r')
				d->logicalPosition++;
			incrementLine();
			checkForWhitespace();
			break;
		}
		d->logicalPosition++;
	}
	if (d->logicalPosition > d->scanPos)
	{
		d->scanPos = d->logicalPosition;
		checkForWhitespace();
		d->needNext = true;
		d->logicalPosition = d->scanPos;
	}
	return ret;
}

Scanner::ParserState &Scanner::state()
{
	return d->state;
}

const Scanner::ParserState &Scanner::state() const
{
	return d->state;
}

bool Scanner::tokensLeft() const
{
	return d->scanPos < d->length;
}

////////////////////////////////////////////////////////////////////////////////
// NOTE: Be sure that '\\' is the first thing in the array otherwise it will re-escape.
static char escapeCharacters[] = {'\\', '"', 0};
const QString &Scanner::escape(QString &str)
{
	for (unsigned int i = 0; escapeCharacters[i] != 0; i++)
	{
		// += 2 because we'll be inserting 1 character.
		for (int p = 0; p < str.length() && (p = str.indexOf(escapeCharacters[i], p)) != -1; p += 2)
		{
			str.insert(p, '\\');
		}
	}
	return str;
}
const QString &Scanner::unescape(QString &str)
{
	for (unsigned int i = 0; escapeCharacters[i] != 0; i++)
	{
		QString sequence = "\\" + QString(escapeCharacters[i]);
		for (int p = 0; p < str.length() && (p = str.indexOf(sequence, p)) != -1; p++)
			str.replace(str.indexOf(sequence, p), 2, escapeCharacters[i]);
	}
	return str;
}
////////////////////////////////////////////////////////////////////////////////

Scanner::ParserState::ParserState()
{
}

Scanner::ParserState::~ParserState()
{
}

const QString &Scanner::ParserState::str() const
{
	return d->str;
}

void Scanner::ParserState::setStr(const QString &v)
{
	d->str = v;
}

unsigned int Scanner::ParserState::number() const
{
	return d->number;
}

void Scanner::ParserState::setNumber(unsigned int v)
{
	d->number = v;
}

double Scanner::ParserState::decimal() const
{
	return d->decimal;
}

void Scanner::ParserState::setDecimal(double v)
{
	d->decimal = v;
}

bool Scanner::ParserState::boolean() const
{
	return d->boolean;
}

void Scanner::ParserState::setBoolean(bool v)
{
	d->boolean = v;
}

char Scanner::ParserState::token() const
{
	return d->token;
}

void Scanner::ParserState::setToken(char v)
{
	d->token = v;
}

unsigned int Scanner::ParserState::tokenLine() const
{
	return d->tokenLine;
}

void Scanner::ParserState::setTokenLine(unsigned int v)
{
	d->tokenLine = v;
}

unsigned int Scanner::ParserState::tokenLinePosition() const
{
	return d->tokenLinePosition;
}

void Scanner::ParserState::setTokenLinePosition(unsigned int v)
{
	d->tokenLinePosition = v;
}

unsigned int Scanner::ParserState::scanPos() const
{
	return d->scanPos;
}

void Scanner::ParserState::setScanPos(unsigned int v)
{
	d->scanPos = v;
}
