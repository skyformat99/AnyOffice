#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QMimeDatabase>
#include <QDir.h>
#include "officetools.h"

#define TEMPLATES_DIR "templates"

namespace Ui
{
	class MainWindow;
}

class MainWindow : public QWidget
{
Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = nullptr);
	~MainWindow();
	bool initComponents();
	bool eventFilter(QObject* watched, QEvent* event) override;
private:
	Ui::MainWindow* ui;
	QMimeDatabase mine;
	QDir applicationDir;
	QDir templatesDir;
	OfficeTools *office;
	
	void flushTemplates();
	bool removeDirectory(const QString& path);
	void extractTemplate(const QString& path);
	//void archiveTemplate(const QString& url);
	//void receivedDrop(const QString& url);
	void processData(const QStringList& list);
	void receivedDrop(const QStringList list);
};

#endif // MAINWINDOW_H
