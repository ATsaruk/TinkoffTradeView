#include <QThread>

#include "getstock.h"
#include "Core/globals.h"
#include "Broker/Tinkoff/tinkoff.h"
#include "DataBase/Query/stocksquery.h"
#include "Tasks/StockTasks/loadstockfromdbfunc.h"
#include "Tasks/StockTasks/loadstockfrombroker.h"

namespace Task {

GetStock::GetStock(const Data::StockKey &stockKey, const size_t minCandleCount)
    : IBaseCommand("GetStock"),
      _minCandlesCount(minCandleCount),
      loadedCandlesCount(0),
      _key(stockKey)
{
}

void GetStock::setData(SharedInterface &inputData)
{
    _loadRange = inputData;
}

SharedInterface &GetStock::getResult()
{
    return &_stock;
}

/* 1. Проверяет валидность исходных данных,
 * 2. Получает доступные свечи из
 *
 */
void GetStock::exec()
{
    //Проверяем корректность исходных данных
    if ( (_loadRange->isValid()) != (_minCandlesCount == 0) )
        throw std::logic_error("getStock::exec():;Invalid range!");

    if ( isEnoughCandles()         //Проверяем доступные свечи в Glo.stocks
         || loadFromDb()           //Загружаем доступные свечи из БД
         || !startLoading() ) {     //Загружаем оставшиеся свечи от брокера
        finishTask();
    }
}

bool GetStock::isEnoughCandles()
{
    if (auto stock = Glo.stocks->getCandlesForRead(_key, _loadRange, _loadRange->isValid() ? 0 : _minCandlesCount); stock) {
        loadedCandlesCount = stock->size();
        return stock->isEnoughCandles(_loadRange, _loadRange->isValid() ? 0 : _minCandlesCount);
    }

    return false;
}

bool GetStock::loadFromDb()
{
    //убираем загруженный интервал!
    if (auto [existedRange, existedCount] = Glo.stocks->stockInfo(_key); existedRange.isValid()) {
        //В Glo.stocks есть акция с ключем key, формируем задачи для подинтервалов, где нет свечей

        size_t remainedCandlesCount = _minCandlesCount - loadedCandlesCount;

        bool isCandlesOnTheLeft = _loadRange->isBeginNull() || _loadRange->begin() <= _key.prevCandleTime(existedRange.begin());
        if ( isCandlesOnTheLeft ) {
            //в начале есть незагруженный интервал
            InterfaceWrapper<Data::Range> leftRange = Data::Range(_loadRange->begin(), existedRange.begin());
            auto loadFromDb = execFunc<LoadStockFromDbFunc>(&leftRange, _key, leftRange->isValid() ? 0 : remainedCandlesCount);
            InterfaceWrapper<Data::Stock> loadedStock = loadFromDb->getResult();
            Glo.stocks->appedStock(loadedStock);
        }

        bool isCandlesOnTheRight = _loadRange->isEndNull() || _loadRange->end() >= _key.nextCandleTime(existedRange.end());
        if ( isCandlesOnTheRight ) {
            //в конеце есть не загруженный интервал
            InterfaceWrapper<Data::Range> rightRange = Data::Range(_key.nextCandleTime(existedRange.end()), _loadRange->end());
            auto loadFromDb = execFunc<LoadStockFromDbFunc>(&rightRange, _key, rightRange->isValid() ? 0 : remainedCandlesCount);
            InterfaceWrapper<Data::Stock> loadedStock = loadFromDb->getResult();
            Glo.stocks->appedStock(loadedStock);
        }
    } else {
        auto loadFromDb = execFunc<LoadStockFromDbFunc>(&_loadRange, _key, _loadRange->isValid() ? 0 : _minCandlesCount);
        InterfaceWrapper<Data::Stock> loadedStock = loadFromDb->getResult();
        Glo.stocks->appedStock(loadedStock);
    }

    return isEnoughCandles();
}

bool GetStock::startLoading()
{
    if (auto [existedRange, existedCount] = Glo.stocks->stockInfo(_key); existedRange.isValid()) {
        //В Glo.stocks есть акция с ключем key, формируем задачи для подинтервалов, где нет свечей

        bool isCandlesOnTheLeft = _loadRange->isBeginNull() || _loadRange->begin() <= _key.prevCandleTime(existedRange.begin());
        bool isCandlesOnTheRight = _loadRange->isEndNull() || _loadRange->end() >= _key.nextCandleTime(existedRange.end());

        if ( isCandlesOnTheLeft ) {
            //в начале есть незагруженный интервал
            InterfaceWrapper<Data::Range> leftRange = Data::Range(_loadRange->begin(), existedRange.begin());
            auto *task = createTask<LoadStockFromBroker>(_key, leftRange->isValid() ? 0 : _minCandlesCount - loadedCandlesCount);
            task->setData( &leftRange );
        } else if ( isCandlesOnTheRight ) {
            static std::unordered_map<Data::StockKey, QDateTime> lastEndLoadingList;    ///@todo !!!!пропускать уже загруженные интервалы
            //в конеце есть не загруженный интервал
            InterfaceWrapper<Data::Range> rightRange = Data::Range(_key.nextCandleTime(existedRange.end()), _loadRange->end());
            auto *task = createTask<LoadStockFromBroker>(_key, rightRange->isValid() ? 0 : _minCandlesCount - loadedCandlesCount);
            task->setData( &rightRange );
        }
    } else {
        //В Glo.stocks отсутствует акция с ключем key, загружаем весь интервал
        auto *task = createTask<LoadStockFromBroker>(_key, _loadRange->isValid() ? 0 : _minCandlesCount - loadedCandlesCount);
        task->setData(&_loadRange);
    }

    if (!taskList.isEmpty()) {
        IBaseCommand::runNextTask();    //Запускаем сформированную выше задачу на загрузку от брокера
        return true;
    }

    return false;
}

void GetStock::finishTask()
{
    //Преобразование QSharedPointer<Data::StockViewReference<QReadLocker>> в InterfaceWrapper<Data::StockViewReference<QReadLocker>>
    auto sharedStockVew = Glo.stocks->getCandlesForRead(_key, _loadRange, _minCandlesCount);
    _stock = InterfaceWrapper<Data::StockView>(sharedStockVew);
    emit finished();

    auto range = _stock->range();
    auto size = _stock->size();
    Q_UNUSED(range) Q_UNUSED(size)
}

//Завершение очередной задачи, сохраняем полученные свечи в БД и в Glo.Stocks и запускаем следующую
void GetStock::taskFinished()
{
    auto *task = dynamic_cast<LoadStockFromBroker*>(sender());

    assert(task != nullptr && QString("%1;taskFinished();can't get task!;tasksLeft: %2")
            .arg(getName()).arg(taskList.size()).toStdString().data());

    InterfaceWrapper<Data::Stock> brokerCandles = task->getResult();

    logDebug << QString("%1;taskFinished();finished: %2;loaded;%3;candles")
                .arg(getName(), task->getName()).arg(brokerCandles->size());

    //Сохраняем свечи в БД
    DB::StocksQuery::insertCandles(brokerCandles);

    //Сохраняем свечи в Glo.stocks
    Glo.stocks->appedStock(brokerCandles);

    task->deleteLater();

    finishTask();
}

}
