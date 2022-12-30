//------------------------------------------------------------------------------
// cfgircsounds.cpp
//
// Copyright (C) 2011 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "cfgircsounds.h"
#include "ui_cfgircsounds.h"

#include "irc/configuration/ircconfig.h"
#include "templatedpathresolver.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QSoundEffect>
#include <QStyle>

DClass<CFGIRCSounds> : public Ui::CFGIRCSounds
{
public:
	QSoundEffect *sfx;
};

DPointered(CFGIRCSounds)

CFGIRCSounds::CFGIRCSounds(QWidget *parent)
	: ConfigPage(parent)
{
	d->sfx = new QSoundEffect(this);

	d->setupUi(this);
	d->lblNicknameUsedWarning->hide();
	d->lblPrivateMessageWarning->hide();
	d->btnBrowseNicknameUsed->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
	d->btnBrowsePrivateMessage->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));

	this->connect(d->cbNicknameUsed, SIGNAL(toggled(bool)),
		SIGNAL(validationRequested()));
	this->connect(d->leNicknameUsed, SIGNAL(editingFinished()),
		SIGNAL(validationRequested()));
	this->connect(d->cbPrivateMessage, SIGNAL(toggled(bool)),
		SIGNAL(validationRequested()));
	this->connect(d->lePrivateMessage, SIGNAL(editingFinished()),
		SIGNAL(validationRequested()));
}

CFGIRCSounds::~CFGIRCSounds()
{
}

void CFGIRCSounds::browseNicknameUsed()
{
	setPath(d->leNicknameUsed, getPathToWav());
}

void CFGIRCSounds::browsePrivateMessage()
{
	setPath(d->lePrivateMessage, getPathToWav());
}

QString CFGIRCSounds::getPathToWav()
{
	return QFileDialog::getOpenFileName(this, tr("Pick Sound File"),
		QString(),
		tr("WAVE (*.wav)"));
}

void CFGIRCSounds::playNicknameUsed()
{
	playSound(d->leNicknameUsed->text());
}

void CFGIRCSounds::playPrivateMessage()
{
	playSound(d->lePrivateMessage->text());
}

void CFGIRCSounds::playSound(const QString &path) const
{
	QFileInfo file(gDoomseekerTemplatedPathResolver().resolve(path));
	if (file.exists())
	{
		d->sfx->setSource(QUrl::fromLocalFile(file.absoluteFilePath()));
		d->sfx->play();
	}
}

void CFGIRCSounds::readSettings()
{
	d->cbNicknameUsed->setChecked(gIRCConfig.sounds.bUseNicknameUsedSound);
	d->cbPrivateMessage->setChecked(gIRCConfig.sounds.bUsePrivateMessageReceivedSound);

	d->leNicknameUsed->setText(gIRCConfig.sounds.nicknameUsedSound);
	d->lePrivateMessage->setText(gIRCConfig.sounds.privateMessageReceivedSound);
}

void CFGIRCSounds::saveSettings()
{
	gIRCConfig.sounds.bUseNicknameUsedSound = d->cbNicknameUsed->isChecked();
	gIRCConfig.sounds.nicknameUsedSound = d->leNicknameUsed->text().trimmed();

	gIRCConfig.sounds.bUsePrivateMessageReceivedSound = d->cbPrivateMessage->isChecked();
	gIRCConfig.sounds.privateMessageReceivedSound = d->lePrivateMessage->text().trimmed();
}

void CFGIRCSounds::setPath(QLineEdit *pLineEdit, const QString &path)
{
	QString trimmedPath = path.trimmed();
	if (!trimmedPath.isEmpty())
		pLineEdit->setText(trimmedPath);
	emit validationRequested();
}

ConfigPage::Validation CFGIRCSounds::validate()
{
	bool error = false;
	QString nicknameUsedError;
	if (d->cbNicknameUsed->isChecked())
		nicknameUsedError = validateFilePath(d->leNicknameUsed->text().trimmed());
	d->lblNicknameUsedWarning->setVisible(!nicknameUsedError.isEmpty());
	d->lblNicknameUsedWarning->setToolTip(nicknameUsedError);
	error = error || !nicknameUsedError.isEmpty();

	QString privateMessageError;
	if (d->cbPrivateMessage->isChecked())
		privateMessageError = validateFilePath(d->lePrivateMessage->text().trimmed());
	d->lblPrivateMessageWarning->setVisible(!privateMessageError.isEmpty());
	d->lblPrivateMessageWarning->setToolTip(privateMessageError);
	error = error || !privateMessageError.isEmpty();

	return !error ? VALIDATION_OK : VALIDATION_ERROR;
}

QString CFGIRCSounds::validateFilePath(QString path) const
{
	path = gDoomseekerTemplatedPathResolver().resolve(path);
	if (path.trimmed().isEmpty())
		return tr("No path specified.");

	QFileInfo fileInfo(path.trimmed());
	if (!fileInfo.exists())
		return tr("File doesn't exist.");

	if (!fileInfo.isFile())
		return tr("This is not a file.");
	return "";
}
