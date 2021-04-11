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
{
    logTags << "debug" << "info" << "warning" << "critical";
}

Global::~Global()
{

}

void Global::init(QObject *parent)
{
    conf.init("config.cfg");

    logger.init();
    for (const auto &it : logTags)
        logger->add(it);

    taskManager.init(parent->thread());

    dataBase.init<DB::PostgreSql>();
    broker.init<Broker::TinkoffApi>();

    // Что бы не пропустить ни одного предупреждения или ошибки выводим их дополнительно на экран! Можно удалить
    logger->add<MsgBoxLogger>(logTags[2]);
    logger->add<MsgBoxLogger>(logTags[3]);
}

}
