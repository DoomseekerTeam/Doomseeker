//------------------------------------------------------------------------------
// dmflagspanel.cpp
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "dmflagspanel.h"
#include "ui_dmflagspanel.h"

#include "ini/ini.h"
#include "plugins/engineplugin.h"
#include "serverapi/gamecreateparams.h"
#include "serverapi/serverstructs.h"

#include <QCheckBox>

class DMFlagsTabWidget
{
public:
	QWidget *widget;
	DMFlagsSection section;

	/**
	 * Check boxes in the same order the flags are stored in the plugin.
	 */
	QList<QCheckBox *> checkBoxes;
};


DClass<DMFlagsPanel> : public Ui::DMFlagsPanel
{
public:
	QList<DMFlagsTabWidget *> dmFlagsTabs;
};

DPointered(DMFlagsPanel)


DMFlagsPanel::DMFlagsPanel(QWidget *parent)
	: QWidget(parent)
{
	d->setupUi(this);
}

DMFlagsPanel::~DMFlagsPanel()
{
	removeDMFlagsTabs();
}

QList<DMFlagsSection> DMFlagsPanel::dmFlags() const
{
	QList<DMFlagsSection> result;
	for (const DMFlagsTabWidget *p : d->dmFlagsTabs)
	{
		DMFlagsSection sec = p->section.copyEmpty();
		for (int i = 0; i < p->section.count(); ++i)
		{
			if (p->checkBoxes[i]->isChecked())
				sec.add(p->section[i]);
		}
		result << sec;
	}
	return result;
}

bool DMFlagsPanel::initDMFlagsTabs(const EnginePlugin *engine)
{
	removeDMFlagsTabs();

	if (engine->data()->createDMFlagsPagesAutomatic)
	{
		QList<DMFlagsSection> dmFlagsSections = engine->dmFlags();
		if (dmFlagsSections.empty())
			return false; // Nothing to do

		for (int i = 0; i < dmFlagsSections.count(); ++i)
		{
			auto dmftw = new DMFlagsTabWidget();

			QWidget *flagsTab = new QWidget(this);
			dmftw->widget = flagsTab;
			dmftw->section = dmFlagsSections[i];

			auto hLayout = new QHBoxLayout(flagsTab);

			QVBoxLayout *layout = nullptr;
			for (int j = 0; j < dmFlagsSections[i].count(); ++j)
			{
				if ((j % 16) == 0)
				{
					if (layout != nullptr)
						layout->addStretch();

					layout = new QVBoxLayout();
					hLayout->addLayout(layout);
				}

				auto checkBox = new QCheckBox();
				checkBox->setText(dmFlagsSections[i][j].name());
				dmftw->checkBoxes << checkBox;
				layout->addWidget(checkBox);
			}

			if (layout != nullptr)
				layout->addStretch();

			d->dmFlagsTabs << dmftw;
			d->tabWidget->addTab(flagsTab, dmFlagsSections[i].name());
		}
		return true;
	}
	return false;
}

void DMFlagsPanel::removeDMFlagsTabs()
{
	for (DMFlagsTabWidget *flags : d->dmFlagsTabs)
	{
		int index = d->tabWidget->indexOf(flags->widget);
		d->tabWidget->removeTab(index);
		delete flags->widget;
		delete flags;
	}

	d->dmFlagsTabs.clear();
}

void DMFlagsPanel::fillInParams(GameCreateParams &params)
{
	params.dmFlags() = dmFlags();
}

void DMFlagsPanel::loadConfig(Ini &config)
{
	IniSection dmflags = config.section("DMFlags");
	for (DMFlagsTabWidget *p : d->dmFlagsTabs)
	{
		const DMFlagsSection &section = p->section;
		for (int i = 0; i < section.count(); ++i)
		{
			QString flagKey = section.internalName() + "/" + section[i].internalName();
			if (!dmflags.hasSetting(flagKey))
			{
				// Fallback to support game setup configs
				// from old Doomseeker versions.
				flagKey = section.name() + "/" + section[i].name();
			}
			p->checkBoxes[i]->setChecked(dmflags[flagKey]);
		}
	}
}

void DMFlagsPanel::saveConfig(Ini &config)
{
	config.deleteSection("DMFlags");
	IniSection dmflags = config.section("DMFlags");
	for (DMFlagsTabWidget *p : d->dmFlagsTabs)
	{
		for (int i = 0; i < p->section.count(); ++i)
		{
			dmflags[p->section.internalName() + "/" + p->section[i].internalName()]
				= p->checkBoxes[i]->isChecked();
		}
	}
}
