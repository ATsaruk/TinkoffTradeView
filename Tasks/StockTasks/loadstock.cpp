#include "loadstock.h"
#include "Core/globals.h"
#include "DataBase/Query/stocksquery.h"
#include "Tasks/StockTasks/loadstockfrombroker.h"

namespace Task {

LoadStock::LoadStock(const StockKey &stockKey, const uint minCandleCount_)
    : CustomCommand()
{
    stock.key = stockKey;
    minCandleCount = minCandleCount_;
    logDebug << "CommandLoadStock;CommandLoadStock();+constructor!";
}

void LoadStock::setData(const Range &range)
{
    loadRange = range;
}

//Возвращает имя задачи
QString LoadStock::getName()
{
    return "CommandLoadStock";
}

Stock &LoadStock::getResult()
{
    return stock;
}

void LoadStock::exec()
{
    DB::StocksQuery::loadCandles(stock, loadRange);
    Glo.stocks->insertCandles(stock);

    /// @todo проверить достаточно ли загружено свечей из БД, если недостаточно, пробуем загружать доп. 2 недели

    Range range = loadRange;
    if (!stock.candles.empty()) {
        if (auto endData = stock.candles.back().dateTime.addSecs(stock.key.intervalToSec()); endData < loadRange.getEnd()) {
            range.setBegin(endData);
        } else if (auto beginData = stock.candles.front().dateTime; beginData > range.getBegin()) {
            range.setEnd(beginData);
        } else
            range = Range();
    }

    /// @todo подрезать время загрузки с учетом ночного интервала

    loadFromBroker(range);
}

void LoadStock::loadFromBroker(const Range &range)
{
    //Закрузка недостающих данных от брокера
    auto task = new LoadStockFromBroker(stock.key);
    task->setData(range);
    registerTask(task);

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

    auto brokerCandles = task->getResult();
    delete task;

    const auto &allCandles = stock.candles;
    std::copy_if(brokerCandles.candles.begin(),
                 brokerCandles.candles.end(),
                 std::back_inserter(stock.candles),
                 [&allCandles] (const auto& candle) { return std::count(allCandles.begin(), allCandles.end(), candle) == 0; } );


    /// @todo проверить достаточно ли загружено свечей от брокера, если недостаточно, загружаем доп. 2 недели

    newCandles = Glo.stocks->insertCandles(stock);
    if (!newCandles.candles.empty())
        DB::StocksQuery::insertCandles(newCandles);

    //if (candles.size() >= minCandleCount) {
        emit finished();
        //return;
    //}

    //Сдвигаем интервал загрузки

    //Запускаем следующую или завершаем выполнение
}

}
