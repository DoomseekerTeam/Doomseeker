//------------------------------------------------------------------------------
// serverstructs.h
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
#ifndef __SERVER_STRUCTS_H_
#define __SERVER_STRUCTS_H_

#include <climits>
#include <QCryptographicHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QVariant>

#include "dptr.h"
#include "global.h"

class Checksum;
class ModFile;
/**
 * \ingroup group_pluginapi
 * \file serverstructs.h
 */

/**
 * @ingroup group_pluginapi
 * @brief A game setting that is a part of a group of settings
 *        that can be OR'ed logically as a single integer.
 *
 * DMFlag is basically a setting that is toggleable between enabled/disabled
 * state, and a bunch of DMFlags can be represented as bitflags that can be
 * OR'ed, XOR'ed, and AND'ed together.
 *
 * DMFlag objects can be put into DMFlagsSection collection.
 *
 * DMFlag is bound by the same human-readable name and 'internal' name rules as
 * the DMFlagsSection class. Refer to the documentation of that class to see
 * the details. Remember that it's absolutely forbidden to use QObject::tr() in
 * the 'internal' name. Each DMFlag should have an unique name within
 * its DMFlagsSection.
 *
 * This structure is safe to copy.
 */
class MAIN_EXPORT DMFlag
{
public:
	DMFlag();
	DMFlag(const QString &internalName, unsigned value);
	DMFlag(const QString &internalName, unsigned value, const QString &name);
	DMFlag(const DMFlag &other);
	virtual ~DMFlag();
	DMFlag &operator=(const DMFlag &other);

	/**
	 * @brief Uniquely identifiable name within its DMFlagsSection,
	 *        ex. "Jump is allowed" or "jumpisallowed".
	 *
	 * @see DMFlagsSection::internalName().
	 */
	const QString &internalName() const;

	/**
	 * @brief Valid objects have value() greater than zero.
	 *
	 * Invalid objects can be treated as 'Null'.
	 */
	bool isValid() const;

	/**
	 * @brief User-displayable, translateable name of the DMFlag,
	 *        ex. "Jump is allowed".
	 */
	const QString &name() const;

	/**
	 * @brief Bits that represent this flag (usually just a single '1' bit).
	 */
	unsigned value() const;

private:
	DPtr<DMFlag> d;
};

/**
 * @ingroup group_pluginapi
 * @brief A group of DMFlag objects that can be safely OR'ed
 *        together to form a meaningful value.
 *
 * This object is safe to copy. If you need to clone the section but omit
 * the DMFlag definitions stored within it, use copyEmpty() method instead.
 *
 * DMFlagsSection uses two names - name() for human-readable purposes and
 * internalName() for identification within the system. name() should normally
 * be wrapped in a QObject::tr() call. It's also allowed to only specify the
 * 'internal' name and the human-readable name will be the same. The 'internal'
 * name is expected to stay the same between versions of Doomseeker as it will
 * be used in configuration files that should be usable for extended period
 * of time (years) without being resaved. The human-readable name can be changed
 * at whim.
 *
 * Each section in a plugin should have an unique internalName().
 *
 * The 'internal' name should be as friendly to any underlying Operating System
 * as possible. Using only lower-case letters and digits is enforced. If plugin
 * specifies invalid characters in the 'internal' name, the proper format will
 * be coerced; all invalid characters will be removed and all letters
 * lowercased. The human-readable name(), when not specified explicitly, will
 * use the specified 'internal' name, but will have all original characters
 * intact. What's forbidden is wrapping the 'internal' name in a QObject::tr()
 * call. Unfortunately, Doomseeker has no means to detect whether string is
 * translated or not, so the responsibility to ensure that it isn't falls on the
 * author of the plugin. It is very important to pay attention to this as the
 * plugin may appear to work correctly at first but break in certain cases only,
 * for example when user saves game configurations while using one language
 * translation and tries to load them when using another.
 *
 * Some call examples:
 *
 * @code
 * DMFlagsSection dmflags("dmflags");
 * DMFlagsSection compatFlags("compatflags", tr("Compat. Flags"));
 * @endcode
 */
class MAIN_EXPORT DMFlagsSection
{
public:
	/**
	 * @brief Matches sections by internalName()
	 *        and calls removed() on them.
	 *
	 * Sections that are emptied are also returned as empty sections.
	 */
	static QList<DMFlagsSection> removedBySection(
		const QList<DMFlagsSection> &original,
		const QList<DMFlagsSection> &removals);

	DMFlagsSection();

	/**
	 * @brief Creates DMFlags section with same user-displayable
	 *        and internal names.
	 *
	 * @warning Don't tr() this name!
	 */
	DMFlagsSection(const QString &internalName);

	/**
	 * @brief Creates DMFlags section with different user-displayable
	 *        and internal names.
	 *
	 * @warning Don't tr() the internalName!
	 */
	DMFlagsSection(const QString &internalName, const QString &name);

	DMFlagsSection(const DMFlagsSection &other);
	virtual ~DMFlagsSection();
	DMFlagsSection &operator=(const DMFlagsSection &other);

	/**
	 * @brief Append a new DMFlag to this section.
	 *
	 * Note that conflicting DMFlags will still be accepted here, as no
	 * check are performed. Objects added here affect results of
	 * combineValues().
	 */
	void add(const DMFlag &flag);
	/**
	 * @brief Logical OR of all DMFlag::value() results in this collection.
	 *
	 * Invalid DMFlag objects, if present in the collection, do not affect
	 * the output of this operation.
	 */
	unsigned combineValues() const;

	/**
	 * @brief Copies section maintaining its properties
	 *        but removing all flags.
	 */
	DMFlagsSection copyEmpty() const;

	/**
	 * @brief Number of DMFlag objects inside the collection.
	 */
	int count() const;

	/**
	 * @param Name used for internal identification purposes.
	 *
	 * This name must be unique within given plugin.
	 */
	const QString &internalName() const;

	/**
	 * @brief Does this section contain any dmflag?
	 */
	bool isEmpty() const;

	/**
	 * @brief User-displayable name of this section,
	 *        ex. "Compatibility flags".
	 */
	const QString &name() const;
	/**
	 * @brief Access DMFlag at specific index with '[]' operator.
	 */
	const DMFlag &operator[](int index) const;
	DMFlag &operator[](int index);

	/**
	 * @brief Stream input operator that appends DMFlag to the collection,
	 *        same as add().
	 */
	DMFlagsSection &operator<<(const DMFlag &flag)
	{
		add(flag);
		return *this;
	}

	/**
	 * @brief Returns a copy of this list with specified DMFlags removed.
	 *
	 * Removal is determined basing on DMFlag::value() comparison.
	 *
	 * @param removals
	 *     List of DMFlags to remove.
	 */
	DMFlagsSection removed(const DMFlagsSection &removals) const;

private:
	DPtr<DMFlagsSection> d;
};

/**
 * @ingroup group_pluginapi
 * @brief A general game setting or variable (like fraglimit).
 *
 * This object is safe to copy.
 */
class MAIN_EXPORT GameCVar
{
public:
	GameCVar();
	GameCVar(const QString &name, const QString &command);
	GameCVar(const QString &name, const QString &command, const QVariant &value);
	GameCVar(const GameCVar &other);
	virtual ~GameCVar();
	GameCVar &operator=(const GameCVar &other);

	/**
	 * @brief Command-line argument that sets this GameCVar.
	 *
	 * When launching a game, this command() is passed as one of the
	 * command line arguments and the value() is what follows directly
	 * after.
	 */
	const QString &command() const;

	/**
	 * @brief Is any value assigned to this GameCVar.
	 */
	bool hasValue() const;
	/**
	 * @brief 'Null' objects are invalid.
	 */
	bool isValid() const;

	/**
	 * @brief Nice name to display to user in Create Game dialog and
	 *        in other widgets.
	 */
	const QString &name() const;

	/**
	 * @brief Assign value() to this GameCVar.
	 */
	void setValue(const QVariant &value);

	/**
	 * @brief Passed as the second argument, following command().
	 */
	const QVariant &value() const;
	QString valueString() const { return value().toString(); }
	bool valueBool() const { return value().toBool(); }
	int valueInt() const { return value().toInt(); }

private:
	DPtr<GameCVar> d;
};

/**
 * @ingroup group_pluginapi
 * @brief Creates GameCVar set.
 *
 * Override get() method and return a list of GameCVar objects.
 *
 * Order may be dependant on the context in which the objects are
 * required. Refer to documentation of specific provider usages.
 */
class MAIN_EXPORT GameCVarProvider : public QObject
{
	Q_OBJECT

public:
	GameCVarProvider();
	virtual ~GameCVarProvider() override;

	/**
	 * @brief Default implementation creates empty set.
	 *
	 * @param context
	 *     Context contents depend on context.
	 */
	virtual QList<GameCVar> get(const QVariant &context);

private:
	Q_DISABLE_COPY(GameCVarProvider)

	DPtr<GameCVarProvider> d;
};

/**
 * @ingroup group_pluginapi
 * @brief Unique identifier of a GameMode within the plugin.
 *
 * This is an integral value that can be cast to GameMode::StandardGameMode
 * or whatever other integral value the plugin provides. It is used to
 * uniquely identify the GameMode and is stored in game setup configurations
 * created by "Create Game" dialog box.
 *
 * gamemode_id should be a small integer that can be safely cast to int
 * regardless of the underlying CPU architecture. Plugins should not change
 * ID of a GameMode in their version history once a concrete value is decided
 * upon. Plugins also should not overlap GameMode::StandardGameMode values.
 *
 * When hosting a game, plugin should be prepared that the passed gamemode_id
 * will be GameMode::SGM_Unknown, upon which plugin should leave it up to the
 * game to pick the game mode.
 */
using gamemode_id = int;

/**
 * @ingroup group_pluginapi
 * @brief Game mode representation.
 *
 * The only available constructor will create an invalid object (returns false
 * on isValid()). The proper method of construction is to use provided static
 * builder methods. Either use one of 'mk' methods that are mentioned
 * in StandardGameMode description or build your own mode using ffaGame() or
 * teamGame() methods. index() in all cases must either be one of
 * StandardGameMode values or unique within your plugin for given game mode.
 *
 * This object is safe to copy.
 */
class MAIN_EXPORT GameMode
{
public:
	/**
	 * @brief These game modes are so common that Doomseeker represents
	 *        them through internal values and static methods.
	 *
	 * If your plugin recognizes that the server uses one of these game
	 * modes it's much better to use these values instead of implementing
	 * your own representation.
	 *
	 * <b>More importantly</b>, static builder methods have been provided
	 * that already create appropriate GameMode objects. These methods are:
	 * mkCooperative(), mkDeathmatch(), mkTeamDeathmatch(),
	 * mkCaptureTheFlag(), and mkUnknown().
	 */
	enum StandardGameMode
	{
		SGM_Cooperative = 900,
		SGM_Deathmatch = 901,
		SGM_TeamDeathmatch = 902,
		SGM_CTF = 903,
		/**
		 * @brief Impossible to determine the game mode.
		 */
		SGM_Unknown = 904
	};

	// Standard game mode set
	// These should be used in order to keep the names uniform.
	// These can't be static members as translations may not work.
	static GameMode mkCooperative();
	static GameMode mkDeathmatch();
	static GameMode mkTeamDeathmatch();
	static GameMode mkCaptureTheFlag();
	static GameMode mkUnknown();

	/**
	 * @brief Construct a custom FFA game where players don't belong
	 *        to any teams.
	 *
	 * Cooperative game modes like "coop", "survival" or "invasion" are
	 * also considered as FFA.
	 *
	 * @param index
	 *     Value unique to given plugin but not within 900 - 1000 range.
	 * @param name
	 *     User-displayable name of the game mode.
	 *
	 * @see teamGame()
	 */
	static GameMode ffaGame(int index, const QString &name);
	/**
	 * @brief Game mode based on rivaling teams.
	 *
	 * @param index
	 *     Value unique to given plugin but not within 900 - 1000 range.
	 * @param name
	 *     User-displayable name of the game mode.
	 *
	 * @see ffaGame()
	 */
	static GameMode teamGame(int index, const QString &name);

	/**
	 * @brief Constructs an invalid GameMode object.
	 */
	GameMode();
	GameMode(const GameMode &other);
	virtual ~GameMode();
	GameMode &operator=(const GameMode &other);

	/**
	 * @brief Index, either a StandardGameMode or custom defined by plugin.
	 */
	gamemode_id index() const;

	/**
	 * @brief User-friendly name to display for game mode.
	 *
	 * This should be fairly short about no longer than "cooperative".
	 */
	const QString &name() const;

	/**
	 * @brief Is this GameMode based on rivaling teams?
	 */
	bool isTeamGame() const;
	/**
	 * @brief 'Null' objects are invalid.
	 */
	bool isValid() const;

private:
	DPtr<GameMode> d;

	GameMode(gamemode_id index, const QString &name);

	void setTeamGame(bool b);
};

/**
 * @ingroup group_pluginapi
 * @brief PWAD hosted on a server.
 *
 * Some ports support optional wads.
 *
 * This object is safe to copy.
 */
class MAIN_EXPORT PWad
{
public:
	PWad(const QString &name, bool optional = false);
	PWad(const QString &name, bool optional, const QList<Checksum> &checksums);
	PWad(const ModFile &modFile);
	PWad(const PWad &other);
	virtual ~PWad();
	PWad &operator=(const PWad &other);

	operator ModFile();
	/**
	 * @brief Is this WAD required to join the server?
	 */
	bool isOptional() const;
	/**
	 * @brief File name of the WAD.
	 */
	const QString &name() const;
	/**
	 * @brief List of checksums of the WAD.
	 */
	const QList<Checksum> checksums() const;
	/**
	 * @brief Adds a checksum to the WAD.
	 *
	 * @param hash - Hash resulting from hashing the WAD with the algorithm.
	 * @param algorithm - Algorithm used to produce the hash.
	 */
	void addChecksum(const QByteArray &hash, const QCryptographicHash::Algorithm &algorithm);
	/**
	 * @brief Verifies if a file has the same checksums as the PWad.
	 *
	 * @param path - Path to the file.
	 */
	bool validFile(const QString &path) const;
private:
	DPtr<PWad> d;
};

/**
 * @ingroup group_pluginapi
 * @brief Game difficulty setting.
 */
class Skill
{
public:
	/**
	 * Undefined difficulty; when creating a game, the skill setting
	 * should be omitted upon encountering this value.
	 */
	static const unsigned char UNDEFINED = CHAR_MAX;
};

#endif
