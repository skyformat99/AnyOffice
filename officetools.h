#ifndef OFFICETOOLS_H
#define OFFICETOOLS_H

#include <QFile>
#include <QDir>

#define OFFICE_CODEC "UTF-8"

class OfficeTools : QObject
{
	Q_OBJECT
public:
	OfficeTools(QDir applicationDir,QObject* parent = Q_NULLPTR);
	bool init() const;
	void formatXML(QFile& file,bool format = true);
	void formatDir(QDir& dir,bool format = true);
    bool extractOffice(const QString &srcPath, const QDir &destPath);
	void render(const QFile& file, const QStringList& list);
	void renderTemplate(QDir& dir, const QStringList& list, QStringList& renderList);
    bool archiveOffice(const QDir &srcPath, const QString &destPath,const QStringList &dataList);
private:
	QFile *command7z = nullptr;
	QFile *commandPhp = nullptr;
};

#endif // OFFICETOOLS_H
