#include <QThread>

#include "loadstock.h"
#include "Core/globals.h"
#include "Broker/Tinkoff/tinkoff.h"
#include "DataBase/Query/stocksquery.h"
#include "Tasks/StockTasks/loadstockfromdbfunc.h"
#include "Tasks/StockTasks/loadstockfrombroker.h"

namespace Task {

constexpr long secInTwoWeek = 14 * 24 * 60 * 60;

LoadStock::LoadStock(const Data::StockKey &stockKey, const uint minCandleCount_)
    : IBaseCommand("LoadStock")
{
    stock->setStockKey(stockKey);
    minCandleCount = minCandleCount_;
}

void LoadStock::setData(SharedInterface &inputData)
{
    range = inputData;

    if (!range->isValid())
        logCritical << "LoadStock::setData():;Invalid range!";
}

SharedInterface &LoadStock::getResult()
{
    return &stock;
}

void LoadStock::exec()
{
    if (!range->isValid()) {
        logCritical << "LoadStock::exec():;Invalid range!";
        emit finished();
        return;
    }

    extraRangeLoaded = false;   //сбрасываем флаг того, что доп. интервал загружен

    //Загружаем доступные свечи из БД
    auto *loadFromDb = execFunc<LoadStockFromDbFunc>(&range, stock->key(), minCandleCount);
    stock = loadFromDb->getResult();

    createLoadingTasks();   //создаем задачи для загрузки оставшихся свечей от брокера

    startNextTask();
}

void LoadStock::createLoadingTasks()
{
    if (stock->getCandles().empty()) {
        existedRange = range;
        //Свечей в БД нет, загружаем весь интервал
        auto *task = createTask<LoadStockFromBroker>(stock->key());
        task->setData(&range);
    } else {
        //В БД есть загруженные вечи. Создаем задачи на загрузку из интервалов, где свечи не загружены.
        existedRange = stock->range();
        Glo.stocks->insert(stock);

        uint candleInterval = stock->key().intervalToSec();
        bool isLeftLoading = range->getBegin().addSecs(candleInterval) <= existedRange.getBegin();  //в начале есть незагруженный интервал
        if (isLeftLoading) {
            auto *task = createTask<LoadStockFromBroker>(stock->key());
            InterfaceWrapper<Data::Range> subRange = Data::Range(range->getBegin(), existedRange.getBegin());
            task->setData( &subRange );
        }

        bool isRightLoading = range->getEnd() >= existedRange.getEnd().addSecs(candleInterval);    //в конеце есть не загруженный интервал
        if (isRightLoading) {
            auto *task = createTask<LoadStockFromBroker>(stock->key());
            InterfaceWrapper<Data::Range> subRange = Data::Range(existedRange.getEnd().addSecs(stock->key().intervalToSec()), range->getEnd());
            task->setData( &subRange );
        }
    }
}

void LoadStock::startNextTask()
{
    if (taskList.isEmpty()) {
        //Задачи по загрузке запрошенных интрвалов завершены
        auto &candles = stock->getCandles();
        if (candles.size() >= minCandleCount || extraRangeLoaded) {
            //Загружено достаточно свечей
            std::sort(candles.begin(), candles.end());    //сортируем
            auto newRange = Glo.stocks->insert(stock);
            DB::StocksQuery::insertCandles(loadedCandles);              //Сохраняем загруженные свечи в БД
            emit finished();
            return;
        }

        //Подготавливам дополнительный 2 недельный интервал для загрузки
        uint missingCandlesCount = minCandleCount - candles.size();
        auto *task = createTask<LoadStockFromBroker>(stock->key(), missingCandlesCount);
        InterfaceWrapper<Data::Range> subRange;
        subRange->setRange(std::min(range->getBegin(), existedRange.getBegin()), -secInTwoWeek);
        task->setData( &subRange );

        extraRangeLoaded = true;
    }

    runNextTask();
}

void LoadStock::taskFinished()
{
    auto *task = dynamic_cast<LoadStockFromBroker*>(sender());

    assert(task != nullptr && QString("%1;taskFinished();can't get task!;tasksLeft: %2")
            .arg(getName()).arg(taskList.size()).toStdString().data());

    InterfaceWrapper<Data::Stock> brokerCandles = task->getResult();

    logDebug << QString("%1;taskFinished();finished: %2;loaded;%3;candles")
                .arg(getName(), task->getName()).arg(brokerCandles->getCandles().size());

    stock->append(brokerCandles);

    task->deleteLater();

    startNextTask();
}

}
