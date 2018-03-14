#include "PrecizaGUI.h"
#include <QtWidgets/QApplication>
#include <iostream>


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	PrecizaGUI w;

	//MyClass w;
	w.show();
	return a.exec();
}
