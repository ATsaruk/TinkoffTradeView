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
    auto db = Glo.dataBase;
    QMutexLocker locker(&db->mutex);

    if (!db->isOpen())
        return;

    QString interval = stock.key.intervalToString();
    for (const auto &it: stock.candles) {
        QString insert = QString("INSERT INTO stocks (figi, interval, time, open, close, high, low, volume) VALUES ('%1', '%2', '%3', %4, %5, %6, %7, %8)")
                .arg(stock.key.figi(),
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

void StocksQuery::loadCandles(Stock &stock, const QDateTime &begin, const QDateTime &end, const uint candleCount)
{
    auto db = Glo.dataBase;
    QMutexLocker locker(&db->mutex);

    if (!db->isOpen())
        return ;

    QString interval = stock.key.intervalToString();
    QString load = QString("SELECT * FROM stocks WHERE figi='%1' and interval='%2'").arg(stock.key.figi(), interval);

    if (begin.isValid())
        load += QString(" and time>='%1'").arg(begin.toString("dd.MM.yyyy hh:mm:ss"));
    if (end.isValid()) {
        load += QString(" and time<'%1'").arg(end.toString("dd.MM.yyyy hh:mm:ss"));
        /* time<'%1' потому что если мы загружаем например 15 минутные свечи с 9:00:00 до 10:00:00,
         * свеча на 10:00:00 она относится к диапазону 10:00:00 - 10:15:00 */
    }

    load += QString(" ORDER BY time DESC");

    if (candleCount > 0)
        load += QString(" LIMIT %1").arg(candleCount);

    QSqlQuery query(db->get());
    if (!query.exec(load)) {
        logCritical << QString("StocksQuery;retrieveCandles();Error can't read calndles;%1;%2")
                       .arg(query.lastError().databaseText(), query.lastError().driverText());
    }

    stock.candles.reserve(stock.candles.size() + query.size());
    while (query.next()) {
        stock.candles.emplace_back( query.value(2).toDateTime(),   //dateTime
                                    query.value(3).toReal(),       //open
                                    query.value(4).toReal(),       //close
                                    query.value(5).toReal(),       //high
                                    query.value(6).toReal(),       //low
                                    query.value(7).toLongLong() ); //volume
    }
}

}
