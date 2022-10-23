//------------------------------------------------------------------------------
// main.cpp - Wadseeker Usage Example
//------------------------------------------------------------------------------
//
// Copyright (c) 2009, "Blzut3"
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

#include <QCoreApplication>
#include <QStringList>

#include <wadseeker/entities/modset.h>
#include <wadseeker/wadseekerversioninfo.h>
#include <iostream>
#include <cstring>

#include "main.h"

using namespace std;

QCoreApplication *instance;
int main(int argc, char* argv[])
{
	// Create an application and store it's instance into a global variable for 
	// later use.
	QCoreApplication app(argc, argv);
	instance = app.instance();

	// Print version information.
	cout << "Wadseeker (" << WadseekerVersionInfo::version().toLatin1().constData() << ")\n";
	if(argc < 2) // Not enough parameters display usage.
		cout << "Usage: wadseeker [-o output_dir] filename ...\n";
	else
	{
		// Collect a list of files to seek.
		QStringList files;

		// Are default output directory will be ./ unless specified by -o
		QString output("./");
		for(int i = 1;i < argc;i++)
		{
			if(strcmp(argv[i], "-o") == 0 && i < argc-1)
				output = argv[++i];
			else // Not a command line option so put it in our file list.
				files << argv[i];
		}

		// If there are no files do nothing.
		if(files.count() == 0)
			return 0;

		// Otherwise creat the wadseeker interface and seek for them.
		WadseekerInterface wi(output);
		wi.seek(files);

		// Wait for wadseeker to finish.
		app.exec();
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// WadseekerInterface handles the interations with the Wadseeker object.

WadseekerInterface::WadseekerInterface(const QString &output) : lastProgressReportLength(0)
{
	// Load up the primary sites and set the output to the specified location.
	wadseeker.setPrimarySitesToDefault();
	wadseeker.setTargetDirectory(output);

	// Connect our slots.
	connect(&wadseeker, SIGNAL(allDone(bool)), this, SLOT(done()));
	connect(&wadseeker, SIGNAL(fileDownloadProgress(const QString &, qint64, qint64)), this, SLOT(downloadProgress(const QString &, qint64, qint64)));
	connect(&wadseeker, SIGNAL(message(const QString &, WadseekerLib::MessageType)), this, SLOT(recieveMessage(const QString &, WadseekerLib::MessageType)));
}

void WadseekerInterface::seek(const QStringList &files)
{
	// Pass the wad list into wadseeker.
	ModSet modSet;
	for (auto file : files)
		modSet.addModFile(file);
	wadseeker.startSeek(modSet);
}

void WadseekerInterface::done()
{
	// We're done close the app.
	cout << '\n';
	instance->exit(0);
}

void WadseekerInterface::downloadProgress(const QString &filename, qint64 done, qint64 total)
{
	// Prevent a divide by zero error.
	if(total == 0)
		return;

	// Update the download progress indicator.
	QString progress = QString("Download Progress %1: %2/%3 (%4%)").arg(filename).arg(done).arg(total).arg(done*100/total);

	cout << QString(lastProgressReportLength, '\b').toLatin1().constData();
	cout << progress.toLatin1().constData();
	if(lastProgressReportLength - progress.length() > 0)
	{
		cout << QString(lastProgressReportLength - progress.length(), ' ').toLatin1().constData();
		cout << QString(lastProgressReportLength - progress.length(), '\b').toLatin1().constData();
	}
	cout.flush();
	lastProgressReportLength = progress.length();
}

void WadseekerInterface::recieveMessage(const QString &msg, WadseekerLib::MessageType type)
{
	// This is a check to see if we need to newline the progress indicator.
	if(lastProgressReportLength != 0)
	{
		cout << "\n";
		lastProgressReportLength = 0;
	}

	// Determine the type of message and print it.
	switch(type)
	{
		default:
		case WadseekerLib::Notice:
			cout << "NOTICE: ";
			break;
		case WadseekerLib::Error:
			cout << "ERROR: ";
			break;
		case WadseekerLib::CriticalError:
			cout << "CRITICAL ERROR: ";
			break;
	}
	cout << msg.toLatin1().constData() << "\n";
}
