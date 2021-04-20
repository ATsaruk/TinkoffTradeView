#include "global.h"

#include "Broker/Tinkoff/tinkoff.h"
#include "Logs/filelogger.h"
#include "Logs/msgboxlogger.h"
#include "database.h"

namespace Core {


Global &Global::get()
{
    static Global instance;
    return instance;
}

Global::Global()
{
    logTags << "debug" << "info" << "warning" << "critical";
}

Global::~Global()
{
    Database::disconnect();

    delete taskManager;
    delete broker;
    //delete dataBase;
    delete stocks;
    delete logger;
    delete conf;
}

void Global::init(QObject *parent)
{
    conf = new Config("config.cfg");

    logger = new LoggerList;
    for (const auto &it : logTags)
        logger->add(it);

    taskManager = new Task::Manager(parent->thread());

    stocks = new Data::Stocks;
    //dataBase = new DB::PostgreSql;
    broker = new Broker::TinkoffApi;

    Database::connect();

    // Что бы не пропустить ни одного предупреждения или ошибки выводим их дополнительно на экран! Можно удалить
    logger->add<MsgBoxLogger>(logTags[2]);
    logger->add<MsgBoxLogger>(logTags[3]);
}

}
