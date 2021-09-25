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
#include "Data/Stock/stocklist.h"
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
#define TaskManager Core::Globals::get().taskManager
///@}

/** @ingroup Core
  * @brief Класс singleton, содержит объекты для общего доступа */
class Globals
{
public:
    Globals(Globals &&) = delete;
    Globals(const Globals &) = delete;
    Globals& operator = (Globals &&) = delete;
    Globals& operator = (const Globals &) = delete;

    //Основные инструменты
    QScopedPointer<Config> conf;
    QScopedPointer<LoggerList> logger;
    QScopedPointer<Task::Manager> taskManager;

    //Вспомогательные
    QScopedPointer<Data::IStocks> stocks;
    QScopedPointer<DB::IDataBase> dataBase;
    QScopedPointer<Broker::Api> broker;

    //Singleton
    static Globals &get();

    void init();

protected:
    Globals();

private:
    QStringList logTags;
};

}

#endif // GLOBALS_H
