#include "lyrtmpClient.h"
#include <QtWidgets/QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication a(argc, argv);
	lyrtmpClient w;
	w.show();
	return a.exec();
}
