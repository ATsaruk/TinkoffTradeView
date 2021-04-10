#include "ilogger.h"

namespace Core {

ILogger::ILogger(const QString &tag_)
    : tag(tag_)
{

}

ILogger::~ILogger()
{

}

const QString &ILogger::getTag() const
{
    return tag;
}

void ILogger::setWriteLog(bool on)
{
    isWriteLog = on;
}

ILogger *ILogger::operator<<(const QString &text)
{
    message(text);
    return this;
}

}
