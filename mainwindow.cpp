#include <QDragEnterEvent>
#include <QMimeData>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->label->installEventFilter(this);
	ui->label->setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::flushTemplates()
{
	ui->templateBox->clear();
	QDir dir = templatesDir;
	dir.setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
    foreach(auto entry, dir.entryInfoList())
        ui->templateBox->addItem(entry.fileName());
}

bool MainWindow::initComponents()
{
	applicationDir = QDir(QApplication::applicationDirPath());
	templatesDir = QDir(QApplication::applicationDirPath());
    // check directory status
    if(!templatesDir.exists(TEMPLATES_DIR)){
        if(!templatesDir.mkdir(TEMPLATES_DIR)){
            QMessageBox::critical(this,tr("Error"),tr("Unable to create template directory."));
            return false;
        }
    }
    // can't change to templates dir
    if(!templatesDir.cd(TEMPLATES_DIR)){
        QMessageBox::critical(this,tr("Error"),tr("Unable to create template directory."));
        return false;
    }
	if (!(office = new OfficeTools(applicationDir, this))->init())
	{
        QMessageBox::critical(this, tr("Error"), tr("Unable to find PHP or 7z command line executable file."));
		return false;
	}
	this->flushTemplates();
	return true;
}

bool MainWindow::removeDirectory(const QString& path)
{
	if (path.isEmpty())
		return false;
	QDir dir(path);
	if (!dir.exists())
		return true;
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
	foreach(auto entry, dir.entryInfoList())
	{
		if (entry.isFile() && !entry.dir().remove(entry.fileName()))
			return false;
		else if (!this->removeDirectory(entry.absoluteFilePath()))
			return false;
	}
	return dir.rmdir(dir.absolutePath());
}

void MainWindow::extractTemplate(const QString& path)
{
	QFileInfo info(path);
	QDir extractDir = templatesDir;
	QString extractName = info.fileName();
	if (extractDir.exists(extractName))
	{
        if (QMessageBox::warning(this, tr("Warning"), tr("The template already exists. Do you want to override it?"),
                                 QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok)
            return;
		// remove directory
		if (!this->removeDirectory(extractDir.absoluteFilePath(extractName)))
		{
            QMessageBox::critical(this, tr("Error"), tr("Unable to delete template %1.").arg(extractName));
			return;
		}
	}
	if (!(extractDir.mkdir(extractName) && extractDir.cd(extractName)))
	{
        QMessageBox::critical(this, tr("Error"), tr("Unable to create template %1 directory.").arg(extractName));
		return;
	}
	if (!this->office->extractOffice(path, extractDir))
	{
        QMessageBox::critical(this, tr("Error"), tr("Unable to unpack template %1").arg(extractName));
	}
	else
	{
        QMessageBox::information(this, tr("Success"), tr("Template successfully created."));
	}
}

void MainWindow::processData(const QStringList& list)
{
	QDir tmpDir = templatesDir;
	if (!tmpDir.cd(ui->templateBox->currentText()))
	{
        QMessageBox::critical(this, tr("Error"), tr("Unable to find template %1.").arg(ui->templateBox->currentText()));
		return;
	}
	// gen output file path
	QFileInfo tempInfo(tmpDir.absolutePath());
	QFileInfo dataInfo(list.at(0));
	QString dest = "%1%2%3.%4";
	dest = dest.arg(dataInfo.absolutePath())
		.arg(QDir::separator())
		.arg(dataInfo.baseName())
		.arg(tempInfo.suffix());
	if (!office->archiveOffice(tmpDir, dest, list))
        QMessageBox::critical(this, tr("Error"), tr("Error rendering template. Unable to export document."));
	else
        QMessageBox::information(this, tr("Success"), tr("Output document success."));
}

void MainWindow::receivedDrop(const QStringList list)
{
	QStringList templateList;
	QStringList dataList;
	for (auto ptr = list.cbegin(); ptr != list.cend(); ++ptr)
	{
		QFileInfo info(*ptr);
		if (!info.isFile())
		{
			QMessageBox::critical(this, tr("Error"),
                                  tr("You can't drag and drop directories here! (%1)").arg(info.absoluteFilePath()));
			continue;
		}
		auto mimeType = this->mine.mimeTypeForFile(info);
		if (mimeType.name().contains("application/vnd.openxmlformats-officedocument"))
			templateList << *ptr;
		else
			dataList << *ptr;
	}
	if (!templateList.isEmpty())
	{
		if (!dataList.isEmpty())
		{
			QMessageBox::critical(this, tr("Error"), tr("Unable to identify the file type you dragged."));
			return;
		}
		for (auto ptr = templateList.cbegin(); ptr != templateList.cend(); ++ptr)
			this->extractTemplate(*ptr);
		this->flushTemplates();
	}
	else
	{
		if (list.size() > 0)
			this->processData(dataList);
	}
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == ui->label)
	{
		if (event->type() == QEvent::DragEnter)
		{
			auto drag = reinterpret_cast<QDragEnterEvent *>(event);
			drag->acceptProposedAction();
			return true;
		}
		if (event->type() == QEvent::Drop)
		{
			auto drop = reinterpret_cast<QDropEvent*>(event);
			auto urls = drop->mimeData()->urls();
			QStringList list;

			foreach(auto url,urls)
                list << url.toLocalFile();
			if (list.isEmpty())
				return true;
			this->receivedDrop(list);
			return true;
		}
	}
	return QWidget::eventFilter(watched, event);
}
