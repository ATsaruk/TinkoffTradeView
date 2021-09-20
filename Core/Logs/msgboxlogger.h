#ifndef MSGBOXLOGGER_H
#define MSGBOXLOGGER_H

#include <QString>

#include "imultilogger.h"

namespace Core {

/** @ingroup Logs
  * @brief Класс MessageBox логгера
  * @details Выводит все сообщения в MessageBox'е, не рекомендуется использовать с debug и info! */
class MsgBoxLogger : public IMultiLogger
{
public:
    explicit MsgBoxLogger(const QString &tag_);

    ///Имя класса логгера
    QString getClassName() const override;

protected:
    ///Вывод сообщения в лог
    void showMessage(const QString &text) override;
};

}

#endif // MSGBOXLOGGER_H
