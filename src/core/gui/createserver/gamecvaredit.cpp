//------------------------------------------------------------------------------
// gamecvaredit.cpp
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
// Copyright (C) 2021 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "gamecvaredit.h"

#include "serverapi/serverstructs.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QLineEdit>

#include <climits>
#include <memory>

namespace
{
class Edit
{
public:
	virtual bool externalLabel() const { return true; }
	virtual void setValue(const QVariant &value) = 0;
	virtual QVariant value() const = 0;
	virtual QWidget *widget() const = 0;
};

class BoolEdit final : public Edit
{
public:
	BoolEdit(const GameCVar &cvar, QWidget *parent)
		: edit(new QCheckBox(parent))
	{
		edit->setText(cvar.name());
		edit->setChecked(cvar.value().toBool());
	}

	bool externalLabel() const override { return false; }

	void setValue(const QVariant &value) override
	{
		edit->setChecked(value.toBool());
	}

	QVariant value() const override
	{
		return edit->isChecked();
	}

	QWidget *widget() const override
	{
		return edit.get();
	}

private:
	std::unique_ptr<QCheckBox> edit;
};

class IntEdit final : public Edit
{
public:
	IntEdit(const GameCVar &cvar, QWidget *parent)
		: edit(new QSpinBox(parent))
	{
		edit->setMaximum(INT_MAX);
		edit->setMinimum(INT_MIN);
		edit->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
		edit->setValue(cvar.value().toInt());
	}

	void setValue(const QVariant &value) override
	{
		edit->setValue(value.toInt());
	}

	QVariant value() const override
	{
		return edit->value();
	}

	QWidget *widget() const override
	{
		return edit.get();
	}

private:
	std::unique_ptr<QSpinBox> edit;
};

class StringEdit final : public Edit
{
public:
	StringEdit(const GameCVar &cvar, QWidget *parent)
		: edit(new QLineEdit(parent))
	{
		edit->setText(cvar.value().toString());
	}

	void setValue(const QVariant &value) override
	{
		edit->setText(value.toString());
	}

	QVariant value() const override
	{
		return edit->text();
	}

	QWidget *widget() const override
	{
		return edit.get();
	}

private:
	std::unique_ptr<QLineEdit> edit;
};
}
DClass<GameCVarEdit>
{
public:
	GameCVar cvar;
	std::unique_ptr<Edit> edit;
};

DPointeredNoCopy(GameCVarEdit)

GameCVarEdit::GameCVarEdit(GameCVar cvar, QWidget *parent)
	: QWidget(parent)
{
	d->cvar = cvar;
	auto layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	switch (cvar.value().type())
	{
	case QMetaType::Bool:
		d->edit.reset(new BoolEdit(cvar, this));
		break;
	case QMetaType::Int:
	case QMetaType::UInt:
	case QMetaType::LongLong:
	case QMetaType::ULongLong:
		d->edit.reset(new IntEdit(cvar, this));
		break;
	default:
		d->edit.reset(new StringEdit(cvar, this));
		break;
	}
	layout->addWidget(d->edit->widget());
}

GameCVar GameCVarEdit::cvar() const
{
	auto cvar = d->cvar;
	cvar.setValue(value());
	return cvar;
}

bool GameCVarEdit::externalLabel() const
{
	return d->edit->externalLabel();
}

void GameCVarEdit::setValue(const QVariant &value)
{
	d->edit->setValue(value);
}

QVariant GameCVarEdit::value() const
{
	return d->edit->value();
}
