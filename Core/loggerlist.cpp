#include "loggerlist.h"

#include <QtGlobal>

namespace Core {

LoggerList::LoggerList()
{

}

LoggerList::~LoggerList()
{

}

std::shared_ptr<ILogger> LoggerList::get(const QString &tag)
{
    return get<defaultLogger>(tag);
}

}
