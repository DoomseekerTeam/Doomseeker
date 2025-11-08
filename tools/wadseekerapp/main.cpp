//------------------------------------------------------------------------------
// main.cpp - Wadseeker Usage Example
//------------------------------------------------------------------------------
//
// Copyright (c) 2009, "Blzut3"
// Copyright (c) 2025, "Zalewa"
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * The name of the author may not be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//------------------------------------------------------------------------------
#include "main.h"
#include "wadseeker/wadseekermessagetype.h"

#include <qcryptographichash.h>
#include <wadseeker/entities/checksum.h>
#include <wadseeker/entities/modset.h>
#include <wadseeker/wadseekerversioninfo.h>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QStringList>
#include <QTextStream>
#include <functional>
#include <cstring>


using namespace std;

namespace
{
const int EC_OK = 0;
const int EC_ARGS = 1;
const int EC_FAIL = 2;

QTextStream logi(stderr);

struct Args
{
	bool noDefaultSites;
	bool noIdgames;
	QString outputDir;
	QStringList files;
	QStringList sites;

	Args()
		: noDefaultSites(false), noIdgames(false), outputDir("./")
	{}
};

Args parseArgs(QCoreApplication &app, QCommandLineParser &argp)
{
	Args args;

	// Standard options and program description.
	argp.setApplicationDescription("Wadseeker command line interface.");
	argp.addHelpOption();
	argp.addVersionOption();

	// Option collector: a convoluted way of not repeating
	// the same code for each option.
	QList<std::function<void(void)>> collectors;
	auto addFlag = [&argp, &collectors](QStringList names, bool &variable, QString help)
	{
		QCommandLineOption opt(names, help);
		argp.addOption(opt);
		collectors << [&argp, opt, &variable]() {
			variable = argp.isSet(opt);
		};
	};

	auto addOption = [&argp, &collectors](QStringList names, QString valueName, QString &variable, QString help)
	{
		QCommandLineOption opt(names, help, valueName, variable);
		argp.addOption(opt);
		collectors << [&argp, opt, &variable]() {
			variable = argp.value(opt);
		};
	};

	auto addListOption = [&argp, &collectors](QStringList names, QString valueName, QStringList &variable, QString help)
	{
		QCommandLineOption opt(names, help, valueName);
		opt.setDefaultValues(variable);
		argp.addOption(opt);
		collectors << [&argp, opt, &variable]() {
			variable << argp.value(opt);
		};
	};

	// Add the options (with the '-' and '--') using the collector.
	addFlag({"no-default-sites"}, args.noDefaultSites,
		"Disable Wadseeker's built-in default sites.");
	addFlag({"no-idgames"}, args.noIdgames,
		"Disable the /idgames Archive.");
	addOption({"o", "output"}, "dir", args.outputDir,
		"Set the output directory.");
	addListOption({"u", "url"}, "url", args.sites,
		"Sites where to look for the files (multiple).");

	// Files to seek go onto positional arguments.
	argp.addPositionalArgument(
		"files",
		"Files to seek. Each file can have an optional checksum specified in format "
		"<algorithm:hex>, where 'algorithm' is one of: md5, sha1, sha256, and 'hex' "
		"is the checksum value expressed using hexadecimals.\n\n"
		"Examples:\n"
		"- myfirstlevel.wad\n"
		"- mysecondlevel.wad:md5:787733ded5c97c5c2cbd120a5e9a8998\n"
		,
		"file[:checksum] [file...]"
	);

	// Process the command line and collect the arguments into a structure.
	argp.process(app);
	for (auto collector : collectors)
		collector();
	args.files = argp.positionalArguments();

	return args;
}
}

int main(int argc, char* argv[])
{
	// Create the application.
	QCoreApplication app(argc, argv);
	app.setApplicationName("wadseeker");
	app.setApplicationVersion(WadseekerVersionInfo::version());

	// Parse the command line arguments.
	QCommandLineParser argp;
	Args args = parseArgs(app, argp);

	// Print version information.
	logi << "Wadseeker (" << WadseekerVersionInfo::version() << ")" << Qt::endl;

	if (args.files.isEmpty())
	{
		// No files specified, show help with a non-zero exitcode.
		argp.showHelp(EC_ARGS);
		// showHelp() should quit, but just in case:
		return EC_ARGS;
	}

	// Create and configure the instance of Wadseeker now.
	Wadseeker wadseeker;

	// Load up the primary sites.
	if (!args.sites.isEmpty())
	{
		QStringList sites = args.sites;
		if (!args.noDefaultSites)
		{
			for (int i = 0; !Wadseeker::defaultSites[i].isEmpty(); ++i)
				sites << Wadseeker::defaultSites[i];
		}
		wadseeker.setPrimarySites(sites);
	}
	else if (!args.noDefaultSites)
	{
		wadseeker.setPrimarySitesToDefault();
	}
	wadseeker.setIdgamesEnabled(!args.noIdgames);

	// Set the output directory.
	wadseeker.setTargetDirectory(args.outputDir);

	// Create the Wadseeker interface and seek for the files.
	WadseekerInterface wadseekerInterface(wadseeker);
	ModSet modSet;
	if (!wadseekerInterface.parseFiles(args.files, modSet))
	{
		return EC_FAIL;
	}
	if (!wadseekerInterface.seek(modSet))
	{
		logi << "Failed to start the seek" << Qt::endl;
		return EC_FAIL;
	}

	// Wait for Wadseeker to finish.
	if (wadseekerInterface.state == WadseekerInterface::S_DONE_SUCCESS)
	{
		return EC_OK;
	}
	else if (wadseekerInterface.state == WadseekerInterface::S_DONE_FAILURE)
	{
		return EC_FAIL;
	}
	else
	{
		return app.exec();
	}
}

////////////////////////////////////////////////////////////////////////////////
// WadseekerInterface handles the interations with the Wadseeker object.

WadseekerInterface::WadseekerInterface(Wadseeker &wadseeker)
	: state(S_PENDING), wadseeker(wadseeker), lastProgressReportLength(0)
{
	// Connect our slots.
	connect(&wadseeker, &Wadseeker::allDone,
		this, &WadseekerInterface::done);
	connect(&wadseeker, &Wadseeker::fileDownloadProgress,
		this, &WadseekerInterface::downloadProgress);
	connect(&wadseeker, &Wadseeker::message,
		this, &WadseekerInterface::receiveMessage);
}

bool WadseekerInterface::parseFiles(const QStringList &files, ModSet &modSet)
{
	// Parse the files with potential checksums into a ModSet.
	modSet.clear();
	for (QString file : files)
	{
		// Collect the info about the file.
		QStringList fileTokens = file.split(":");
		QString fileName;
		QList<Checksum> checksums;
		// Valid formats are:
		//
		// 1. <filename>
		// 2. <filename:algo:hex>
		if (fileTokens.size() == 1)
		{
			fileName = fileTokens[0];
		}
		else if (fileTokens.size() == 3)
		{
			fileName = fileTokens[0];
			const QString algoName = fileTokens[1].toLower();
			const QByteArray hexvalue = fileTokens[2].toLower().toLatin1();

			// Identify the checksum algorithm by name.
			QCryptographicHash::Algorithm checksumAlgorithm = QCryptographicHash::Md5;
			if (algoName == "md5")
			{
				checksumAlgorithm = QCryptographicHash::Md5;
			}
			else if (algoName == "sha1")
			{
				checksumAlgorithm = QCryptographicHash::Sha1;
			}
			else if (algoName == "sha256")
			{
				checksumAlgorithm = QCryptographicHash::Sha256;
			}
			else
			{
				logi << "Unknown checksum algorithm '" << algoName << "' for file: " << file;
				return false;
			}

			// Convert the hexadecimal values into a byte array.
			QByteArray checksumValue = QByteArray::fromHex(hexvalue);
			if (checksumValue.toHex() != hexvalue)
			{
				logi << "Unable to parse hex value '" << hexvalue << "' for file: " << file;
				return false;
			}

			checksums << Checksum(checksumValue, checksumAlgorithm);
		}
		else
		{
			logi << "File specified in invalid format: " << file << Qt::endl;
			return false;
		}

		// Create ModFile structure from the collected info.
		ModFile modFile(fileName);
		modFile.setChecksums(checksums);
		modSet.addModFile(modFile);
	}
	return true;
}

bool WadseekerInterface::seek(const ModSet &modSet)
{
	this->state = S_RUNNING;

	// Pass the wad list into wadseeker.
	bool started = wadseeker.startSeek(modSet);
	if (!started)
		this->state = S_DONE_FAILURE;
	return started;
}

void WadseekerInterface::done(bool success)
{
	// We're done close the app.
	logi << "DONE: " << (success ? "Success!" : "Fail!") << Qt::endl;
	this->state = success ? S_DONE_SUCCESS : S_DONE_FAILURE;
	QCoreApplication::instance()->exit(success ? EC_OK : EC_FAIL);
}

void WadseekerInterface::downloadProgress(const ModFile &filename, qint64 done, qint64 total)
{
	// Prevent a divide by zero error.
	if (total == 0)
		return;

	// Update the download progress indicator.
	QString progress = QString("Download Progress %1: %2/%3 (%4%)")
		.arg(filename.fileName())
		.arg(done)
		.arg(total)
		.arg(done*100/total);

	logi << QString(lastProgressReportLength, '\b')
		<< progress;
	if (lastProgressReportLength - progress.length() > 0)
	{
		logi << QString(lastProgressReportLength - progress.length(), ' ')
			<< QString(lastProgressReportLength - progress.length(), '\b');
	}
	logi << Qt::flush;
	lastProgressReportLength = progress.length();
}

void WadseekerInterface::receiveMessage(const QString &msg, WadseekerLib::MessageType type)
{
	// This is a check to see if we need to newline the progress indicator.
	if (lastProgressReportLength != 0)
	{
		logi << Qt::endl;
		lastProgressReportLength = 0;
	}

	// Determine the type of message and print it.
	switch (type)
	{
	default:
	case WadseekerLib::Notice:
		logi << "NOTI: ";
		break;
	case WadseekerLib::NoticeImportant:
		logi << "IMPO: ";
		break;
	case WadseekerLib::Navigation:
		logi << "NAVI: ";
		break;
	case WadseekerLib::Error:
		logi << "ERR : ";
		break;
	case WadseekerLib::CriticalError:
		logi << "CRIT: ";
		break;
	}
	logi << msg << Qt::endl;
}
