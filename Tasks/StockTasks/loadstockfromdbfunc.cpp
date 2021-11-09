/** @todo ! комментарий для чего в LoadStockFromDbFunc::exec() в конце загружем 1 свечу
  *
  */

#include "loadstockfromdbfunc.h"

#include "Core/globals.h"
#include "DataBase/Query/stocksquery.h"

namespace Task {

LoadStockFromDbFunc::LoadStockFromDbFunc(const Data::StockKey &stockKey, const uint minCandlesCount)
    : IFunction("LoadStockFromDbFunc"), _minCandlesCount(minCandlesCount)
{
    _stock.create();
    _stock->setStockKey(stockKey);
}

//Загружает данные из заданного диапазона, но не менее minCandlesCount
void LoadStockFromDbFunc::exec()
{
    if (!_loadRange->isValid()) {
        logCritical << "LoadStockFromDbFunc::exec:;invalid loadRange!";
        return;
    }

    DB::StocksQuery::loadCandles(_stock, _loadRange->begin(), _loadRange->end());

    if (_stock->count() < _minCandlesCount)
        loadByCount();

    if (_stock->count() == 0) {
        //Загружаем одну свечу за диапазоном, это нужня для "неразрывности" данных
        DB::StocksQuery::loadCandles(_stock, QDateTime(), _loadRange->begin(), 1);
    }

    //Свечи загружаются отсортированными по убыванию, и так нужно для loadByCount()
    std::reverse(_stock->getCandles().begin(), _stock->getCandles().end());
}

void LoadStockFromDbFunc::setData(SharedInterface &inputData)
{
    _loadRange = inputData;
    if (!_loadRange->isValid())
        logCritical << "LoadStockFromDbFunc::setData:;invalid loadRange!";
}

SharedInterface &LoadStockFromDbFunc::getResult()
{
    return &_stock;
}

void LoadStockFromDbFunc::loadByCount()
{
    QDateTime end = _stock->count() == 0 ? _loadRange->begin() : _stock->range().end();
    uint remainsCount = _minCandlesCount - _stock->count();

    //Дату начала оставляем пустой, значит на неё не будет накладываться ограничения
    DB::StocksQuery::loadCandles(_stock, QDateTime(), end, remainsCount);
}

}
