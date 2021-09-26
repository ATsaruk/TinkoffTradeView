#include "globals.h"

#include "Logs/filelogger.h"
#include "Logs/msgboxlogger.h"
#include "DataBase/postgresql.h"
#include "Broker/Tinkoff/tinkoff.h"

namespace Core {

Globals &Globals::get()
{
    static Globals instance;
    return instance;
}

Globals::Globals()
{
    logTags << "debug" << "info" << "warning" << "critical";
}

void Globals::init()
{
    conf.reset( new Config("config.cfg") );

    logger.reset( new LoggerList );
    for (const auto &it : logTags)
        logger->add(it);            //Добавление FileLogger's

    logger->add<MsgBoxLogger>(logTags[2]);  // Что бы не пропустить warning выводим его дополнительно на экран! Можно удалить
    logger->add<MsgBoxLogger>(logTags[3]);  // Что бы не пропустить critical выводим его дополнительно на экран! Можно удалить

    stocks.reset( new Data::Stocks );
    dataBase.reset( new DB::PostgreSql );
    broker.reset( new Broker::TinkoffApi );

    taskManager.reset( new Task::Manager );

}

}
