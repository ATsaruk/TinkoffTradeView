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

Globals::~Globals()
{
    delete taskManager;
    delete broker;
    delete dataBase;
    delete stocks;
    delete logger;
    delete conf;
}

void Globals::init(QObject *parent)
{
    conf = new Config("config.cfg");

    logger = new LoggerList;
    for (const auto &it : logTags)
        logger->add(it);

    taskManager = new Task::Manager(parent->thread());

    stocks = new Data::Stocks;
    dataBase = new DB::PostgreSql;
    broker = new Broker::TinkoffApi;

    // Что бы не пропустить ни одного предупреждения или ошибки выводим их дополнительно на экран! Можно удалить
    logger->add<MsgBoxLogger>(logTags[2]);
    logger->add<MsgBoxLogger>(logTags[3]);
}

}
