/** @ingroup Core
  * @defgroup Logs Модуль классов предназначенных для ведения логов */

#ifndef ILOGGER_H
#define ILOGGER_H

#include <QMutex>
#include <QString>

namespace Core {


/** @ingroup Logs
  * @brief Базоый класс логгера
  * @warning Не нужно от него наследоваться, это класс интерфейс!, наследуйтесь от IMultiLogger */
class ILogger
{
public:
    explicit ILogger(const QString &tag_);
    virtual ~ILogger();

    /// Включает/отключает запись текущего лога
    virtual void setWriteLog(bool on);

    /// Вывод сообщений в лог
    virtual void message(const QString &text) = 0;
    virtual ILogger &operator <<(const QString &text);

protected:
    QMutex mutex;
    QString tag;        //тэг лога (например: info, warning, critical, debug, и т.д.)
    bool isWriteLog;    //признак писать текущий лог или нет (нужно для включения и отключения логов на runtime)
};

}

#endif // ILOGGER_H
