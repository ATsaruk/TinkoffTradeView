#ifndef MSGBOXLOGGER_H
#define MSGBOXLOGGER_H

#include <QString>

#include "imultilogger.h"

namespace Core {

/** @ingroup Logs
  * @brief Класс MessageBox логгера
  * @details Выводит все сообщения в MessageBox'е, не рекомендуется использовать с debug и warning */
class MsgBoxLogger : public IMultiLogger
{
public:
    explicit MsgBoxLogger(const QString &tag_);

    /// Имя класса логгера
    QString getClassName() const override;

    void message(const QString &text) override;

private:
    QString tag;
};

}

#endif // MSGBOXLOGGER_H
