#include <QFile>
#include <QApplication>

#include "mainwindow.h"
#include "Core/globals.h"

int main(int argc, char *argv[])
{
    try {
        QApplication a(argc, argv);

        QFile file("dark.qss");
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream stream(&file);
        a.setStyleSheet(stream.readAll());
        file.close();

        MainWindow w;
        w.show();
        return a.exec();
    }
    catch (std::exception &except) {
        logCritical << except.what();
        qDebug() << except.what();
    }
    return 0;
}
