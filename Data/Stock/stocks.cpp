#include "stocks.h"
#include "Core/globals.h"

namespace Data {

Stocks::Stocks()
{

}

const Candles &Stocks::getCandles(const StockKey &key)
{
    return findStock(key).candles;
}

long Stocks::getCandlesCount(const StockKey &key)
{
    QReadLocker lock(&rwMutex);

    return findStock(key).candles.size();
}

Range Stocks::getRange(const StockKey &key)
{
    QReadLocker lock(&rwMutex);

    Candles &curCandles = findStock(key).candles;
    if (curCandles.empty())
        throw std::logic_error("candles is empty");

    return Range(curCandles.front().dateTime,
                 curCandles.back().dateTime);
}

Stock Stocks::insertCandles(const Stock &appendStock)
{
    QWriteLocker lock(&rwMutex);

    Candles &candles = findStock(appendStock.key).candles;

    Stock resultStock(appendStock.key);
    auto &newCandles = resultStock.candles;

    std::copy_if(appendStock.candles.begin(),
                 appendStock.candles.end(),
                 std::back_inserter(newCandles),
                 [&candles] (const auto& candle) { return std::count(candles.begin(), candles.end(), candle) == 0; } );

    if (!newCandles.empty()) {
        candles.reserve(candles.size() + newCandles.size());
        std::copy(newCandles.begin(), newCandles.end(), std::back_inserter(candles));
        std::sort(candles.begin(), candles.end());

        emit dataChanged();
        logInfo << QString("DataStocks;insertStockData();Append;%1;candles;%2;%3")
                   .arg(newCandles.size()).arg(newCandles.front().dateTime.toString(), newCandles.back().dateTime.toString());
    }
    return resultStock;
}

Stock &Stocks::findStock(const StockKey &key)
{
    return *(std::find_if(stocks.begin(),
                          stocks.end(),
                          [&key](const auto &it){
        return key == it.key;
    } ));
}

Stock::Stock()
{

}

Stock::Stock(const StockKey &key_)
{
    key = key_;
}

bool Stock::operator==(const Stock &other) const
{
    return key == other.key;
}

}
