#include <QThread>

#include "loadstockfromdb.h"

#include "Core/global.h"
#include "DataBase/Query/stocksquery.h"
#include "Tasks/BrokerTasks/loadstockfrombroker.h"

namespace Task {

LoadStockFromDb::LoadStockFromDb(QThread *parent)
    : IBaseTask(parent)
{
    logDebug << "loadStockFromDbTask;loadStockFromDbTask();+constructor!";
}

LoadStockFromDb::~LoadStockFromDb()
{
    logDebug << "loadStockFromDbTask;~loadStockFromDbTask();-destructor!";
}

void LoadStockFromDb::setData(const StockKey &stockKey, const DateRange &range_, const uint minCandleCount)
{
    key = stockKey;
    range = range_;
    minCount = minCandleCount;

    qint64 minLoadInterval = (minCount + 1) * key.intervalToSec();
    if (range.toSec() < minLoadInterval)
        range.setRange(range.getEnd(), -minLoadInterval);
}

QString LoadStockFromDb::getName()
{
    return "TaskLoadStockFromDb";
}

void LoadStockFromDb::exec()
{
    //Получаем доступ к свечной информации
    Candles candles;

    //2 недели это новогодние каникулы
    QDateTime twoWeeksAgo = range.getBegin().addSecs(-14 * 86400); //86400 - число секунд в сутках
    while (candles.size() < minCount) {
        //Загрузка свечных данных из БД
        DB::StocksQuery::retrieveCandles(*Glo.dataBase, key, candles, range);

        long interval = LoadStockFromBroker::getMaxLoadInterval(key.interval());
        range.displace(-interval, -interval);
        if (range.getBegin() < twoWeeksAgo)
            break;

        //Если пришел запрос на остановку задачи
        if (isStopRequested) {
            emit finished();
            return;
        }
    }

    if (!candles.empty())
        logInfo << QString("TaskLoadStockFromDb;exec();loaded;%1;candles;%2;%3")
                   .arg(candles.size()).arg(candles.front().dateTime.toString(), candles.back().dateTime.toString());

    //Добавляем данные в общий список акций
    Glo.stocks.insertCandles(key, candles);

    //Отпавляем сигнал о завершении задачи
    emit finished();
}

}
