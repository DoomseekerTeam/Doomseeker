//------------------------------------------------------------------------------
// enginezandronumconfigbox.cpp
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "enginezandronumconfigbox.h"

#include "zandronumengineplugin.h"

#include <datapaths.h>
#include <ini/ini.h>
#include <plugins/engineplugin.h>

#include <QCheckBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QStyle>

EngineZandronumConfigBox::EngineZandronumConfigBox(EnginePlugin *plugin, IniSection &cfg, QWidget *parent)
	: EngineConfigPage(plugin, cfg, parent)
{
	// Create the testing box, we might as well do this in code.
	groupTesting = new QGroupBox();
	groupTesting->setTitle(tr("Testing releases"));
	groupTesting->setCheckable(true);
	groupTesting->setLayout(new QVBoxLayout());
	groupTesting->layout()->addWidget(new QLabel(tr("Directory for testing releases:")));
	addWidget(groupTesting);

	QWidget *releasePathLayout = new QWidget();
	releasePathLayout->setLayout(new QHBoxLayout());
	leTestingPath = new QLineEdit();
	btnBrowseTestingPath = new QPushButton();
	btnBrowseTestingPath->setToolTip(tr("Browse"));
	btnBrowseTestingPath->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
	releasePathLayout->layout()->addWidget(leTestingPath);
	releasePathLayout->layout()->addWidget(btnBrowseTestingPath);
	groupTesting->layout()->addWidget(releasePathLayout);

	connect(btnBrowseTestingPath, SIGNAL(clicked()), this, SLOT(btnBrowseTestingPathClicked()));

	cbSegmentedQuery = new QCheckBox(tr("Segmented server query (requires Zandronum 3.2-alpha)"), this);
	cbSegmentedQuery->setToolTip(tr(
		"<p>When enabled, the servers will be asked to provide their info in "
		"segments. Enable this if you experience problems when refreshing "
		"servers that host a lot of WADs or players. These servers send a "
		"lot of data, and enabling this asks them to send this data split "
		"between several packets.</p>"
		"<p>This feature requires the server to host at least a Zandronum "
		"3.2-alpha version.</p>"));
	addWidget(cbSegmentedQuery);
}

void EngineZandronumConfigBox::btnBrowseTestingPathClicked()
{
	QString strDirpath = QFileDialog::getExistingDirectory(this, tr("Doomseeker - choose Zandronum testing directory"));
	if (!strDirpath.isEmpty())
		leTestingPath->setText(gDefaultDataPaths->portablizePath(strDirpath));
}

void EngineZandronumConfigBox::readSettings()
{
	EngineConfigPage::readSettings();

	IniSection &config = *ZandronumEnginePlugin::staticInstance()->data()->pConfig;

	groupTesting->setChecked(config["EnableTesting"]);
	leTestingPath->setText(config["TestingPath"]);
	cbSegmentedQuery->setChecked(config["SegmentedQuery"]);
}

void EngineZandronumConfigBox::saveSettings()
{
	EngineConfigPage::saveSettings();

	QString strVal;

	IniSection &config = *ZandronumEnginePlugin::staticInstance()->data()->pConfig;

	config["EnableTesting"] = groupTesting->isChecked();

	strVal = leTestingPath->text();
	config["TestingPath"] = strVal;

	config["SegmentedQuery"] = cbSegmentedQuery->isChecked();
}
