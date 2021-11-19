/** @todo !комментарий для чего в LoadStockFromDbFunc::exec() в конце загружем 1 свечу
  *
  */

#include "loadstockfromdbfunc.h"

#include "Core/globals.h"
#include "DataBase/Query/stocksquery.h"

namespace Task {

LoadStockFromDbFunc::LoadStockFromDbFunc(const Data::StockKey &stockKey, const uint minCandlesCount)
    : IFunction("LoadStockFromDbFunc"),
      _minCandlesCount(minCandlesCount),
      _stock(stockKey)
{
    //_stock.create();
    //_stock->setStockKey(stockKey);
}

void LoadStockFromDbFunc::setData(SharedInterface &inputData)
{
    _loadRange = inputData;
}

//Загружает данные из заданного диапазона, но не менее minCandlesCount
void LoadStockFromDbFunc::exec()
{
    //_minCandlesCount должен быть 0! либо !_loadRange.isValid()! иначе пишем ошибку в лог!
    if ( (_loadRange->isValid()) != (_minCandlesCount == 0) ) {
        logCritical << QString("LoadStockFromDbFunc::exec:;invalid input data!;%1;%2;%3")
                       .arg(_loadRange->start().toString(), _loadRange->end().toString())
                       .arg(_minCandlesCount);
        return;
    }

    DB::StocksQuery::loadCandles(_stock, _loadRange, _minCandlesCount);

    if (_loadRange->isValid() && _stock->size() == 0) {
        //Производилась загрузка конкретного диапазона и ничего не загружено!
        //Загружаем одну свечу за диапазоном, это нужня для "неразрывности" данных
        Data::Range leftFromLoadRange(QDateTime(), _loadRange->start());
        DB::StocksQuery::loadCandles(_stock, leftFromLoadRange, 1);
    }
}

SharedInterface &LoadStockFromDbFunc::getResult()
{
    return &_stock;
}

}
