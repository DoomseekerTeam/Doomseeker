/****************************************************************************
**
** Copyright (C) 2020 Giuseppe D'Angelo <dangelog@gmail.com>.
** Copyright (C) 2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

// Copy of QRegularExpression::wildcardToRegularExpression from Qt 5.15 for use
// with Qt < 5.12

#include "pattern.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
static inline QString anchoredPattern(const QString &expression)
{
    return QLatin1String("\\A(?:")
           + expression
           + QLatin1String(")\\z");
}

/*!
    \since 5.12

    Returns a regular expression representation of the given glob \a pattern.
    The transformation is targeting file path globbing, which means in particular
    that path separators receive special treatment. This implies that it is not
    just a basic translation from "*" to ".*".

    \snippet code/src_corelib_tools_qregularexpression.cpp 31

    The returned regular expression is already fully anchored. In other
    words, there is no need of calling anchoredPattern() again on the
    result.

    \warning Unlike QRegExp, this implementation follows closely the definition
    of wildcard for glob patterns:
    \table
    \row \li \b{c}
         \li Any character represents itself apart from those mentioned
         below. Thus \b{c} matches the character \e c.
    \row \li \b{?}
         \li Matches any single character. It is the same as
         \b{.} in full regexps.
    \row \li \b{*}
         \li Matches zero or more of any characters. It is the
         same as \b{.*} in full regexps.
    \row \li \b{[abc]}
         \li Matches one character given in the bracket.
    \row \li \b{[a-c]}
         \li Matches one character from the range given in the bracket.
    \row \li \b{[!abc]}
         \li Matches one character that is not given in the bracket. It is the
         same as \b{[^abc]} in full regexp.
    \row \li \b{[!a-c]}
         \li Matches one character that is not from the range given in the
         bracket. It is the same as \b{[^a-c]} in full regexp.
    \endtable

    \note The backslash (\\) character is \e not an escape char in this context.
    In order to match one of the special characters, place it in square brackets
    (for example, \c{[?]}).

    More information about the implementation can be found in:
    \list
    \li \l {https://en.wikipedia.org/wiki/Glob_(programming)} {The Wikipedia Glob article}
    \li \c {man 7 glob}
    \endlist

    \sa escape()
*/
QString Pattern::wildcardToRegularExpression(const QString &pattern)
{
    const int wclen = pattern.length();
    QString rx;
    rx.reserve(wclen + wclen / 16);
    int i = 0;
    const QChar *wc = pattern.data();

#ifdef Q_OS_WIN
    const QLatin1Char nativePathSeparator('\\');
    const QLatin1String starEscape("[^/\\\\]*");
    const QLatin1String questionMarkEscape("[^/\\\\]");
#else
    const QLatin1Char nativePathSeparator('/');
    const QLatin1String starEscape("[^/]*");
    const QLatin1String questionMarkEscape("[^/]");
#endif

    while (i < wclen) {
        const QChar c = wc[i++];
        switch (c.unicode()) {
        case '*':
            rx += starEscape;
            break;
        case '?':
            rx += questionMarkEscape;
            break;
        case '\\':
#ifdef Q_OS_WIN
        case '/':
            rx += QLatin1String("[/\\\\]");
            break;
#endif
        case '$':
        case '(':
        case ')':
        case '+':
        case '.':
        case '^':
        case '{':
        case '|':
        case '}':
            rx += QLatin1Char('\\');
            rx += c;
            break;
        case '[':
            rx += c;
            // Support for the [!abc] or [!a-c] syntax
            if (i < wclen) {
                if (wc[i] == QLatin1Char('!')) {
                    rx += QLatin1Char('^');
                    ++i;
                }

                if (i < wclen && wc[i] == QLatin1Char(']'))
                    rx += wc[i++];

                while (i < wclen && wc[i] != QLatin1Char(']')) {
                    // The '/' appearing in a character class invalidates the
                    // regular expression parsing. It also concerns '\\' on
                    // Windows OS types.
                    if (wc[i] == QLatin1Char('/') || wc[i] == nativePathSeparator)
                        return rx;
                    if (wc[i] == QLatin1Char('\\'))
                        rx += QLatin1Char('\\');
                    rx += wc[i++];
                }
            }
            break;
        default:
            rx += c;
            break;
        }
    }

    return anchoredPattern(rx);
}
#endif
