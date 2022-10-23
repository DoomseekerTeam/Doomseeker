//------------------------------------------------------------------------------
// configurationdialog.h
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
// Copyright (C) 2009 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __CONFIGUREDIALOG_H_
#define __CONFIGUREDIALOG_H_

#include "dptr.h"
#include "gui/configuration/configpage.h"

#include <QDialog>

class QAbstractButton;
class QModelIndex;
class QStandardItem;
class QTreeView;

class ConfigurationDialog : public QDialog
{
	Q_OBJECT

public:
	ConfigurationDialog(QWidget *parent = nullptr);
	~ConfigurationDialog() override;

	/**
	 * @brief Adds a new configuration page to the options tree view.
	 *
	 * @param rootItem
	 *     Pointer to the root node of the tree to which the new box
	 *     will be attached. If nullptr, the standard model root is used.
	 * @param configPage
	 *     Pointer to ConfigPage instance.
	 * @param position
	 *     Passing <0 will use appendRow method, otherwise the new
	 *     tree view node will be inserted at specified position.
	 *
	 * @return nullptr if ConfigPage was not added. Pointer to a new
	 * tree node if operation was successful.
	 */
	virtual QStandardItem *addConfigPage(QStandardItem *rootItem, ConfigPage *configPage, int position = -1);

	/**
	 * @brief Adds a label node to the options tree view.
	 *
	 * Such node has no ConfigPage attached. It serves only
	 * organizational purposes.
	 *
	 * @param rootItem
	 *     Pointer to the root node of the tree to which the new label
	 *     will be attached. If nullptr, the standard model root is used.
	 * @param label
	 *     Label for the new node.
	 * @param position
	 *     Passing <0 will use appendRow method, otherwise the new
	 *     tree view node will be inserted at specified position.
	 *
	 * @return Newly created options tree view node. nullptr if rootItem
	 * was not a member of the tree view.
	 */
	QStandardItem *addLabel(QStandardItem *rootItem, const QString &label, int position = -1);

public slots:
	void reject() override;

protected:
	virtual void doSaveSettings() {}
	void keyPressEvent(QKeyEvent *e) override;

	/**
	 * @brief Returns pointer to the tree widget that contains
	 *        configuration sections list.
	 */
	QTreeView *optionsTree();

	/**
	 * @param widget - hide currently displayed box if nullptr.
	 */
	void showConfigPage(ConfigPage *widget);

	virtual bool validate()
	{
		return true;
	}

private:
	DPtr<ConfigurationDialog> d;

	bool canConfigPageBeAdded(ConfigPage *configPage);
	bool isConfigPageValid(ConfigPage *configPage);
	bool hasConfigPage(ConfigPage *configPage);
	bool hasItemOnList(QStandardItem *pItem) const;
	QModelIndex findPageModelIndex(ConfigPage *page);
	void saveSettings();
	void validatePage(ConfigPage *page);

private slots:
	void btnClicked(QAbstractButton *button);
	void onPageValidationRequested();
	void switchToItem(const QModelIndex &current, const QModelIndex &previous);
};

#endif
