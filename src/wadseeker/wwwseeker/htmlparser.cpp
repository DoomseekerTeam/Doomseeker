//------------------------------------------------------------------------------
// htmlparser.cpp
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
// Copyright (C) 2009 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "htmlparser.h"

#include <cctype>
#include <QFileInfo>

HtmlParser::HtmlParser(const QByteArray &data)
{
	this->data = data;
	capitalizeHtmlTags();
}

void HtmlParser::capitalizeHtmlTags()
{
	int begin = 0;
	int end = 0;

	while (true)
	{
		char endingChar = ' ';
		bool bNext = false;
		bool bValueBegin = false;

		begin = findTag(end, &end);
		if (begin < 0 || end < 0)
			break;

		for (int i = begin; i < end; ++i)
		{
			if (bNext)
			{
				if (data[i] == endingChar)
				{
					bNext = false;
					bValueBegin = false;
				}
				continue;
			}
			else
			{
				if (bValueBegin)
				{
					if (data[i] == '\"')
					{
						bNext = true;
						endingChar = '\"';
					}
					else if (data[i] != ' ')
					{
						bNext = true;
						endingChar = ' ';
					}
				}
				else
				{
					if (data[i] == '=')
					{
						bValueBegin = true;
					}
					else
					{
						data[i] = toupper(data[i]);
					}
				} // end of else
			} // end of else
		} // end of for
	} // end of while
}

int HtmlParser::findTag(int beginAt, int *end)
{
	if (end == nullptr)
	{
		return -1;
	}

	int begin = -1;
	*end = -1;

	// Find first occurence of '<' character.
	for (int i = beginAt; i < data.length(); ++i)
	{
		if (data[i] == '<')
		{
			begin = i;
			break;
		}
	}

	if (begin < 0)
	{
		return -1;
	}

	// Find first occurence of '>' character that exists directly after
	// previously found '<' character.
	for (int i = begin; i < data.length(); ++i)
	{
		if (data[i] == '>')
		{
			*end = i;
			break;
		}
	}

	return begin;
}

QString HtmlParser::htmlValue(int beginIndex, int endIndex)
{
	const QByteArray &byte = this->data;
	int indexStartValue = -1;
	int indexEndValue = -1;

	for (; beginIndex < endIndex; ++beginIndex)
	{
		if (byte[beginIndex] == '=')
		{
			++beginIndex;
			break;
		}
	}

	char endingChar = ' ';
	for (int i = beginIndex; i < endIndex; ++i)
	{
		if (indexStartValue < 0)
		{
			if (byte[i] == '\"')
			{
				indexStartValue = i + 1;
				endingChar = '\"';
			}
			else if (byte[i] == ' ')
			{
				continue;
			}
			else
			{
				indexStartValue = i;
			}
		}
		else
		{
			if (byte[i] == endingChar )
			{
				indexEndValue = i - 1;
				break;
			}
		}
	}

	if (indexStartValue < 0)
		return QString();

	if (indexEndValue < 0)
	{
		if (byte[endIndex] == '>')
			indexEndValue = endIndex - 1;
		else
			return QString();
	}

	QString ret = byte.mid(indexStartValue, indexEndValue - indexStartValue + 1);
	return ret;
}

QString HtmlParser::htmlValue(const QString &key)
{
	const QByteArray &byte = this->data;
	QByteArray upperByte = byte.toUpper();
	QString upperKey = key.toUpper();

	int beginIndex = upperByte.indexOf(upperKey.toUtf8());

	if (beginIndex < 0)
		return QString();

	for (; beginIndex < byte.size(); ++beginIndex)
	{
		if (byte[beginIndex] == '=')
		{
			++beginIndex;
			break;
		}
	}

	int indexStartValue = -1;
	int indexEndValue = byte.size() - 1;

	char endingChar = ' ';
	for (int i = beginIndex; i < byte.size(); ++i)
	{
		if (indexStartValue < 0)
		{
			if (byte[i] == '\"')
			{
				indexStartValue = i + 1;
				endingChar = '\"';
			}
			else if (byte[i] == ' ')
			{
				continue;
			}
			else
			{
				indexStartValue = i;
			}
		}
		else
		{
			if (byte[i] == endingChar )
			{
				indexEndValue = i - 1;
				break;
			}
		}
	}

	if (indexStartValue < 0)
	{
		return QString();
	}

	QString ret = byte.mid(indexStartValue, indexEndValue - indexStartValue + 1);
	return ret;
}

QList<Link> HtmlParser::linksFromHtml()
{
	QList<Link> list;

	int indexBeginTag = 0;
	while (true)
	{
		int indexCloseBracket = 0;
		int indexHref = 0;

		// Find an occurence of "<A " character sequence,
		// if nothing found then return.
		indexBeginTag = data.indexOf("<A ", indexBeginTag);
		if (indexBeginTag < 0)
		{
			break;
		}

		// Find an occurence of ">" character starting
		// from index returned by 1, if nothing found then return.
		indexCloseBracket = data.indexOf(">", indexBeginTag);
		if (indexCloseBracket < 0)
		{
			break;
		}

		// Find an occurence of " HREF" character sequence
		// starting from index returned by previous steps.
		indexHref = data.indexOf(" HREF", indexBeginTag) + 1;

		// Check if HREF is inside the A tag and if there is ' ' or '='
		// character after " HREF"	if not go to first step starting
		// from index at the previously found '>' character.
		int strLength = QString("HREF").length();
		if (indexHref > indexCloseBracket || (data[indexHref + strLength] != ' ' && data[indexHref + strLength] != '='))
		{
			// next iteration
			indexBeginTag = indexCloseBracket;
		}
		else
		{
			// Get HREF value.
			QString url = htmlValue(indexHref, indexCloseBracket);

			// Find "</A>" character sequence starting from index at
			// the previously found '>' character. If nothing found then return.
			int indexEndA = 0;

			indexEndA = data.indexOf("</A>", indexCloseBracket);
			if (indexEndA < 0)
			{
				break;
			}

			// Get all text between '>' and "</A>".
			QString text = data.mid(indexCloseBracket + 1, indexEndA - (indexCloseBracket + 1));

			// Append Link object to list.
			Link link(url, text);
			list.append(link);

			// Begin searching for the next URL.
			indexBeginTag = indexEndA;
		}
	}

	return list;
}
