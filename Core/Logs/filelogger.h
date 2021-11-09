#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <QFile>
#include <QPointer>

#include "imultilogger.h"

namespace Core {


/** @ingroup Logs
  * @brief Класс файлового логгера
  * @details Пишет сообщения в файл, имя файла будет иметь следующий вид: \n
  * LogDir/date/time_tag.csv,\n
  * LogDir берется из настроек (conf->getValue("Log/dir")) */
class FileLogger : public IMultiLogger
{
public:
    explicit FileLogger(const QString &tag);

    /// Имя класса логгера
    QString getClassName() const override;

protected:
    ///Обработка события вкл/откл записи лога
    void enableLog(bool value) override;

    ///Вывод сообщения в лог
    void showMessage(const QString &text) override;

private:
    std::unique_ptr<QFile> _logFile;
};

}

#endif // FILELOGGER_H
