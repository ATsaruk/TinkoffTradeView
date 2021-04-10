#include "loggerlist.h"

#include <QtGlobal>

namespace Core {

LoggerList::LoggerList()
{

}

LoggerList::~LoggerList()
{
    for (auto &it : logs)
        delete it.second;
}

ILogger *LoggerList::get(QString tag)
{
    return get<defaultLogger>(tag);
}

}
