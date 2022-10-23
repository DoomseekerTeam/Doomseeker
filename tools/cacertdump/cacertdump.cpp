// Public Domain. Authored by Zalewa <zalewapl@gmail.com>, 2017.
#include <cstdio>
#include <QDebug>
#include <QFile>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QTextStream>

int main(int argc, char **argv)
{
	QTextStream out(stdout);
	qDebug() << "Dumping " << QSslConfiguration::systemCaCertificates().size() << "certificates";
	for (const QSslCertificate &cert : QSslConfiguration::systemCaCertificates())
	{
		out << "BEGIN CERT" << endl;
		out << cert.toText() << " " << cert.toPem();
		out << "END CERT" << endl;
	}
}
