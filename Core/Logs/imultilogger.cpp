#include <QString>

#include "imultilogger.h"

namespace Core {

IMultiLogger::IMultiLogger(const QString &tag_)
    : ILogger(tag_)
{

}

IMultiLogger::~IMultiLogger()
{

}

void IMultiLogger::setWriteLog(bool on)
{
    enableLog(on);

    for (auto &it : loggers)
        it->setWriteLog(on);

    ILogger::setWriteLog(on);
}


void IMultiLogger::message(const QString &text)
{
    showMessage(text);

    for (auto &it : loggers)
        it->message(text);
}

void IMultiLogger::enableLog(bool on)
{
    Q_UNUSED(on);
}

}
