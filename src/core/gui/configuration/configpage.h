//------------------------------------------------------------------------------
// configpage.h
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

#ifndef _CONFIG_PAGE_H_
#define _CONFIG_PAGE_H_

#include "dptr.h"
#include "global.h"
#include "serverapi/polymorphism.h"
#include <QWidget>

/**
 * @ingroup group_pluginapi
 * @brief Base class for configuration pages.
 *
 * Subclassed pages contain actual widgets allowing user input
 * in order to configure program's modules. Specific settings are
 * read and saved through virtual methods: saveSettings() and readSettings().
 *
 * Plugins should rather inherit from EngineConfigPage
 * as this provides some default, common behavior, like executable path or
 * custom program parameters configuration.
 */
class MAIN_EXPORT ConfigPage : public QWidget
{
	Q_OBJECT

public:
	/// Result of validate()
	enum Validation
	{
		/// Validation detected no problems.
		VALIDATION_OK,
		/// Validation detected at least one problem.
		VALIDATION_ERROR,
	};

	ConfigPage(QWidget *parent = nullptr);
	virtual ~ConfigPage() override;

	/**
	 * @brief Does this page allow to save the new settings?
	 *
	 * @see setAllowSave()
	 */
	bool allowSave();
	/**
	 * @brief true if settings for this page have already been loaded
	 *        at least once.
	 */
	bool areSettingsAlreadyRead();

	/**
	 * @brief Reimplement this to return a displayable icon for the
	 *        ConfigPage.
	 *
	 * If there is no QIcon associated with this page just return
	 * a QIcon object with argument-less constructor.
	 */
	virtual QIcon icon() const = 0;

	/**
	 * @brief Reimplement this to return a list-displayable name for this
	 *        ConfigPage.
	 */
	virtual QString name() const = 0;

	/**
	 * @brief Change whether settings on this page can be stored in
	 *        persisting configuration.
	 */
	void setAllowSave(bool b);
	/**
	 * @brief Read configuration from persistence to page contents.
	 */
	void read();
	/**
	 * @b [Virtual] Called when the config box is discarded
	 * without accepting the settings.
	 *
	 * The default implementation does nothing.
	 */
	void reject();
	/**
	 * @brief Save configuration from page contents to persistence.
	 *
	 * @return true if save completes successfully, false on error.
	 */
	bool save();
	/**
	 * @brief Page title, by default returns the same string as name().
	 */
	virtual QString title() const;

	/**
	 * @brief Validate settings on this page.
	 *
	 * If settings seem to be configured incorrectly (paths point
	 * to missing directories or files, data is missing, etc.),
	 * the page can decide to notify the user about a problem.
	 * Validation result determines how the problem notification
	 * will be displayed by Doomseeker in the configuration dialog box.
	 * Still, the page itself must also take care to precisely pinpoint
	 * the reason of the failure. When user navigates to the page,
	 * it must be immediately visible what caused the problem
	 * and why.
	 *
	 * This is used purely for notification purposes. User
	 * should not be blocked from saving the configuration
	 * even if validation fails spectacularly.
	 *
	 * This method can be called at any time by Doomseeker
	 * and must not block. It's expected that all checks
	 * will be light-weight.
	 *
	 * A page can also decide to validate itself at any time.
	 * This is done by emitting the validationRequested() signal,
	 * upon which Doomseeker will call validate().
	 *
	 * @return Return result of validation. Default implementation
	 * always returns VALIDATION_OK.
	 */
	virtual Validation validate();

signals:
	/**
	 * @brief Emit to tell Doomseeker to redraw certain widgets.
	 *
	 * This will send a request to Doomseeker to redraw some graphics.
	 * This should be emitted if settings on current page change
	 * program's appearance so that program can redraw affected widgets.
	 */
	void appearanceChanged();
	/**
	 * @brief Emit to tell Doomseeker to ask the user for a restart.
	 *
	 */
	void restartNeeded();

	/**
	 * @brief Request that the page should be (re-)validated.
	 *
	 * Emit this whenever page data changes and you wish Doomseeker
	 * to re-evaluate the validity of the configuration.
	 */
	void validationRequested();

protected:
	/**
	 * @brief Reimplement this to read settings from config into widgets.
	 */
	virtual void readSettings() = 0;

	/**
	 * @brief Reimplement this to write settings to config from widgets.
	 */
	virtual void saveSettings() = 0;

	POLYMORPHIC_SETTER_DECLARE(void, ConfigPage, reject, ())
	void reject_default();

private:
	DPtr<ConfigPage> d;
};

#endif
