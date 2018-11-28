#include "officetools.h"
#include <QProcess>
#include <QApplication>
#include <QTextStream>
#include <QXmlStreamWriter>


OfficeTools::OfficeTools(QDir applicationDir, QObject* parent)
{
#if defined(_WIN32)
	if (applicationDir.cd("utils"))
	{
		this->command7z = new QFile(applicationDir.filePath("7z.exe"), parent);
		this->commandPhp = new QFile(applicationDir.filePath("php.exe"), parent);
	}
#else
#error unsupport platform
#endif
}

bool OfficeTools::init() const
{
	if (this->command7z != nullptr && this->commandPhp != nullptr)
		return this->command7z->exists() && this->commandPhp->exists();
	return false;
}

void OfficeTools::formatXML(QFile& file, bool format)
{
	if (file.open(QFile::ReadOnly))
	{
		qDebug(QStringLiteral("Format %1").arg(file.fileName()).toStdString().c_str());
		QTextStream readStream(&file);
		readStream.setCodec(OFFICE_CODEC);
		auto content = readStream.readAll();
		QXmlStreamReader reader(content);
		file.close();
		if (file.open(QFile::WriteOnly))
		{
			QXmlStreamWriter writer(&file);
			writer.setCodec(readStream.codec());
			writer.setAutoFormatting(format);
			writer.setAutoFormattingIndent(format ? 2 : 0);

			while (!reader.atEnd())
			{
				reader.readNext();
				if (reader.tokenType() != reader.Invalid)
				{
					if (!reader.isWhitespace())
					{
						writer.writeCurrentToken(reader);
					}
				}
			}
			file.close();
		}
	}
}

void OfficeTools::formatDir(QDir& dir, bool format)
{
	if (!dir.exists())
		return;
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
	foreach(auto entry, dir.entryInfoList())
	{
		if (entry.isFile() && entry.fileName().endsWith(".xml") && entry.isWritable())
		{
			QFile file(entry.absoluteFilePath());
			this->formatXML(file, format);
		}
		else if (entry.isDir())
		{
			QDir entryDir(entry.absoluteFilePath());
			this->formatDir(entryDir, format);
		}
	}
}

bool OfficeTools::extractOffice(const QString& srcPath, const QDir& destPath)
{
	QStringList args;
	args << "x" << srcPath.toStdString().c_str() << QStringLiteral("-o%1").arg(destPath.absolutePath());

	QProcess p(this);
	p.start(QFileInfo(*this->command7z).absoluteFilePath(), args);

	if (!p.waitForStarted() || !p.waitForFinished())
		return false;
	qDebug(p.readAll().toStdString().c_str());
	if (p.exitCode() == 0)
	{
		QDir dir(destPath.absolutePath());
		this->formatDir(dir);
		return true;
	}
	return false;
}

void OfficeTools::render(const QFile& file, const QStringList& list)
{
	QFileInfo info(file);
	QString outputName = info.fileName().left(info.fileName().length() - 4);
	QFile output(info.absoluteDir().absolutePath() + QDir::separator() + outputName);
	// if (output.open(QFile::WriteOnly))
	// {
	QStringList args;
	args << info.absoluteFilePath() << list;
	QProcess p(this);
	p.start(QFileInfo(*this->commandPhp).absoluteFilePath(), args);
	if (!p.waitForStarted() || !p.waitForFinished())
		return; // TODO: process
	QString outData = p.readAll();
	if (output.open(QFile::WriteOnly))
	{
		QTextStream outStream(&output);
		outStream.setCodec(OFFICE_CODEC);
		outStream << outData;
		output.close();
	}
	// }
}

void OfficeTools::renderTemplate(QDir& dir, const QStringList& list, QStringList& renderList)
{
	if (!dir.exists())
		return;
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
	foreach(auto entry, dir.entryInfoList())
	{
		if (entry.isFile() && entry.fileName().endsWith(".php") && entry.isWritable())
		{
			QFile file(entry.absoluteFilePath());
			this->render(file, list);
			renderList << entry.fileName();
		}
		else if (entry.isDir())
		{
			QDir entryDir(entry.absoluteFilePath());
			this->renderTemplate(entryDir, list, renderList);
		}
	}
}

bool OfficeTools::archiveOffice(const QDir& srcPath, const QString& destPath, const QStringList& dataList)
{
	QDir dir = srcPath;
	QStringList renderList;
	this->renderTemplate(dir, dataList, renderList);
	// archive office file

	QStringList args;
	args << QStringLiteral("a") << QStringLiteral("-tzip")
		<< destPath
		<< srcPath.absolutePath() + QDir::separator() + "*" << QStringLiteral("-xr0!*.php");

	foreach(auto s,args)
	{
		qDebug(s.toStdString().c_str());
	}

	QProcess p(this);
	p.start(QFileInfo(*this->command7z).absoluteFilePath(), args);

	if (!p.waitForStarted() || !p.waitForFinished())
		return false;
	qDebug(p.readAll().toStdString().c_str());
	return p.exitCode() == 0;
}
