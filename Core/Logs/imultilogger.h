/* Пояснения к классу IMultiLogger:
 * Например нам нужно писать warning лог в файл, базу данных и на экран, вот так делать НЕ НУЖНО:
 *   auto fileLogger = new FileLogger("warning");
 *   auto dbLogger = new DbLogger("warning");
 *   auto msgBoxLogger = new MsgBoxLogger("warning");
 *   ...
 *   fileLogger << "Error: this is bad code!";
 *   dbLogger << "Error: this is bad code!";
 *   msgBoxLogger << "Error: this is bad code!";
 * ПРАВИЛЬНО вот так:
 *   auto fileLogger = new FileLogger("warning");
 *   fileLogger->appendLogger<DbLogger>();
 *   fileLogger->appendLogger<MsgBoxLogger>();
 *   ...
 *   fileLogger << "Attention: this is good code :)";
 * или
 *   auto dbLogger = new DbLogger("warning");
 *   dbLogger->appendLogger<FileLogger>();
 *   dbLogger->appendLogger<MsgBoxLogger>();
 *   ...
 *   dbLogger << "Attention: this is good code :)";
 * Все 3 примера дадут одинаковый результат, сообщения разумеется будут записаны разные,
 * но сообщение во всех 3 случаях окажутся в файле, в БД и на экране. */

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
  * Помимо функции message сообщения можно выводить перегруженным оператором <<, перегрузка реализована в ILogger.
  * Для наследования от данного класса достаточно перегрузить функции getClassName и showMessage*/
class IMultiLogger : public ILogger
{
public:
    explicit IMultiLogger(const QString &tag_);
    ~IMultiLogger();

    /** @brief Включает/отключает запись текущего лога, а так же вложенных логов (логов с тем же тэгом)
     *  @param on - вкл/откл ведение лога */
    void setWriteLog(bool on) override final;


    /** @brief Выводит сообщение text в текущий и во все вложенные логи!
     *  @param text - текст сообщения */
    void message(const QString &text) override final;


    /// Имя класса логгера
    virtual QString getClassName() const = 0;


    /** @brief Добавляет новый обработчик сообщений (волженный логгер)
      * @param[IN] T - класс логгера, наследник от IMultiLogger */
    template<class T>
    typename std::enable_if_t<std::is_base_of_v<IMultiLogger, T>>
    appendLogger()
    {
        QMutexLocker locker(&mutex);

        std::shared_ptr<T> newLogger = std::make_shared<T>(tag);
        QString newClass = newLogger->getClassName();
        auto count = std::count_if(loggers.begin(), loggers.end(), [&newClass] (auto &it) {return it->getClassName() == newClass;});

        //Нам не нужны 2 логера одного и того же класса (с одним и тем же тэгом)
        if (count > 0 || newClass == this->getClassName())   //оператор == перегружен в ILogger, он сравнивает getClassName()
            qDebug() << QString("Logger %1 (%2) is allready existed").arg(newClass, tag);
        else
            loggers.push_back(newLogger);
    }

protected:
    /// Список вложенных логгеров, которым так же будет передаваться сообщение для обработки
    std::vector<std::shared_ptr<IMultiLogger>> loggers;

    /** @brief Вызывается при вызове функции setWriteLog(bool)
     *  @param on - признак вкл/откл запись лога
     *
     *      Если при отклыючении/включении лога нужно произвести какие то дополнительные действия, то нужно перегрузить
     *  данную функцию, например для FileLogger при отключении можно закрыть файл.
     *  параметр isWriteLog изменять в данной функции не нужно, это будет сделано в setWriteLog
     *      Функция не является виртуальной т.к. для большенства случаев переопределение её не потребуется */
    virtual void enableLog(bool on);

    /** @brief showMessage Вызывается при вызове функции message()
     *  @param text - текст, который нужно сохранить в log
     *
     *      В данной функции нужно реализовать вывод сообщения в log */
    virtual void showMessage(const QString &text) = 0;
};

}

#endif // IMULTILOGGER_H
