/** @defgroup Core Cистемные инструменты
  * @brief Модуль системных инструментов
  * @author Царюк А.В.
  * @date Апрель 2021 года */

#ifndef GLOBAL_H
#define GLOBAL_H

#include "config.h"
#include "loggerlist.h"
#include "Broker/api.h"
#include "Data/Stock/stocks.h"
//#include "DataBase/idatabase.h"
#include "Tasks/manager.h"

namespace Core {


///@ingroup Core @{

///Быстрый доступ к классу
#define Glo Core::Global::get()

///Быстрый доступ к Debug логу
#define logDebug    *Core::Global::get().logger->get("debug")
///Быстрый доступ к Info логу
#define logInfo     *Core::Global::get().logger->get("info")
///Быстрый доступ к Warning логу
#define logWarning  *Core::Global::get().logger->get("warning")
///Быстрый доступ к Critical логу
#define logCritical *Core::Global::get().logger->get("critical")

///Основная функция для запуска задач
#define NEW_TASK Core::Global::get().taskManager->addTask
///@}

/** @ingroup Core
  * @brief Класс singleton, содержит объекты для общего доступа */
class Global
{
public:
    //Основные инструменты
    Config *conf;
    LoggerList *logger;
    Task::Manager *taskManager;

    //Вспомогательные
    Data::Stocks *stocks;
    Broker::Api *broker;
    //DB::IDataBase *dataBase;

    //Singleton
    static Global &get();
    ~Global();

    void init(QObject *parent);

protected:
    Global();

private:
    QStringList logTags;
};

}

#endif // GLOBAL_H
