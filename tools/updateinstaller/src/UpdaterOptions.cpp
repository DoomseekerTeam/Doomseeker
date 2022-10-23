#include "UpdaterOptions.h"

#include "Log.h"
#include "AnyOption/anyoption.h"
#include "FileUtils.h"
#include "Platform.h"
#include "StringUtils.h"

#include <cstdlib>
#include <iostream>

const char HEX2DEC[256] =
{
	/*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
	/* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,

	/* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

	/* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

	/* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};

std::string uriDecode(const std::string& sSrc)
{
	// (c) Jin Qing - 2nd November 2006.
	// http://www.codeguru.com/cpp/cpp/algorithms/strings/article.php/c12759/URI-Encoding-and-Decoding.htm

	// Note from RFC1630: "Sequences which start with a percent
	// sign but are not followed by two hexadecimal characters
	// (0-9, A-F) are reserved for future extension"

	const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
	const int SRC_LEN = sSrc.length();
	const unsigned char * const SRC_END = pSrc + SRC_LEN;
	// last decodable '%' 
	const unsigned char * const SRC_LAST_DEC = SRC_END - 2;

	char * const pStart = new char[SRC_LEN];
	char * pEnd = pStart;

	while (pSrc < SRC_LAST_DEC)
	{
		if (*pSrc == '%')
		{
			char dec1, dec2;
			if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
				&& -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
			{
				*pEnd++ = (dec1 << 4) + dec2;
				pSrc += 3;
				continue;
			}
		}
		*pEnd++ = *pSrc++;
	}

	// the last 2- chars
	while (pSrc < SRC_END)
	{
		*pEnd++ = *pSrc++;
	}

	std::string sResult(pStart, pEnd);
	delete [] pStart;
	return sResult;
}

UpdaterOptions::UpdaterOptions()
: mode(UpdateInstaller::Setup)
, waitPid(0)
, showVersion(false)
, forceElevated(false)
{
}

UpdateInstaller::Mode stringToMode(const std::string& modeStr)
{
	if (modeStr == "main")
	{
		return UpdateInstaller::Main;
	}
	else
	{
		if (!modeStr.empty())
		{
			LOG(Error,"Unknown mode " + modeStr);
		}
		return UpdateInstaller::Setup;
	}
}

std::list<std::string> UpdaterOptions::parseArgs(const std::string& args)
{
	// Spaces in this string separate arguments.
	// Arguments themselves are URL encoded.
	std::list<std::string> argsList;
	unsigned idx = 0;
	while (idx < args.length())
	{
		int posSpace = args.find(" ", idx);
		std::string subString = args.substr(idx, posSpace);
		argsList.push_back(uriDecode(subString));
		idx = posSpace;
		if (idx != string::npos)
		{
			++idx;
		}
	}
	for (std::list<std::string>::iterator it = argsList.begin();
		it != argsList.end(); ++it)
	{
		std::cout << *it << std::endl;
	}
	return argsList;
}

void UpdaterOptions::parseOldFormatArg(const std::string& arg, std::string* key, std::string* value)
{
	size_t pos = arg.find('=');
	if (pos != std::string::npos)
	{
		*key = arg.substr(0,pos);
		*value = arg.substr(pos+1);
	}
}

// this is a compatibility function to allow the updater binary
// to be involved by legacy versions of Mendeley Desktop
// which used a different syntax for the updater's command-line
// arguments
void UpdaterOptions::parseOldFormatArgs(int argc, char** argv)
{
	for (int i=0; i < argc; i++)
	{
		std::string key;
		std::string value;

		parseOldFormatArg(argv[i],&key,&value);

		if (key == "CurrentDir")
		{
			// CurrentDir is the directory containing the main application
			// binary.  On Mac and Linux this differs from the root of
			// the installation directory

#ifdef PLATFORM_LINUX
			// the main binary is in lib/mendeleydesktop/libexec,
			// go up 3 levels
			installDir = FileUtils::canonicalPath((value + "/../../../").c_str());
#elif defined(PLATFORM_MAC)
			// the main binary is in Contents/MacOS,
			// go up 2 levels
			installDir = FileUtils::canonicalPath((value + "/../../").c_str());
#elif defined(PLATFORM_WINDOWS)
			// the main binary is in the root of the install directory
			installDir = value;
#endif
		}
		else if (key == "TempDir")
		{
			packageDir = value;
		}
		else if (key == "UpdateScriptFileName")
		{
			scriptPath = value;
		}
		else if (key == "AppFileName")
		{
			// TODO - Store app file name
		}
		else if (key == "PID")
		{
			waitPid = static_cast<PLATFORM_PID>(atoll(value.c_str()));
		}
		else if (key == "--main")
		{
			mode = UpdateInstaller::Main;
		}
	}
}

void UpdaterOptions::parse(int argc, char** argv)
{
	AnyOption parser;
	parser.setVerbose();
	parser.setOption("install-dir");
	parser.setOption("package-dir");
	parser.setOption("script");
	parser.setOption("wait");
	parser.setOption("mode");
	parser.setOption("args");
	parser.setOption("exec");
	parser.setFlag("version");
	parser.setFlag("force-elevated");

	parser.processCommandArgs(argc,argv);

	if (parser.getValue("mode"))
	{
		mode = stringToMode(parser.getValue("mode"));
	}
	if (parser.getValue("install-dir"))
	{
		installDir = parser.getValue("install-dir");
	}
	if (parser.getValue("package-dir"))
	{
		packageDir = parser.getValue("package-dir");
	}
	if (parser.getValue("script"))
	{
		scriptPath = parser.getValue("script");
	}
	if (parser.getValue("exec"))
	{
		executable = parser.getValue("exec");
	}
	if (parser.getValue("args"))
	{
		runAfterInstallCmdArgs = parseArgs(parser.getValue("args"));
	}
	if (parser.getValue("wait"))
	{
		waitPid = static_cast<PLATFORM_PID>(atoll(parser.getValue("wait")));
	}

	showVersion = parser.getFlag("version");
	forceElevated = parser.getFlag("force-elevated");
		
	if (installDir.empty())
	{
		// if no --install-dir argument is present, try parsing
		// the command-line arguments in the old format (which uses
		// a list of 'Key=Value' args)
		parseOldFormatArgs(argc,argv);
	}
}

