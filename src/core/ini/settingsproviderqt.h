//------------------------------------------------------------------------------
// settingproviderqt.h
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
#ifndef idB4E0ED4E_CB13_4106_84A03E8713EF4A09
#define idB4E0ED4E_CB13_4106_84A03E8713EF4A09

#include "dptr.h"
#include "ini/settingsprovider.h"
#include <QSettings>

/**
 * @ingroup group_pluginapi
 */
class MAIN_EXPORT SettingsProviderQt : public SettingsProvider
{
public:
	SettingsProviderQt(QSettings *target);
	virtual ~SettingsProviderQt() override;

	QStringList allKeys() const override;
	QStringList allSections() const override;
	bool hasKey(const QString &key) const override;
	void remove(const QString &key) override;
	void setValue(const QString &key, const QVariant &value) override;
	QVariant value(const QString &key, QVariant defValue = QVariant()) const override;

private:
	DPtr<SettingsProviderQt> d;
};

#endif // header
