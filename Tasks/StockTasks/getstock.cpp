#include <QThread>

#include "getstock.h"
#include "Core/globals.h"
#include "Broker/Tinkoff/tinkoff.h"
#include "DataBase/Query/stocksquery.h"
#include "Tasks/StockTasks/loadstockfromdbfunc.h"
#include "Tasks/StockTasks/loadstockfrombroker.h"

///@fixme вынести загрузку доп. 2 недельного интервала в LoadStockFromBroker

namespace Task {

constexpr long secInTwoWeek = 14 * 24 * 60 * 60;

GetStock::GetStock(const Data::StockKey &stockKey, const size_t minCandleCount)
    : IBaseCommand("GetStock"),
      _minCandlesCount(minCandleCount),
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
    //
    if ( (_loadRange->isValid()) != (_minCandlesCount > 0) ) {
        logCritical << "getStock::exec():;Invalid range!";
        emit finished();
        return;
    }

    //Получаем доступные свечи из Data::Stocks
    _subRange = Data::Range(_loadRange);  //создаем копию range!

    //Проверяем доступные свечи (Glo.stocks)
    if (isEnoughCandles())
        return;

    //Загружаем доступные свечи из БД
    if (loadFromDb())
        return;

    //Загружаем оставшиеся свечи от брокера
    startLoading();
}

bool GetStock::isEnoughCandles()
{
    return Glo.stocks->getCandlesForRead(_key)->isEnoughCandles(_loadRange, _minCandlesCount);
}


bool GetStock::loadFromDb()
{
    auto loadFromDb = execFunc<LoadStockFromDbFunc>(&_subRange, _key, _minCandlesCount);
    InterfaceWrapper<Data::Stock> loadedStock = loadFromDb->getResult();
    Glo.stocks->appedStock(loadedStock);

    return isEnoughCandles();
}

void GetStock::startLoading()
{
    if (auto [existedRange, existedCount] = Glo.stocks->getRange(_key); existedRange.isValid()) {
        //В Glo.stocks есть акция с ключем key, формируем задачи для интервалов, где нет свечей

        bool isLeftLoading = _subRange->begin() <= _key.prevCandleTime(existedRange.begin());
        if (isLeftLoading) {
            //в начале есть незагруженный интервал
            InterfaceWrapper<Data::Range> leftRange = Data::Range(_subRange->begin(), existedRange.begin());
            auto *task = createTask<LoadStockFromBroker>(_key);
            task->setData( &leftRange );
        }

        bool isRightLoading = _subRange->end() >= _key.nextCandleTime(existedRange.end());
        if (isRightLoading) {
            static std::unordered_map<Data::StockKey, QDateTime> lastEndLoadingList;    ///@todo !!!!пропускать уже загруженные интервалы
            //в конеце есть не загруженный интервал
            InterfaceWrapper<Data::Range> rightRange = Data::Range(_key.nextCandleTime(existedRange.end()), _subRange->end());
            auto *task = createTask<LoadStockFromBroker>(_key);
            task->setData( &rightRange );
        }
    } else {
        //В Glo.stocks отсутствует акция с ключем key, загружаем весь интервал
        auto *task = createTask<LoadStockFromBroker>(_key);
        task->setData( &_subRange );
    }

    //Запускаем первую задачу
    startNextTask();
}

void GetStock::startNextTask()
{
    if (taskList.isEmpty()) {
        //Задачи по загрузке запрошенных интрвалов завершены

        if (isEnoughCandles(/*true*/))
            return;

        if (_extraRangeLoaded) {
            //Список задач пуст и загрузка доп. 2 недельного интервала завершена, теоретически это невозможно...
            logCritical << "GetStock::startNextTask();extraRangeLoaded but candlesLeft > 0!";
            finishTask();
            return;
        }

        createExtraRangeTasks();
    } else if (_extraRangeLoaded) {
        //Происходит загрузка дополнительного 2 недельного интервала
        if (isEnoughCandles(/*true*/))
            return;
    }

    IBaseCommand::runNextTask();
}

/* В запрашиваемом диапазоне оказалось недостаточно свечей, создаем задачи для загрузки дополнительного 2 недельного
 * интервала. 2 недели, т.к. новогодние каникулы могу длится до 2 недель и в этот промежуток времени биржа не работает*/
void GetStock::createExtraRangeTasks()
{
    //Подготавливам дополнительный 2 недельный интервал для загрузки
    QDateTime endTime;
    if (auto [existedRange, count] = Glo.stocks->getRange(_key); existedRange.isValid())
        endTime = existedRange.begin();
    else
        endTime = _loadRange->begin();

    //Разбвиваем загрузку на поддиапазоны, каждый длительностью с максимальный размер разовой загрузки от брокера
    qint64 maxLoadRange = Broker::TinkoffApi::getMaxLoadInterval(_key.interval());
    uint taskCount = ceil(secInTwoWeek / maxLoadRange);
    for (uint i=0; i<taskCount; i++) {
        auto *task = createTask<LoadStockFromBroker>(_key);
        _subRange->setRange(endTime, -maxLoadRange);
        task->setData(&_subRange);

        endTime = endTime.addSecs(-maxLoadRange);
    }

    _extraRangeLoaded = true;
}

void GetStock::finishTask()
{
    //Преобразование QSharedPointer<Data::StockViewReference<QReadLocker>> в InterfaceWrapper<Data::StockViewReference<QReadLocker>>
    auto sharedStockVew = Glo.stocks->getCandlesForRead(_key, _loadRange, _minCandlesCount);
    _stock = InterfaceWrapper<Data::StockView>(sharedStockVew);
    emit finished();
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

    startNextTask();
}

}
