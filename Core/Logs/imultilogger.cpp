#include <QString>

#include "imultilogger.h"

namespace Core {

IMultiLogger::IMultiLogger(const QString &tag_)
    : ILogger(tag_)
{

}

IMultiLogger::~IMultiLogger()
{
    for (auto &it : loggers)
        delete it;
}

void IMultiLogger::message(const QString &text)
{
    for (auto &it : loggers)
        it->message(text);
}

}
