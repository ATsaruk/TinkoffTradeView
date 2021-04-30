#include "loadstock.h"
#include "Core/globals.h"
#include "DataBase/Query/stocksquery.h"
#include "Tasks/StockTasks/loadstockfrombroker.h"

namespace Task {

LoadStock::LoadStock(QThread *parent)
    : CustomCommand(parent)
{
    logDebug << "CommandLoadStock;CommandLoadStock();+constructor!";
}

void LoadStock::setData(const StockKey &stockKey, const Range &range)
{
    key = stockKey;
    loadRange = range;
}

void LoadStock::setMinCandleCount(const uint minCandleCount_)
{
    minCandleCount = minCandleCount_;
}

//Возвращает имя задачи
QString LoadStock::getName()
{
    return "CommandLoadStock";
}

Candles &LoadStock::getCandles()
{
    return candles;
}

void LoadStock::exec()
{
    candles = DB::StocksQuery::loadCandles(Glo.dataBase, key, loadRange);

    Range range = loadRange;
    if (!candles.empty()) {
        if (auto endData = candles.back().dateTime.addSecs(key.intervalToSec()); endData < loadRange.getEnd()) {
            range.setBegin(endData);
        } else if (auto beginData = candles.front().dateTime; beginData > range.getBegin()) {
            range.setEnd(beginData);
        } else
            range = Range();
    }

    loadFromBroker(range);
}

void LoadStock::loadFromBroker(const Range &range)
{
    //Закрузка недостающих данных от брокера
    addTask <LoadStockFromBroker> (key, range);

    runNextTask();
}

void LoadStock::taskFinished()
{
    //Удаляем завершившуюся задачу
    auto task = dynamic_cast<LoadStockFromBroker*>(sender());
    if ( task == nullptr ) {
        throw std::logic_error(QString("%1;taskFinished();can't get task!;tasksLeft = %2")
                               .arg(getName()).arg(taskList.size()).toStdString());
    } else {
        logDebug << QString("%1;taskFinished();finished: %2;tasksLeft = %3")
                    .arg(getName(), task->getName()).arg(taskList.size());
    }

    auto brokerCandles = task->getCandles();
    delete task;

    const auto &allCandles = candles;
    std::copy_if(brokerCandles.begin(),
                 brokerCandles.end(),
                 std::back_inserter(candles),
                 [&allCandles] (const auto& candle) { return std::count(allCandles.begin(), allCandles.end(), candle) == 0; } );

    //if (candles.size() >= minCandleCount) {
        emit finished();
        //return;
    //}

    //Сдвигаем интервал загрузки

    //Запускаем следующую или завершаем выполнение
}

}
