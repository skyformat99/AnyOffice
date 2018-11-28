#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include "mainwindow.h"

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);



    QTranslator translator;
    QLocale locale;
    switch (locale.language()) {
    case QLocale::Chinese:
        translator.load("zh-CN",":/i18n");
        break;
    default:
        break;
    }
a.installTranslator(&translator);

	MainWindow window;
	window.show();
	if (window.initComponents())
		return a.exec();
	return -1;
}
