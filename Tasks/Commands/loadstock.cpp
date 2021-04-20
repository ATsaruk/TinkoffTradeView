#include "loadstock.h"

#include "Core/globals.h"
#include "Tasks/DataBaseTasks/loadstockfromdb.h"
#include "Tasks/BrokerTasks/loadstockfrombroker.h"

namespace Task {

LoadStock::LoadStock(QThread *parent)
    : CustomCommand(parent)
{
    logDebug << "CommandLoadStock;CommandLoadStock();+constructor!";
}

void LoadStock::setData(const StockKey &stockKey, const Range &range, const uint minCandleCount)
{
    //Задача на загрузку данных из БД
    addTask <LoadStockFromDb> (stockKey, range, minCandleCount);

    //Закрузка недостающих данных от брокера
    addTask <LoadStockFromBroker> (stockKey, range, minCandleCount);
}

//Возвращает имя задачи
QString LoadStock::getName()
{
    return "CommandLoadStock";
}

}
