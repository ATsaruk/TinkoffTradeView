/** @defgroup Core Cистемные инструменты
  * @brief Модуль системных инструментов
  * @author Царюк А.В.
  * @date Апрель 2021 года */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "config.h"
#include "loggerlist.h"
#include "Broker/api.h"
#include "DataBase/idatabase.h"
#include "Data/Stock/stocks.h"
#include "Tasks/manager.h"

namespace Core {


///@ingroup Core @{

///Быстрый доступ к классу
#define Glo Core::Globals::get()

///Быстрый доступ к Debug логу
#define logDebug    *Core::Globals::get().logger->get("debug")
///Быстрый доступ к Info логу
#define logInfo     *Core::Globals::get().logger->get("info")
///Быстрый доступ к Warning логу
#define logWarning  *Core::Globals::get().logger->get("warning")
///Быстрый доступ к Critical логу
#define logCritical *Core::Globals::get().logger->get("critical")

///Основная функция для запуска задач
#define NEW_TASK Core::Globals::get().taskManager->addTask
///@}

/** @ingroup Core
  * @brief Класс singleton, содержит объекты для общего доступа */
class Globals
{
public:
    //Основные инструменты
    Config *conf;
    LoggerList *logger;
    Task::Manager *taskManager;

    //Вспомогательные
    Data::Stocks *stocks;
    DB::IDataBase *dataBase;
    Broker::Api *broker;

    //Singleton
    static Globals &get();
    ~Globals();

    void init(QObject *parent);

protected:
    Globals();

private:
    QStringList logTags;
};

}

#endif // GLOBALS_H
