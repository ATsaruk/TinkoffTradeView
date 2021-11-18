#include <QSqlQuery>
#include <QSqlError>

#include "stocksquery.h"
#include "Core/globals.h"

namespace DB {

StocksQuery::StocksQuery()
{

}

void StocksQuery::insertCandles(const Stock &stock)
{
    auto *db = Glo.dataBase.data();
    QMutexLocker locker(&db->mutex);

    if (!db->isOpen())
        return;

    const auto &figi = stock.key().figi();
    QString interval = stock.key().intervalToString();

    auto insertCandle = [&figi, &interval, &db] (const auto &it) {
        QString insert = QString("INSERT INTO stocks (figi, interval, time, open, close, high, low, volume) VALUES ('%1', '%2', '%3', %4, %5, %6, %7, %8)")
                .arg(figi,
                     interval,
                     it.dateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(it.open())
                .arg(it.close())
                .arg(it.high())
                .arg(it.low())
                .arg(it.volume());

        QSqlQuery query( db->get() );
        if (!query.exec(insert))
            qDebug() << query.lastError().databaseText();
    };

    std::for_each(stock.begin(), stock.end(), insertCandle);
}

void StocksQuery::loadCandles(Stock &stock, const Range &range, const uint loadCandlesCount)
{
    if ( (range.isValid()) != (loadCandlesCount == 0) ) {
        logCritical << QString("StocksQuery::loadCandles:;invalid input data!;%1;%2;%3")
                       .arg(range.begin().toString(), range.end().toString())
                       .arg(loadCandlesCount);
        return;
    }

    auto *db = Glo.dataBase.data();
    if (!db->isOpen())
        return;

    QMutexLocker locker(&db->mutex);

    QString interval = stock.key().intervalToString();
    QString load = QString("SELECT * FROM stocks WHERE figi = '%1' and interval = '%2'").arg(stock.key().figi(), interval);

    if (range.begin().isValid())
        load += QString(" and time >= '%1'").arg(range.begin().toString("dd.MM.yyyy hh:mm:ss"));
    if (range.end().isValid()) {
        load += QString(" and time < '%1'").arg(range.end().toString("dd.MM.yyyy hh:mm:ss"));
        /* time<'%1' потому что если мы загружаем например 15 минутные свечи с 9:00:00 до 10:00:00,
         * свеча на 10:00:00 она относится к диапазону 10:00:00 - 10:15:00 */
    }

    bool isForwardLoading = range.isBeginValid() && range.isEndNull();
    if (isForwardLoading)
        load += QString(" ORDER BY time ASC");
    else
        load += QString(" ORDER BY time DESC");

    if (loadCandlesCount > 0)
        load += QString(" LIMIT %1").arg(loadCandlesCount);

    QSqlQuery query(db->get());
    if (!query.exec(load)) {
        logCritical << QString("StocksQuery;retrieveCandles();Error can't read calndles;%1;%2")
                       .arg(query.lastError().databaseText(), query.lastError().driverText());
    }

    while (query.next()) {
        auto position = isForwardLoading
                ? stock.end()
                : (stock.size() ? stock.begin() : stock.end());
        //проверить что загружается в правильном порядке!
        stock.insertCandle(position, Data::Candle( query.value(2).toDateTime(),    //dateTime
                                                   query.value(3).toReal(),        //open
                                                   query.value(4).toReal(),        //close
                                                   query.value(5).toReal(),        //high
                                                   query.value(6).toReal(),        //low
                                                   query.value(7).toLongLong()) ); //volume
    }
}

}
