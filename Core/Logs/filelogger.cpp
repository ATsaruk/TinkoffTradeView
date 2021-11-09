#include <QDir>
#include <QDebug>
#include <QString>

#include "filelogger.h"
#include "../globals.h"

namespace Core {

FileLogger::FileLogger(const QString &tag)
    : IMultiLogger(tag), _logFile(new QFile)
{
    isWriteLog =      Glo.conf->getValue(QString("Log/%1").arg(tag), true);
    QString dirName = Glo.conf->getValue("Log/dir", QString("Logs"));

    QDir dir(dirName);
    dir.mkpath(QDate::currentDate().toString("yyyy_MM_dd"));

    QString fileName = dirName
            + QDate::currentDate().toString("/yyyy_MM_dd")
            + QTime::currentTime().toString("/hh_mm_ss")
            + QString("_%1.csv").arg(tag);

    _logFile->setFileName(fileName);
}

QString FileLogger::getClassName() const
{
    return "FileLogger";
}

//При выключении лога закрывает файл
void FileLogger::enableLog(bool value)
{
    if (value == false && _logFile->isOpen()) {
        QMutexLocker locker(&mutex);
        _logFile->close();
    }
}

void FileLogger::showMessage(const QString &text)
{
    if (!isWriteLog)
        return;

    QMutexLocker locker(&mutex);

    //Если файл не открыт, открываем его
    if (!_logFile->isOpen()) {
        _logFile->open(QIODevice::WriteOnly);
        if (!_logFile->isOpen()) {
            qDebug() << QString("Can't create file: %1, stop logging %2 messages to file...").arg(_logFile->fileName(), tag);
            isWriteLog = false;
            return;
        }
    }

    //Сохраняем сообщение
    QTextStream stream(_logFile.get());
    stream << QDateTime::currentDateTime().toString("yyyy.MM.dd;hh:mm:ss;%1;\n").arg(text);
    stream.flush();
}

}
