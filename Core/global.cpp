#include "global.h"

#include "Broker/Tinkoff/tinkoff.h"
#include "DataBase/postgresql.h"
#include "Logs/filelogger.h"
#include "Logs/msgboxlogger.h"

namespace Core {

Global &Global::get()
{
    static Global instance;
    return instance;
}

Global::Global()
    : conf ("config.cfg")
{
    logTags << "debug" << "info" << "warning" << "critical";
}

Global::~Global()
{
    broker->deleteLater();
    taskManager->deleteLater();
    delete dataBase;

    delete stocks;
}

void Global::init(QObject *parent)
{
    for (const auto &it : logTags)
        logger->add(it);

    stocks = new Data::Stocks;

    dataBase = new DB::PostgreSql;
    taskManager = new Task::Manager(parent->thread());
    broker = new Broker::TinkoffApi;

    // Что бы не пропустить ни одного предупреждения или ошибки выводим их дополнительно на экран! Можно удалить
    logger->add<MsgBoxLogger>(logTags[2]);
    logger->add<MsgBoxLogger>(logTags[3]);
}

}
