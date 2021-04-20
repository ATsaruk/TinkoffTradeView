#include <QSqlQuery>
#include <QSqlError>

#include "stocksquery.h"
#include "Core/globals.h"

namespace DB {

StocksQuery::StocksQuery()
{

}

void StocksQuery::placeCandles(IDataBase *db, const StockKey &key, Candles &candles)
{
    QMutexLocker locker(&db->mutex);

    if (!db->isOpen())
        return;

    QString interval = key.intervalToString();
    for (const auto &it: candles) {
        QString insert = QString("INSERT INTO stocks (figi, interval, time, open, close, high, low, volume) VALUES ('%1', '%2', '%3', %4, %5, %6, %7, %8)")
                .arg(key.figi(),
                     interval,
                     it.dateTime.toString("yyyy-MM-dd hh:mm:ss"))
                .arg(it.open)
                .arg(it.close)
                .arg(it.high)
                .arg(it.low)
                .arg(it.volume);

        QSqlQuery query( db->get() );
        if (!query.exec(insert))
            qDebug() << query.lastError().databaseText();
    }
}

void StocksQuery::retrieveCandles(IDataBase *db, const StockKey &key, Candles &candles, const Range &range)
{
    QMutexLocker locker(&db->mutex);

    if (!db->isOpen())
        return;

    QString interval = key.intervalToString();
    QString load = QString("SELECT * FROM stocks WHERE figi='%1' and interval='%2'").arg(key.figi(), interval);

    if (range.isValid()) {
        load += QString(" and time>='%1'").arg(range.getBegin().toString("dd.MM.yyyy hh:mm:ss"));
        load += QString(" and time<'%1'").arg(range.getEnd().toString("dd.MM.yyyy hh:mm:ss"));
        /* time<'%1' потому что если мы загружаем например 15 минутные свечи с 9:00:00 до 10:00:00,
         * свеча на 10:00:00 она относится к диапазону 10:00:00 - 10:15:00 */
    }

    QSqlQuery query( db->get() );
    if (!query.exec(load)) {
        logCritical << QString("StocksQuery;retrieveCandles();Error can't read calndles;%1;%2")
                       .arg(query.lastError().databaseText(), query.lastError().driverText());
        return;
    }

    candles.reserve(candles.size() + query.size());

    while (query.next()) {
        Candle candle;

        //Заполняем свечную информацию
        candle.dateTime = query.value(2).toDateTime();
        candle.open = query.value(3).toReal();
        candle.close = query.value(4).toReal();
        candle.high = query.value(5).toReal();
        candle.low = query.value(6).toReal();
        candle.volume = query.value(7).toLongLong();

        candles.push_back( std::move(candle) );
    }
}

}
