/** @todo !!написать комментарий для чего в LoadStockFromDbFunc::exec() в конце загружем 1 свечу
  *
  */

#include "loadstockfromdbfunc.h"

#include "Core/globals.h"
#include "DataBase/Query/stocksquery.h"

namespace Task {

LoadStockFromDbFunc::LoadStockFromDbFunc(const Data::StockKey &stockKey, const uint minCandlesCount_)
    : IFunction("LoadStockFromDbFunc")
{
    stock->setStockKey(stockKey);
    minCandlesCount = minCandlesCount_;
}

/* Загружает данные из заданного диапазона, но не менее minCandlesCount,
 * и дата крайней левой свечи должна быть меньше или равно началу заданного диапазона */
void LoadStockFromDbFunc::exec()
{
    if (!loadRange->isValid()) {
        logCritical << "LoadStockFromDbFunc::exec:;invalid loadRange!";
        return;
    }

    DB::StocksQuery::loadCandles(stock, loadRange->getBegin(), loadRange->getEnd());

    if (stock->count() < minCandlesCount)
        loadByCount();

    if (stock->count() == 0) {
        //Загружаем одну свечу за диапазоном, это нужня для "неразрывности" данных
        DB::StocksQuery::loadCandles(stock, QDateTime(), loadRange->getBegin(), 1);
    }
}

void LoadStockFromDbFunc::setData(SharedInterface &inputData)
{
    loadRange = inputData;
    if (!loadRange->isValid())
        logCritical << "LoadStockFromDbFunc::setData:;invalid loadRange!";
}

SharedInterface &LoadStockFromDbFunc::getResult()
{
    return &stock;
}

void LoadStockFromDbFunc::loadByCount()
{
    //Дату начала оставляем пустой, значит на неё не будет накладываться ограничения
    QDateTime begin = QDateTime();
    QDateTime end = stock->count() == 0 ? loadRange->getBegin() : stock->range().getEnd();
    uint remainsCount = minCandlesCount - stock->count();

    DB::StocksQuery::loadCandles(stock, begin, end, remainsCount);
}

}
