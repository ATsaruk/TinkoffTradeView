#include <QThread>

#include "loadstockfromdb.h"

#include "Core/globals.h"
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

void LoadStockFromDb::setData(const StockKey &stockKey, const Range &range, const uint minCandleCount)
{
    if (!range.isValid())
        throw std::invalid_argument("LoadStockFromDb::setData: invalid loadRange!");

    key = stockKey;
    loadRange = range;
    minCount = minCandleCount;
}

QString LoadStockFromDb::getName()
{
    return "TaskLoadStockFromDb";
}

void LoadStockFromDb::exec()
{
    Candles candles = DB::StocksQuery::loadCandles(Glo.dataBase, key, loadRange);

    if (!candles.empty()) {
        Glo.stocks->insertCandles(key, candles);
        logInfo << QString("TaskLoadStockFromDb;exec();loaded;%1;candles;%2;%3")
                   .arg(candles.size()).arg(candles.front().dateTime.toString(), candles.back().dateTime.toString());
    }

    emit finished();
}

}
