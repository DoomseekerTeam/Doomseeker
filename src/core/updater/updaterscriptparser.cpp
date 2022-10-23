//------------------------------------------------------------------------------
// updaterscriptparser.cpp
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
#include "updaterscriptparser.h"

#include "autoupdater.h"
#include <cassert>
#include <QDebug>

DClass<UpdaterScriptParser>
{
public:
	QDomDocument *pDoc;
};

DPointered(UpdaterScriptParser)
//////////////////////////////////////////////////////////////////////////////
UpdaterScriptParser::UpdaterScriptParser(QDomDocument &document)
{
	d->pDoc = &document;
}

UpdaterScriptParser::~UpdaterScriptParser()
{
}

QDomNode UpdaterScriptParser::installNode(const QDomDocument &doc)
{
	return doc.elementsByTagName("update").at(0).toElement()
		.elementsByTagName("install").at(0);
}

QDomElement UpdaterScriptParser::mainElement()
{
	QDomNodeList nodes = d->pDoc->elementsByTagName("update");
	return nodes.at(0).toElement();
}

void UpdaterScriptParser::merge(const QDomDocument &otherDoc)
{
	assert(d->pDoc != nullptr && "UpdaterScriptParser::merge()");
	if (!d->pDoc->isNull())
	{
		QDomNodeList listNodes;
		// We'll copy only the "packages" and "install" files.
		// We'll assume the rest is already set correctly in the
		// source doc.
		QDomNode nodePackagesDst = packagesNode(*d->pDoc);
		QDomNode nodePackagesSrc = packagesNode(otherDoc);
		listNodes = nodePackagesSrc.toElement().elementsByTagName("package");
		for (int i = 0; i < listNodes.count(); ++i)
		{
			nodePackagesDst.appendChild(listNodes.at(i).cloneNode(true));
		}

		QDomNode nodeInstallDst = installNode(*d->pDoc);
		QDomNode nodeInstallSrc = installNode(otherDoc);
		listNodes = nodeInstallSrc.toElement().elementsByTagName("file");
		for (int i = 0; i < listNodes.count(); ++i)
		{
			nodeInstallDst.appendChild(listNodes.at(i).cloneNode(true));
		}
	}
	else
	{
		*d->pDoc = otherDoc.cloneNode().toDocument();
	}
}

QString UpdaterScriptParser::msgMainElementMissingError()
{
	return AutoUpdater::tr("Missing main \"update\" node.");
}

QDomNode UpdaterScriptParser::packagesNode(const QDomDocument &doc)
{
	return doc.elementsByTagName("update").at(0).toElement()
		.elementsByTagName("packages").at(0);
}

QString UpdaterScriptParser::setPackageName(const QString &name)
{
	assert(d->pDoc != nullptr && "UpdaterScriptParser::setPackageName()");
	QString errorMsg;
	errorMsg = setPackageNameInPackages(name);
	if (errorMsg.isNull())
	{
		errorMsg = setPackageNameInFiles(name);
	}
	return errorMsg;
}

QString UpdaterScriptParser::setPackageNameInFiles(const QString &name)
{
	// Path: /"install"/"file"(*)/"package"/#text
	QDomNodeList nodes;
	QDomElement element;

	element = mainElement();
	if (element.isNull())
	{
		return msgMainElementMissingError();
	}
	element = element.elementsByTagName("install").at(0).toElement();
	if (element.isNull())
	{
		return AutoUpdater::tr("Missing \"install\" element.");
	}
	nodes = element.elementsByTagName("file");
	for (int i = 0; i < nodes.count(); ++i)
	{
		QDomNode textNode = nodes.at(i).toElement()
			.elementsByTagName("package").at(0).toElement().firstChild();
		textNode.setNodeValue(name);
		if (textNode.isNull())
		{
			return AutoUpdater::tr("Missing text node for \"package\" "
				"element for \"file\" element %1").arg(i);
		}
	}
	return QString();
}

QString UpdaterScriptParser::setPackageNameInPackages(const QString &name)
{
	// Path: /"packages"/"package"/"name"/#text
	QDomNodeList nodes;
	QDomElement element;

	element = mainElement();
	if (element.isNull())
	{
		return msgMainElementMissingError();
	}

	nodes = element.elementsByTagName("packages");
	if (nodes.isEmpty())
	{
		return AutoUpdater::tr("Missing \"packages\" element.");
	}
	element = nodes.at(0).toElement();
	// Now, Mendeley updater allows to have many packages defined in single
	// script file. Mendeley iterates over these packages and then uses
	// values of "name" elements to build filenames under which
	// packages archives are stored. Doomseeker will always use only one
	// package per script so if there is more than one element under "packages"
	// tag then this should be treated as an error in the script.
	nodes = element.elementsByTagName("package");
	if (nodes.size() != 1)
	{
		if (nodes.size() > 1)
		{
			return AutoUpdater::tr("More than one \"package\" element found.");
		}
		return AutoUpdater::tr("Missing \"package\" element.");
	}
	// Since we now know that there is only one "package" node
	// we can finally set its name.
	QDomNode nameTextNode = nodes.at(0).toElement()
		.elementsByTagName("name").at(0).toElement().firstChild();
	nameTextNode.setNodeValue(name);
	if (nameTextNode.isNull())
	{
		return AutoUpdater::tr("Failed to find \"name\" text node.");
	}
	return QString();
}
