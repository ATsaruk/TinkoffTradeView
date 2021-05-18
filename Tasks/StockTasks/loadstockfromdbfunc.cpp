#include "loadstockfromdbfunc.h"

#include "DataBase/Query/stocksquery.h"

namespace Task {

LoadStockFromDbFunc::LoadStockFromDbFunc(const Data::StockKey &stockKey, const uint minCandlesCount_)
    : IFunction("LoadStockFromDbFunc")
{
    if (stockKey.interval() == Data::StockKey::INTERVAL::ANY)
        throw std::logic_error("LoadStock(): can't load stock with ANY interval!");

    stock->key = stockKey;
    minCandlesCount = minCandlesCount_;
}

/* Загружает данные из заданного диапазона, но не менее minCandlesCount,
 * и дата крайней левой свечи должна быть меньше или равно началу заданного диапазона
 */
void LoadStockFromDbFunc::exec()
{
    DB::StocksQuery::loadCandles(stock, loadRange->getBegin(), loadRange->getEnd());

    if (stock->candles.size() < minCandlesCount)
        loadByCount();

    if (!stock->candles.empty()) {
        if (stock->candles.front().dateTime > loadRange->getBegin()) {
            //Загружаем ещё одну свечу
            QDateTime &end = std::min_element(stock->candles.begin(), stock->candles.end())->dateTime;
            DB::StocksQuery::loadCandles(stock, QDateTime(), end, 1);
        }
        std::sort(stock->candles.begin(), stock->candles.end());
    }
}

void LoadStockFromDbFunc::setData(SharedInterface &inputData)
{
    loadRange = inputData;
}

SharedInterface &LoadStockFromDbFunc::getResult()
{
    return &stock;
}

void LoadStockFromDbFunc::loadByCount()
{
    //Дату начала оставляем пустой, значит на неё не будет накладываться ограничения
    QDateTime begin = QDateTime();
    QDateTime end = stock->candles.empty() ? loadRange->getBegin() : stock->candles.back().dateTime;
    uint remainsCount = minCandlesCount - stock->candles.size();

    DB::StocksQuery::loadCandles(stock, begin, end, remainsCount);
}

}
