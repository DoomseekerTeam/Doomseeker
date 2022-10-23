//------------------------------------------------------------------------------
// engineconfigpage.cpp
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
// Copyright (C) 2010 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------

#include "engineconfigpage.h"
#include "filefilter.h"
#include "gui/widgets/filepickwidget.h"
#include "ini/ini.h"
#include "plugins/engineplugin.h"
#include "serverapi/gameexefactory.h"
#include "serverapi/gamefile.h"
#include "ui_engineconfigpage.h"

#include <QFileDialog>
#include <QTimer>

DClass<EngineConfigPage> : public Ui::EngineConfigPage
{
public:
	class KnownNeighbours : public FilePickWidget::NeighbourStrategy
	{
	public:
		DPtr< ::EngineConfigPage> *wrapper;
		KnownNeighbours(DPtr< ::EngineConfigPage> *wrapper)
		{
			this->wrapper = wrapper;
		}
		QStringList neighbours() override
		{
			return (*wrapper)->parent->collectKnownGameFilePaths();
		}
	};


	::EngineConfigPage *parent;
	IniSection *config;
	EnginePlugin *plugin;
	bool clientOnly;
	QList<FilePickWidget *> filePickers;
	QTimer errorTimer;

	QStringList readStoredCustomParameters() const
	{
		return config->value("StoredCustomParameters").toStringList();
	}

	void saveStoredCustomParameters(const QStringList &params)
	{
		config->setValue("StoredCustomParameters", params);
	}

	bool existsInStoredCustomParameters(const QString &text) const
	{
		return readStoredCustomParameters().contains(text);
	}
};

DPointeredNoCopy(EngineConfigPage)


EngineConfigPage::EngineConfigPage(EnginePlugin *plugin, IniSection &cfg, QWidget *parent)
	: ConfigPage(parent)
{
	d->parent = this;
	d->plugin = plugin;
	d->config = &cfg;
	d->setupUi(this);

	d->lblError->hide();
	d->errorTimer.setInterval(5000);
	d->errorTimer.setSingleShot(true);
	this->connect(&d->errorTimer, SIGNAL(timeout()), d->lblError, SLOT(hide()));

	// Prevent combo box stretching.
	d->cboCustomParameters->setMinimumContentsLength(25);
	d->cboCustomParameters->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

	makeFileBrowsers();

	if (!plugin->data()->hasMasterClient())
		d->masterAddressBox->hide();
}

EngineConfigPage::~EngineConfigPage()
{
}

void EngineConfigPage::addWidget(QWidget *widget)
{
	layout()->removeItem(d->verticalSpacer);
	layout()->addWidget(widget);
	layout()->addItem(d->verticalSpacer);
}

void EngineConfigPage::autoFindNeighbouringPaths()
{
	auto picker = qobject_cast<FilePickWidget *>(sender());
	if (QFileInfo(picker->path()).isFile())
	{
		QStringList knownPaths = collectKnownGameFilePaths();
		for (FilePickWidget *picker : d->filePickers)
		{
			if (picker->isEmpty())
			{
				picker->blockSignals(true);
				picker->findPath();
				picker->blockSignals(false);
			}
		}
	}
	emit validationRequested();
}

QStringList EngineConfigPage::collectKnownGameFilePaths() const
{
	QStringList knownPaths;
	for (const FilePickWidget *picker : d->filePickers)
	{
		if (!picker->isEmpty())
			knownPaths << picker->path();
	}
	return knownPaths;
}

QString EngineConfigPage::currentCustomParameters() const
{
	return d->cboCustomParameters->currentText().trimmed();
}

QIcon EngineConfigPage::icon() const
{
	return d->plugin->icon();
}

void EngineConfigPage::makeFileBrowsers()
{
	QSharedPointer<FilePickWidget::NeighbourStrategy> neighbours(
		new PrivData<EngineConfigPage>::KnownNeighbours(&d));
	QList<GameFile> files = d->plugin->gameExe()->gameFiles().asQList();
	for (const GameFile &file : files)
	{
		auto widget = new FilePickWidget(d->exePickArea);
		widget->setFile(file);
		widget->setNeighbourStrategy(neighbours);
		this->connect(widget, SIGNAL(findFailed()), SLOT(showFindFailError()));
		this->connect(widget, SIGNAL(pathChanged()), SLOT(autoFindNeighbouringPaths()));
		this->connect(widget, SIGNAL(pathChanged()), SIGNAL(validationRequested()));

		d->exePickArea->layout()->addWidget(widget);
		d->filePickers << widget;
	}
}

QString EngineConfigPage::name() const
{
	return d->plugin->data()->name;
}

const EnginePlugin *EngineConfigPage::plugin() const
{
	return d->plugin;
}

void EngineConfigPage::readSettings()
{
	for (FilePickWidget *filePicker : d->filePickers)
	{
		filePicker->blockSignals(true);
		filePicker->load(*d->config);
		filePicker->blockSignals(false);
	}

	d->cboCustomParameters->clear();
	d->cboCustomParameters->addItems(d->readStoredCustomParameters());
	d->cboCustomParameters->setEditText(d->config->value("CustomParameters").toString());
	if (d->plugin->data()->hasMasterClient())
		d->leMasterserverAddress->setText(d->config->value("Masterserver").toString());

	updateCustomParametersSaveState();
}

void EngineConfigPage::removeCurrentCustomParametersFromStorage()
{
	QString currentParams = currentCustomParameters();
	removeStoredCustomParametersFromConfig(currentParams);
	removeStoredCustomParametersFromWidget(currentParams);
	updateCustomParametersSaveState();
}

void EngineConfigPage::removeStoredCustomParametersFromConfig(const QString &parameters)
{
	QStringList storedParameters = d->readStoredCustomParameters();
	storedParameters.removeAll(parameters);
	d->saveStoredCustomParameters(storedParameters);
}

void EngineConfigPage::removeStoredCustomParametersFromWidget(const QString &parameters)
{
	int indexInWidget = d->cboCustomParameters->findText(parameters);
	if (indexInWidget >= 0)
	{
		d->cboCustomParameters->removeItem(indexInWidget);
		d->cboCustomParameters->setEditText(parameters);
	}
}

void EngineConfigPage::saveCustomParameters()
{
	if (!d->existsInStoredCustomParameters(currentCustomParameters()))
	{
		QStringList parameters = d->readStoredCustomParameters();
		parameters << currentCustomParameters();
		d->saveStoredCustomParameters(parameters);
		d->cboCustomParameters->addItem(currentCustomParameters());
	}
	updateCustomParametersSaveState();
}

void EngineConfigPage::saveSettings()
{
	for (FilePickWidget *filePicker : d->filePickers)
	{
		filePicker->save(*d->config);
	}
	d->config->setValue("CustomParameters", currentCustomParameters());
	if (d->plugin->data()->hasMasterClient())
		d->config->setValue("Masterserver", d->leMasterserverAddress->text());
}

void EngineConfigPage::showError(const QString &error)
{
	d->lblError->setText(error);
	d->lblError->show();
	d->errorTimer.start();
}

void EngineConfigPage::showFindFailError()
{
	showError(tr("Failed to automatically find file.\nYou may need to use the browse button."));
}

QString EngineConfigPage::title() const
{
	return tr("Game - %1").arg(name());
}

void EngineConfigPage::updateCustomParametersSaveState()
{
	bool paramExists = d->existsInStoredCustomParameters(currentCustomParameters());
	bool isEmpty = currentCustomParameters().isEmpty();
	d->btnSaveCustomParameters->setVisible(!isEmpty && !paramExists);
	d->btnDeleteCustomParameters->setVisible(!isEmpty && paramExists);
}

ConfigPage::Validation EngineConfigPage::validate()
{
	bool allFilesOk = true;
	for (FilePickWidget *filePicker : d->filePickers)
	{
		bool thisFileOk = filePicker->validate();
		allFilesOk = allFilesOk && thisFileOk;
	}
	return allFilesOk ? VALIDATION_OK : VALIDATION_ERROR;
}
