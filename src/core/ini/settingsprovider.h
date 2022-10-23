//------------------------------------------------------------------------------
// settingsprovider.h
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
#ifndef idEB87F251C_EC08_4EFE_9DC1754821652427
#define idEB87F251C_EC08_4EFE_9DC1754821652427

#include "dptr.h"
#include "global.h"

#include <QString>
#include <QStringList>
#include <QVariant>

/**
 * @ingroup group_pluginapi
 */
class MAIN_EXPORT SettingsProvider
{
public:
	SettingsProvider();
	virtual ~SettingsProvider();

	virtual QStringList allKeys() const = 0;
	virtual QStringList allSections() const = 0;
	virtual bool hasKey(const QString &key) const = 0;
	virtual void remove(const QString &key) = 0;
	virtual void setValue(const QString &key, const QVariant &value) = 0;
	virtual QVariant value(const QString &key, QVariant defValue = QVariant()) const = 0;

	/**
	 * @brief Copies all settings from 'other' to 'this'.
	 *
	 * Duplicate settings are replaced.
	 */
	void copyFrom(const SettingsProvider &other);

private:
	DPtr<SettingsProvider> d;
};

#endif // header
