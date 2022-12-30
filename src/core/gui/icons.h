#ifndef DOOMSEEKER_GUI_ICONS_H
#define DOOMSEEKER_GUI_ICONS_H

#include <QIcon>

/**
 * @brief Provide convoluted icons.
 *
 * Helps deal with cases where the icons may depend on the environment.
 */
class Icons
{
public:
	/**
	 * @brief An appropriate clear icon.
	 *
	 * Provided from the style if available, or falls back to the app's
	 * built-in.
	 */
	static QIcon clear();

private:
	Icons() = delete;
};

#endif
