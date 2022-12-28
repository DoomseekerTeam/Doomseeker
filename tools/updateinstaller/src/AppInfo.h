#pragma once

#include <string>

/** This class provides project-specific updater properties,
  * such as the name of the application being updated and
  * the path to log details of the update install to.
  */
class AppInfo
{
	public:
		// Basic application information
		static std::string name();
		static std::string appName();
		static std::string organizationName();

		/** Returns a message to display to the user in the event
		  * of a problem installing the update.
		  */
		static std::string updateErrorMessage(const std::string& details);
};

inline std::string AppInfo::name()
{
	return "Doomseeker Updater";
}

inline std::string AppInfo::appName()
{
	return "Doomseeker";
}

inline std::string AppInfo::organizationName()
{
	return "The Doomseeker Team";
}

