//------------------------------------------------------------------------------
// message.h
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
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __DS_MESSAGE_H_
#define __DS_MESSAGE_H_

#include "dptr.h"
#include "global.h"
#include <QDateTime>
#include <QMetaType>
#include <QObject>
#include <QString>

class StaticMessages : public QObject
{
	Q_OBJECT

public:
	/**
	 * @brief Gets static string for a Message::Type.
	 *
	 * @param messageType
	 *     One of constant values defined in Message::Type.
	 */
	static QString getMessage(unsigned messageType);
};

/**
 * @ingroup group_pluginapi
 * @brief Message object used to pass messages throughout the Doomseeker's
 *        system.
 *
 * Message type and content is carried over through this object. If type
 * member is set to Message::Type::IGNORE_TYPE then content member should
 * be ignored and the entire Message object should be treated as carrying
 * no message at all.
 *
 * Messages can be displayed by the application through a pop-up box or in
 * the program log. This may differ depending on message severity and current
 * procedure.
 *
 * Message objects are safe to copy.
 */
class MAIN_EXPORT Message
{
public:
	/**
	 * @brief One of possible, Doomseeker recognizable error types.
	 *
	 * Doomseeker may take certain actions depending on the type of the
	 * Message. This is largely dependent on the situation. However,
	 * in general, IGNORE_TYPE messages are... ignored silently, while
	 * all error type messages are displayed in one manner or another.
	 * Methods that use Message objects describe any behavior that
	 * differs from standard or might be unexpected.
	 */
	class Type
	{
	public:
		/**
		 * @brief 'Null' Message object; ignore it.
		 */
		static const unsigned IGNORE_TYPE = 0;

		/**
		 * @brief Programmer-defined information message, not an error.
		 *
		 * This is used for display only and no parsing other than
		 * severity determination is performed.
		 */
		static const unsigned CUSTOM_INFORMATION = 1;
		/**
		 * @brief Message indicates that the operation was cancelled.
		 */
		static const unsigned CANCELLED = 2;
		/**
		 * @brief Message indicates that the operation was successful.
		 */
		static const unsigned SUCCESSFUL = 3;

		/**
		 * @brief Programmer-defined error message.
		 *
		 * This is used for display and may interrupt any affected
		 * operation.
		 */
		static const unsigned CUSTOM_ERROR = 0x7fffffff;
		/**
		 * @brief Information indicating that current player is banned
		 *        from given server.
		 */
		static const unsigned BANNED_FROM_MASTERSERVER = CUSTOM_ERROR + 1;
		/**
		 * @brief Indicates that program executable was not found, but
		 *        Doomseeker or plugin are capable of performing
		 *        the installation.
		 */
		static const unsigned GAME_NOT_FOUND_BUT_CAN_BE_INSTALLED = CUSTOM_ERROR + 2;
	};

	/**
	 * @brief Convenience method. Sets type to Type::CUSTOM_ERROR.
	 */
	static Message customError(const QString &content)
	{
		return Message(Type::CUSTOM_ERROR, content);
	}

	/**
	 * @brief Convenience method. Sets type to Type::CUSTOM_INFORMATION.
	 */
	static Message customInformation(const QString &content)
	{
		return Message(Type::CUSTOM_INFORMATION, content);
	}

	/**
	 * @brief 'Null' message object, returns true on isIgnore().
	 */
	Message();
	/**
	 * @brief Message with defined type but carrying nothing to display.
	 *
	 * Sometimes Doomseeker may override and display a hard-coded message
	 * anyway, depending on current procedure and message type.
	 */
	Message(unsigned type);
	/**
	 * @brief Message with defined type and custom display content.
	 *
	 * Sometimes Doomseeker may override and display a hard-coded message
	 * anyway, depending on current procedure and message type.
	 */
	Message(unsigned type, const QString &content);
	Message(const Message &other);
	Message &operator=(const Message &other);
	virtual ~Message();

	/**
	 * @brief Customized displayable contents of this Message.
	 */
	QString contents() const;

	/**
	 * @brief True if type() equals to CUSTOM_INFORMATION or CUSTOM_ERROR.
	 */
	bool isCustom() const;
	/**
	 * @brief True if type() is equal to or greater than CUSTOM_ERROR.
	 */
	bool isError() const;
	/**
	 * @brief True for 'Null' messages.
	 */
	bool isIgnore() const;
	/**
	 * @brief True if type() is equal to or greater than CUSTOM_INFORMATION,
	 *        and lesser than CUSTOM_ERROR.
	 */
	bool isInformation() const;

	/**
	 * @brief Generation time in seconds since UTC epoch.
	 */
	unsigned timestamp() const;
	/**
	 * @brief Message::Type.
	 */
	unsigned type() const;

private:
	DPtr<Message> d;

	void construct();
};

Q_DECLARE_METATYPE(Message)

#endif
