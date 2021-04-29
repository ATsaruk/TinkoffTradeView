#include <QThread>

#include "loadstockfromdb.h"

#include "Core/globals.h"
#include "DataBase/Query/stocksquery.h"
#include "Tasks/BrokerTasks/loadstockfrombroker.h"

namespace Task {

constexpr auto SECS_IN_ONE_DAY = 24 * 3600;
constexpr auto SECS_IN_TWO_WEEK = 14 * SECS_IN_ONE_DAY;

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
    //2 недели это новогодние каникулы
    QDateTime twoWeeksAgo = loadRange.getBegin().addSecs(-SECS_IN_TWO_WEEK);

    Candles candles;
    while (true) {
        if (isStopRequested) {
            emit finished();
            return;
        }

        DB::StocksQuery::loadCandles(Glo.dataBase, key, candles, loadRange);
        if (candles.size() >= minCount)
            break;

        //Если после загрузки недостаточно свечей, загружаем дополнительно 1 день, предшествующий loadRange
        loadRange.setRange(loadRange.getBegin().addSecs(-SECS_IN_ONE_DAY), SECS_IN_ONE_DAY);

        if (loadRange.getBegin() < twoWeeksAgo) {
            //Дополнительный 2 недельный интервал загружен и надостаточно свечей,
            //значит их нету в БД или указано заведомо завышенное minCandleCount
            break;
        }
    }

    if (!candles.empty()) {
        Glo.stocks->insertCandles(key, candles);
        logInfo << QString("TaskLoadStockFromDb;exec();loaded;%1;candles;%2;%3")
                   .arg(candles.size()).arg(candles.front().dateTime.toString(), candles.back().dateTime.toString());
    }

    emit finished();
}

}
