/* Пояснения к классу IMultiLogger:
 * Например нам нужно писать warning лог в файл и в базу данных, вот так делать НЕ НУЖНО:
 *   FileLogger *fileLogger = new FileLogger("warning");
 *   DbLogger *dbLogger = new DbLogger("warning");
 *   ...
 *   fileLogger << "Error: this is bad code!";
 *   dbLogger << "Error: this is bad code!";
 * ПРАВИЛЬНО вот так:
 *   FileLogger *fileLogger = new FileLogger("warning");
 *   fileLogger->appendLogger<DbLogger>();
 *   ...
 *   fileLogger << "Attention: this is good code :)";
 * или
 *   DbLogger *dbLogger = new DbLogger("warning");
 *   dbLogger->appendLogger<FileLogger>();
 *   ...
 *   dbLogger << "Attention: this is good code :)";
 * Все 3 примера дадут одинаковый результат, сообщения разумеется будут записаны разные, но сообщение во всех 3 случаях окажутся и в файле и в БД. */
#ifndef IMULTILOGGER_H
#define IMULTILOGGER_H

#include <vector>
#include <QString>
#include <QDebug>

#include "ilogger.h"

namespace Core {


/** @ingroup Logs
  * @brief Базовый класс мульти логгера
  * @details tag - это уникальный идентификатор логгера, не нужно создавать 2 экземпляра ILogger с одинаковым tag.
  * Лучше создать 1 IMultiLogger с тэгом warning.
  * Данный класс может содержать в себе другие логгеры, и после обработки сообщения функцией message, сообщение text
  * будет передано во все логгеры, которые содержатся в данном классе.
  * Помимо функции message сообщения можно выводить перегруженным оператором <<, перегрузка реализована в ILogger. */
class IMultiLogger : public ILogger
{
public:
    IMultiLogger(const QString &tag_);
    ~IMultiLogger();

    /// Имя класса логгера
    QString getClassName() const override = 0;


    /// Включает/отключает запись текущего лога
    void setWriteLog(bool on) override;

    /** @brief Выводит сообщение text в лог
      * @warning Данную функцию нужно перегрузить, реализовав в ней обработку сообщения.\n
      * Для передачи сообще вложенным логгерам в конце реализации функции нужно вызвать @code
      * IMultiLogger::message(text);  * @endcode */
    void message(const QString &text) override;


    /** @brief Добавляет новый обработчик сообщений (волженный логгер)
      * @param[IN] T - класс логгера, наследник от IMultiLogger */
    template<class T>
    typename std::enable_if_t<std::is_base_of_v<IMultiLogger, T>>
    appendLogger()
    {
        QMutexLocker locker(&mutex);

        T *newLogger = new T(tag);
        QString newClass = newLogger->getClassName();
        auto count = std::count_if(loggers.begin(), loggers.end(), [&newClass] (auto &it) {return it->getClassName() == newClass;});

        //Нам не нужны 2 логера одного и того же класса (с одним и тем же тэгом)
        if (count > 0 || newClass == this->getClassName()) {  //оператор == перегружен в ILogger, он сравнивает getClassName()
            qDebug() << QString("Logger %1 (%2) is allready existed").arg(newClass, tag);
            delete newLogger;
        } else
            loggers.push_back(newLogger);
    }

protected:
    /// Список вложенных логгеров, которым так же будет передаваться сообщение для обработки
    std::vector<IMultiLogger*> loggers;
};

}

#endif // IMULTILOGGER_H
