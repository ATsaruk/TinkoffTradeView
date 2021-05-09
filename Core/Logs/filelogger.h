#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <QPointer>

class QFile;

#include "imultilogger.h"

namespace Core {


/** @ingroup Logs
  * @brief Класс файлового логгера
  * @details Пишет сообщения в файл, имя файла будет иметь следующий вид: \n
  * LogDir/date/time_tag.csv,\n LogDir берется из настроек (conf->getValue("Log/dir")) */
class FileLogger : public IMultiLogger
{
public:
    explicit FileLogger(const QString &tag);

    /// Имя класса логгера
    QString getClassName() const override;

    void setWriteLog(bool value) override;
    void message(const QString &text) override;

private:
    QPointer<QFile> logFile;
};

}

#endif // FILELOGGER_H
