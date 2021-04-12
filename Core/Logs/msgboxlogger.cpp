#include <QApplication>
#include <QMessageBox>

#include "msgboxlogger.h"

namespace Core {

MsgBoxLogger::MsgBoxLogger(const QString &tag_)
    : IMultiLogger(tag_), tag(tag_)
{
}

QString MsgBoxLogger::getClassName() const
{
    return "MsgBoxLogger";
}

void MsgBoxLogger::message(const QString &text)
{
    IMultiLogger::message(text);

    QMutexLocker loker(&mutex);
    if (tag.toLower() == QString("critical"))
        QMessageBox::critical(0, tag, text);
    else if (tag.toLower() == QString("warning"))
        QMessageBox::warning(0, tag, text);
    else
        QMessageBox::information(0, tag, text);
}

}
