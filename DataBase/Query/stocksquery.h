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
    StocksQuery();

    //Помещает список свечей candles в БД (таблица stocks)
    static void placeCandles(IDataBase *db, const StockKey &key, Candles &candles);
    //Извлекает свечи из базы данных (таблица stocks) в структуру candles
    static void retrieveCandles(IDataBase *db, const StockKey &key, Candles &candles, const Range &range = Range());
};

}

#endif // STOCKSQUERY_H
