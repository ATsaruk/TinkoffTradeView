#include <QThread>

#include "getstock.h"
#include "Core/globals.h"
#include "Broker/Tinkoff/tinkoff.h"
#include "DataBase/Query/stocksquery.h"
#include "Tasks/StockTasks/loadstockfromdbfunc.h"
#include "Tasks/StockTasks/loadstockfrombroker.h"

namespace Task {

constexpr long secInTwoWeek = 14 * 24 * 60 * 60;

GetStock::GetStock(const Data::StockKey &stockKey, const size_t minCandleCount)
    : IBaseCommand("GetStock")
{
    key = stockKey;
    this->minCandleCount = minCandleCount;
    extraRangeLoaded = false;   //сбрасываем флаг того, что доп. интервал загружен
}

void GetStock::setData(SharedInterface &inputData)
{
    range = inputData;
}

SharedInterface &GetStock::getResult()
{
    return &stock;
}

/* 1. Проверяет валидность исходных данных,
 * 2. Получает доступные свечи из
 *
 */
void GetStock::exec()
{
    if (!range->isValid()) {
        logCritical << "getStock::exec():;Invalid range!";
        emit finished();
        return;
    }

    //Получаем доступные свечи из Data::Stocks
    subRange = Data::Range(range);  //создаем копию range!

    //Проверяем доступные свечи (Glo.stocks)
    if (isEnoughCandles(false))
        return;

    //Загружаем доступные свечи из БД
    if (loadFromDb())
        return;

    //Загружаем оставшиеся свечи от брокера
    startLoading();
}

/* Функция проверяет достаточно ли свечей в Glo.stocks, условия при которых загрузка считается НЕ завершенной:
 * 1. existedCandles->size() < minCandleCount
 * 2. если existedCandles->getRange().getEnd() == existedRange.getEnd() это означает, что запрашиваемый интервал
 * находится правее существующих данных и загрузки от брокера ещё не было,
 * где existedCandles это свечи в Glo.stocks, дата конца которых <= subRange->getEnd()
 */
bool GetStock::isEnoughCandles(const bool loadFromBrockerComplete)
{
    if (auto [existedRange, existedCount] = Glo.stocks->getRange(key); existedRange.isValid()) {
        auto existedCandles = Glo.stocks->getCandlesForRead(key, QDateTime(), subRange->getEnd(), minCandleCount);
        loadedCount = existedCandles->size();
        if ((existedRange.getEnd() > existedCandles->getRange().getEnd() || loadFromBrockerComplete)
                && loadedCount >= minCandleCount) {
            finishTask();
            return true;
        }
        subRange->remove(existedRange); //продолжаем загрузку без существующего поддиапазона
    }
    return false;
}

bool GetStock::loadFromDb()
{
    auto loadFromDb = execFunc<LoadStockFromDbFunc>(&subRange, key, minCandleCount);
    InterfaceWrapper<Data::Stock> loadedStock = loadFromDb->getResult();
    Glo.stocks->appedStock(loadedStock);

    return isEnoughCandles(false);
}

void GetStock::startLoading()
{
    if (auto [existedRange, existedCount] = Glo.stocks->getRange(key); existedRange.isValid()) {
        //В Glo.stocks есть акция с ключем key, формируем задачи для интервалов, где нет свечей

        bool isLeftLoading = subRange->getBegin() <= key.prevCandleTime(existedRange.getBegin());
        if (isLeftLoading) {
            //в начале есть незагруженный интервал
            InterfaceWrapper<Data::Range> leftRange = Data::Range(subRange->getBegin(), existedRange.getBegin());
            auto *task = createTask<LoadStockFromBroker>(key);
            task->setData( &leftRange );
        }

        bool isRightLoading = subRange->getEnd() >= key.nextCandleTime(existedRange.getEnd());
        if (isRightLoading) {
            //в конеце есть не загруженный интервал
            InterfaceWrapper<Data::Range> rightRange = Data::Range(key.nextCandleTime(existedRange.getEnd()), subRange->getEnd());
            auto *task = createTask<LoadStockFromBroker>(key);
            task->setData( &rightRange );
        }
    } else {
        //В Glo.stocks отсутствует акция с ключем key, загружаем весь интервал
        auto *task = createTask<LoadStockFromBroker>(key);
        task->setData( &subRange );
    }

    //Запускаем первую задачу
    startNextTask();
}

void GetStock::startNextTask()
{
    if (taskList.isEmpty()) {
        //Задачи по загрузке запрошенных интрвалов завершены

        if (isEnoughCandles(true))
            return;

        if (extraRangeLoaded) {
            //Список задач пуст и загрузка доп. 2 недельного интервала завершена, теоретически это невозможно...
            logCritical << "GetStock::startNextTask();extraRangeLoaded but candlesLeft > 0!";
            finishTask();
            return;
        }

        createExtraRangeTasks();
    } else if (extraRangeLoaded) {
        //Происходит загрузка дополнительного 2 недельного интервала
        if (isEnoughCandles(true))
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
    if (auto [existedRange, count] = Glo.stocks->getRange(key); existedRange.isValid())
        endTime = existedRange.getBegin();
    else
        endTime = range->getBegin();

    //Разбвиваем загрузку на поддиапазоны, каждый длительностью с максимальный размер разовой загрузки от брокера
    qint64 maxLoadRange = Broker::TinkoffApi::getMaxLoadInterval(key.interval());
    uint taskCount = ceil(secInTwoWeek / maxLoadRange);
    for (uint i=0; i<taskCount; i++) {
        auto *task = createTask<LoadStockFromBroker>(key);
        subRange->setRange(endTime, -maxLoadRange);
        task->setData(&subRange);

        endTime = endTime.addSecs(-maxLoadRange);
    }

    extraRangeLoaded = true;
}

void GetStock::finishTask()
{
    //Преобразование QSharedPointer<Data::StockViewReference<QReadLocker>> в InterfaceWrapper<Data::StockViewReference<QReadLocker>>
    auto sharedStockVewRef = Glo.stocks->getCandlesForRead(key, range->getBegin(), range->getEnd(), minCandleCount);
    stock = InterfaceWrapper<SharedStockVewRef>(sharedStockVewRef);
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
                .arg(getName(), task->getName()).arg(brokerCandles->getCandles().size());

    //Сохраняем свечи в БД
    DB::StocksQuery::insertCandles(brokerCandles);

    //Сохраняем свечи в Glo.stocks
    Glo.stocks->appedStock(brokerCandles);

    task->deleteLater();

    startNextTask();
}

}
