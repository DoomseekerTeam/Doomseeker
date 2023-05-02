//------------------------------------------------------------------------------
// aboutdialog.cpp
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
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------
#include "aboutdialog.h"

#include "gui/commongui.h"
#include "gui/copytextdlg.h"
#include "plugins/engineplugin.h"
#include "plugins/pluginloader.h"
#include "ui_aboutdialog.h"
#include "version.h"
#include "wadseeker/wadseekerversioninfo.h"
#include <QPixmap>
#include <QResource>
#include <QString>

DClass<AboutDialog> : public Ui::AboutDialog
{
};

DPointered(AboutDialog)

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
	d->setupUi(this);
	CommonGUI::setupDialog(*this);

	connect(d->buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(close()));

	// Doomseeker
	d->versionChangeset->setText(Version::changeset());
	d->versionNumber->setText(Version::versionRevision());
	d->versionAbi->setText(QString("(ABI: %1)").arg(DOOMSEEKER_ABI_VERSION));
	d->lblRevision->setText(QString::number(Version::revisionNumber()));
	d->logo->setPixmap(QPixmap(":/logo.png"));
	d->pteCopyrightNotice->setPlainText(copyrightVerboseNotice());

	// Wadseeker
	d->wadseekerAuthor->setText(WadseekerVersionInfo::author());
	d->wadseekerDescription->setText(WadseekerVersionInfo::description());
	d->wadseekerVersion->setText(WadseekerVersionInfo::version());
	d->wadseekerYearSpan->setText(WadseekerVersionInfo::yearSpan());

	// Populate plugins dialog
	for (unsigned i = 0; i < gPlugins->numPlugins(); ++i)
		d->pluginBox->addItem( gPlugins->plugin(i)->info()->data()->name);
	connect(d->pluginBox, SIGNAL(currentIndexChanged(int)), SLOT(changePlugin(int)));
	changePlugin(0);

	adjustSize();
}

AboutDialog::~AboutDialog()
{
}

void AboutDialog::changePlugin(int pluginIndex)
{
	if (static_cast<unsigned>(pluginIndex) >= gPlugins->numPlugins())
		return; // Invalid plugin.

	const EnginePlugin *plug = gPlugins->plugin(pluginIndex)->info();

	if (plug->data()->aboutProvider.isNull())
		d->pluginDescription->setPlainText("");
	else
		d->pluginDescription->setPlainText(plug->data()->aboutProvider->provide());
	d->pluginAuthor->setText(plug->data()->author);
	d->pluginVersion->setText(QString("Version: %1").arg(plug->data()->version));
	d->pluginAbiVersion->setText(QString("(ABI: %1)").arg(plug->data()->abiVersion));
}

QString AboutDialog::copyrightVerboseNotice() const
{
	// This text is split into separate strings to simplify translation.
	// Whenever a single paragraph needs to be changed or a new text needs
	// to be added, it won't invalidate all existing translations.
	QStringList paragraphs;

	// Unicode chars
	QChar copyrightChar(0x00a9);
	QChar smallAGraveChar(0x00e0);

	// License
	paragraphs << tr("Copyright %1 %2 The Doomseeker Team")
		.arg(copyrightChar).arg(Version::yearSpan());
	paragraphs << tr("This program is distributed under the terms of the LGPL v2.1 or later.");

	// Translations
	paragraphs << tr("Doomseeker translations contributed by:\n")
		+ tr("- Polish: Zalewa") + "\n"
		+ tr("- Spanish: Pol Marcet Sard%1").arg(smallAGraveChar) + "\n"
		+ tr("- Catalan: Pol Marcet Sard%1").arg(smallAGraveChar);

	// GeoLite2
	paragraphs << tr("This program uses GeoLite2 data for IP-to-Country (IP2C) purposes, "
			"available from https://www.maxmind.com") + "\n" +
		tr("Database and Contents Copyright (c) 2018 MaxMind, Inc.");
	paragraphs << tr("GeoLite2 License:\n"
			"This work is licensed under the Creative Commons Attribution - ShareAlike 4.0 Unported License. "
			"To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/4.0/.");
	paragraphs << tr("GeoLite2 available at:\n"
			"https://dev.maxmind.com/geoip/geoip2/geolite2/");

	// Icons
	QStringList icons;
	icons << tr("- Aha-Soft");
	icons << tr("- Crystal Clear by Everaldo Coelho");
	icons << tr("- Fugue Icons (C) 2013 Yusuke Kamiyamane. All rights reserved.");
	icons << tr("- Nuvola 1.0 (KDE 3.x icon set)");
	icons << tr("- Oxygen Icons 4.3.1 (KDE)");
	icons << tr("- Silk Icon Set (C) Mark James (famfamfam.com)");
	icons << tr("- Tango Icon Library / Tango Desktop Project");
	paragraphs << tr("This program uses icons (or derivates of) from following sources:\n") + icons.join("\n");

	return paragraphs.join("\n\n");
}
