//------------------------------------------------------------------------------
// settingsproviderqt.cpp
//------------------------------------------------------------------------------
// Copyright 2011 - 2013 Zalewa <zalewapl@gmail.com>. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    1. Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//
//    2. Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in
//       the documentation and/or other materials provided with
//       the distribution.
//
// THIS SOFTWARE IS PROVIDED BY ZALEWA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL ZALEWA OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
// WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
// OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation are
// those of the authors and should not be interpreted as representing official
// policies, either expressed or implied, of Zalewa.
//------------------------------------------------------------------------------
// Copyright (C) 2013 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "settingsproviderqt.h"

#include <cassert>

DClass<SettingsProviderQt>
{
public:
	QSettings *target;

	/**
	 * We want case-insensitivity, but QSettings
	 * is not always case-insensitive, so let's
	 * try to retrieve the exact key name.
	 */
	QString exactKey(const QString &key) const
	{
		assert(target != nullptr);
		for (const QString &candidate : target->allKeys())
		{
			if (candidate.compare(key, Qt::CaseInsensitive) == 0)
				return candidate;
		}
		return key;
	}
};

DPointered(SettingsProviderQt)

SettingsProviderQt::SettingsProviderQt(QSettings *target)
{
	d->target = target;
}

SettingsProviderQt::~SettingsProviderQt()
{
}

QStringList SettingsProviderQt::allKeys() const
{
	assert(d->target != nullptr);
	return d->target->allKeys();
}

QStringList SettingsProviderQt::allSections() const
{
	assert(d->target != nullptr);
	return d->target->childGroups();
}

bool SettingsProviderQt::hasKey(const QString &key) const
{
	return d->target->contains(d->exactKey(key));
}

void SettingsProviderQt::remove(const QString &key)
{
	d->target->remove(d->exactKey(key));
}

void SettingsProviderQt::setValue(const QString &key, const QVariant &value)
{
	assert(d->target != nullptr);
	d->target->setValue(d->exactKey(key), value);
}

QVariant SettingsProviderQt::value(const QString &key, QVariant defValue) const
{
	assert(d->target != nullptr);
	return d->target->value(d->exactKey(key), defValue);
}
