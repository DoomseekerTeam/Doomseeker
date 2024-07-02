//------------------------------------------------------------------------------
// engineplugincombobox.cpp
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
#include "engineplugincombobox.h"

#include "plugins/engineplugin.h"
#include "plugins/pluginloader.h"

EnginePluginComboBox::EnginePluginComboBox(QWidget *parent)
	: QComboBox(parent), allowUnknown(false)
{
	this->connect(this, SIGNAL(currentIndexChanged(int)), SLOT(onIndexChanged()));
	loadPlugins();
}

EnginePlugin *EnginePluginComboBox::currentPlugin() const
{
	if (currentIndex() >= 0)
	{
		unsigned enginePluginIndex = itemData(currentIndex()).toUInt();
		if (enginePluginIndex < gPlugins->numPlugins())
			return gPlugins->info(enginePluginIndex);
	}
	return nullptr;
}

void EnginePluginComboBox::loadPlugins()
{
	clear();

	for (unsigned i = 0; i < gPlugins->numPlugins(); ++i)
	{
		const EnginePlugin *plugin = gPlugins->info(i);
		addItem(plugin->icon(), plugin->data()->name, i);
	}

	if (count() > 0)
		setCurrentIndex(0);
}

void EnginePluginComboBox::onIndexChanged()
{
	emit currentPluginChanged(currentPlugin());
}

void EnginePluginComboBox::setAllowUnknown(bool allow)
{
	this->allowUnknown = allow;
	if (!allow && this->count() > gPlugins->numPlugins())
	{
		this->removeItem(gPlugins->numPlugins());
	}
}

bool EnginePluginComboBox::setPluginByName(const QString &name)
{
	int engIndex = gPlugins->pluginIndexFromName(name);
	if (engIndex < 0)
	{
		if (this->allowUnknown)
		{
			QString label = tr("%1 (unknown)").arg(name);
			if (this->count() > gPlugins->numPlugins())
			{
				// There already is an unknown plugin selected.
				this->setItemText(gPlugins->numPlugins(), label);
			}
			else
			{
				this->addItem(label);
			}
			this->setCurrentIndex(gPlugins->numPlugins());
		}
		return false;
	}
	setCurrentIndex(engIndex);
	return true;
}
