#include <QDir>
#include <QDebug>
#include <QString>

#include "filelogger.h"
#include "../globals.h"

namespace Core {

FileLogger::FileLogger(const QString &tag)
    : IMultiLogger(tag)
{
    isWriteLog = Glo.conf->getValue(QString("Log/%1").arg(tag), QVariant(true)).toBool();

    QString dirName = Glo.conf->getValue("Log/dir", QVariant("Logs")).toString();

    QDir dir(dirName);
    dir.mkpath(QDate::currentDate().toString("yyyy_MM_dd"));

    QString fileName = dirName
            + QDate::currentDate().toString("/yyyy_MM_dd")
            + QTime::currentTime().toString("/hh_mm_ss")
            + QString("_%1.csv").arg(tag);

    logFile = new QFile(fileName);
}

FileLogger::~FileLogger()
{
    if(logFile->isOpen())
        logFile->close();
    delete logFile;
}

QString FileLogger::getClassName() const
{
    return "FileLogger";
}

void FileLogger::setWriteLog(bool value)
{
    if (value == isWriteLog)
        return;

    IMultiLogger::setWriteLog(value);

    QMutexLocker locker(&mutex);
    if (value == false) {
        if (logFile->isOpen())
            logFile->close();
    }
}

void FileLogger::message(const QString &text)
{
    if (!isWriteLog)
        return;

    IMultiLogger::message(text);

    QMutexLocker locker(&mutex);
    if (!logFile->isOpen()) {
        logFile->open(QIODevice::WriteOnly);
        if (!logFile->isOpen()) {
            qDebug() << QString("Can't create file: %1, stop logging %2 messages to file...").arg(logFile->fileName(), tag);
            isWriteLog = false;
            return;
        }
    }

    QTextStream stream(logFile);

    stream << QDateTime::currentDateTime().toString("yyyy.MM.dd;hh:mm:ss;%1;\n").arg(text);
    stream.flush();
}

}
