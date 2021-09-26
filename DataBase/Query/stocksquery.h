#ifndef STOCKSQUERY_H
#define STOCKSQUERY_H

#include "Data/range.h"
#include "Data/stocks.h"
#include "DataBase/idatabase.h"

namespace DB {

using namespace Data;

///Класс запросов свечной информации из БД
class StocksQuery
{
public:
    explicit StocksQuery();

    //Вставляет список свечей candles в БД (таблица stocks)
    static void insertCandles(const Stock &stock);
    //Загружает свечи из базы данных (таблица stocks) в структуру candles
    static void loadCandles(Stock &stock, const QDateTime &begin, const QDateTime &end = QDateTime(), const uint candleCount = 0);
};

}

#endif // STOCKSQUERY_H
