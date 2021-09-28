#include <QThread>

#include "getstock.h"
#include "Core/globals.h"
#include "Broker/Tinkoff/tinkoff.h"
#include "DataBase/Query/stocksquery.h"
#include "Tasks/StockTasks/loadstockfromdbfunc.h"
#include "Tasks/StockTasks/loadstockfrombroker.h"

namespace Task {

constexpr long secInTwoWeek = 14 * 24 * 60 * 60;

GetStock::GetStock(const Data::StockKey &stockKey, const size_t minCandleCount_)
    : IBaseCommand("GetStock")
{
    key = stockKey;
    candlesLeft = minCandleCount_;
    extraRangeLoaded = false;   //сбрасываем флаг того, что доп. интервал загружен
}

void GetStock::setData(SharedInterface &inputData)
{
    range = inputData;

    if (!range->isValid())
        logCritical << "getStock::setData():;Invalid range!";
}

SharedInterface &GetStock::getResult()
{
    return &stock;
}

void GetStock::exec()
{
    if (!range->isValid()) {
        logCritical << "getStock::exec():;Invalid range!";
        emit finished();
        return;
    }

    ///@todo проверить / отладить
    //Получаем доступные свечи из Data::Stocks
    subRange = Data::Range(range);

    if (auto [existedRange, existedCount] = Glo.stocks->getRange(key); existedRange.isValid())
        if ( removeRange(existedRange, existedCount) )
            return;

    //Загружаем доступные свечи из БД
    if (auto [existedRange, count] = loadFromDb(); existedRange.isValid())
        if (removeRange(existedRange, count))
            return;

    //Сохранить полученные свечи!
    createLoadingTasks();   //создаем задачи для загрузки оставшихся свечей от брокера

    startNextTask();
}

bool GetStock::removeRange(const Data::Range &existed, const size_t count)
{
    if (existed.contains(subRange) && candlesLeft <= count) {
        finishTask();
        return false;
    }
    subRange->remove(existed);    ///@< проверить какая функция remove вызовется??? нужна Data::Range::remove(Range)
    candlesLeft -= count;
    return true;
}

std::pair<Data::Range, size_t> GetStock::loadFromDb()
{
    auto loadFromDb = execFunc<LoadStockFromDbFunc>(&subRange, key, candlesLeft);
    InterfaceWrapper<Data::Stock> loadedStock = loadFromDb->getResult();

    Data::StockViewReference<QReadLocker> view(loadedStock);
    auto loadedRange = view.getRange();
    auto loadedCount = std::size(view);

    Glo.stocks->appedStock(loadedStock);

    return std::make_pair(loadedRange, loadedCount);
}

void GetStock::createLoadingTasks()
{
    if (auto [existedRange, existedCount] = Glo.stocks->getRange(key); existedRange.isValid()) {
        bool isLeftLoading = subRange->getBegin() <= key.prevCandleTime(existedRange.getBegin());  //в начале есть незагруженный интервал
        if (isLeftLoading) {
            InterfaceWrapper<Data::Range> leftRange = Data::Range(subRange->getBegin(), existedRange.getBegin());
            auto *task = createTask<LoadStockFromBroker>(key);
            task->setData( &leftRange );
        }

        bool isRightLoading = subRange->getEnd() >= key.nextCandleTime(existedRange.getEnd());    //в конеце есть не загруженный интервал
        if (isRightLoading) {
            InterfaceWrapper<Data::Range> rightRange = Data::Range(key.nextCandleTime(existedRange.getEnd()), subRange->getEnd());
            auto *task = createTask<LoadStockFromBroker>(key);
            task->setData( &rightRange );
        }
    } else {
        //В Glo.stocks отсутствует акция с ключем key, загружаем весь интервал
        auto *task = createTask<LoadStockFromBroker>(key);
        task->setData( &subRange );
    }
}

void GetStock::startNextTask()
{
    if (taskList.isEmpty()) {
        //Задачи по загрузке запрошенных интрвалов завершены

        if (candlesLeft == 0) {
            //Загружено достаточно свечей, завершаем задачу
            finishTask();
            return;
        }

        if (extraRangeLoaded) {
            //Чисто теоретически такого быть не может...
            logCritical << "GetStock::startNextTask();extraRangeLoaded but candlesLeft > 0!";
            finishTask();
            return;
        }

        createExtraRangeTasks();
    } else if (extraRangeLoaded) {
        //Происходит загрузка дополнительного 2 недельного интервала
        if (candlesLeft == 0) {
            //И загружено достаточно свечей, завершаем задачу
            finishTask();
            return;
        }
    }

    runNextTask();
}

void GetStock::createExtraRangeTasks()
{
    extraRangeLoaded = true;

    //Подготавливам дополнительный 2 недельный интервал для загрузки
    QDateTime endTime;
    if (auto [existedRange, count] = Glo.stocks->getRange(key); existedRange.isValid())
        endTime = existedRange.getBegin();
    else
        endTime = range->getBegin();

    //Разбвиваем загрузку на поддиапазоны, каждый длительностью с максимальный размер разовой загрузки от брокера
    qint64 maxLoadRange = Broker::TinkoffApi::getMaxLoadInterval(key.interval());
    uint taskCount = ceil(secInTwoWeek / maxLoadRange);
    for (uint i=0; i<taskCount; i++) {
        auto *task = createTask<LoadStockFromBroker>(key, candlesLeft);
        subRange->setRange(endTime, -maxLoadRange);
        task->setData(&subRange);

        endTime = endTime.addSecs(-maxLoadRange);
    }
}

void GetStock::finishTask()
{
    stock = Glo.stocks->getCandlesForRead(key, range->getBegin(), range->getEnd());
    emit finished();
}

void GetStock::taskFinished()
{
    auto *task = dynamic_cast<LoadStockFromBroker*>(sender());

    assert(task != nullptr && QString("%1;taskFinished();can't get task!;tasksLeft: %2")
            .arg(getName()).arg(taskList.size()).toStdString().data());

    InterfaceWrapper<Data::Stock> brokerCandles = task->getResult();

    logDebug << QString("%1;taskFinished();finished: %2;loaded;%3;candles")
                .arg(getName(), task->getName()).arg(brokerCandles->getCandles().size());

    //Сохраняем свечи в БД
    DB::StocksQuery::insertCandles(brokerCandles);

    Data::StockViewReference<QReadLocker> view(brokerCandles);
    if (auto loadedCount = std::size(view); candlesLeft > loadedCount)
        candlesLeft -= loadedCount;
    else
        candlesLeft = 0;

    //Сохраняем свечи в Glo.stocks
    Glo.stocks->appedStock(brokerCandles);

    task->deleteLater();

    startNextTask();
}

}
