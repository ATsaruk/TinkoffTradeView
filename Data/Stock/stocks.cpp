#include "stocks.h"
#include "Core/globals.h"

namespace Data {

Stocks::Stocks()
{

}

const Candles &Stocks::getStock(const StockKey &key)
{
    return stocks[key.keyToString()];
}

long Stocks::getCandlesCount(const StockKey &key)
{
    QReadLocker lock(&rwMutex);

    return stocks[key.keyToString()].size();
}

Range Stocks::getRange(const StockKey &key)
{
    QReadLocker lock(&rwMutex);

    Candles &curCandles = stocks[key.keyToString()];
    if (curCandles.empty())
        throw std::logic_error("candles is empty");

    return Range(curCandles.front().dateTime,
                 curCandles.back().dateTime);
}

Candles Stocks::insertCandles(const StockKey &key, const Candles &candles)
{
    Candles newCandles;
    if (candles.empty())
        return newCandles;

    QWriteLocker lock(&rwMutex);
    Candles &stock = stocks[key.keyToString()];

    std::copy_if(candles.begin(),
                 candles.end(),
                 std::back_inserter(newCandles),
                 [&stock] (const auto& candle) { return std::count(stock.begin(), stock.end(), candle) == 0; } );

    if (!newCandles.empty()) {
        stock.reserve(stock.size() + newCandles.size());
        std::copy(newCandles.begin(), newCandles.end(), std::back_inserter(stock));
        std::sort(stock.begin(), stock.end());

        emit dataChanged();
        logInfo << QString("DataStocks;insertStockData();Append;%1;candles;%2;%3")
                   .arg(newCandles.size()).arg(newCandles.front().dateTime.toString(), newCandles.back().dateTime.toString());
    }
    return newCandles;
}

}
