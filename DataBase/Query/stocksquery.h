#ifndef STOCKSQUERY_H
#define STOCKSQUERY_H

#include "Data/range.h"
#include "Data/Stock/stocks.h"
#include "DataBase/idatabase.h"

namespace DB {

using namespace Data;

///Класс запросов свечной информации из БД
class StocksQuery
{
public:
    explicit StocksQuery();

    //Вставляет список свечей candles в БД (таблица stocks)
    static void insertCandles(IDataBase *db, const StockKey &key, Candles &candles);
    //Загружает свечи из базы данных (таблица stocks) в структуру candles
    static void loadCandles(IDataBase *db, const StockKey &key, Candles &candles, const Range &range = Range());
};

}

#endif // STOCKSQUERY_H
