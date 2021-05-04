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
        return Range();

    return Range(curCandles.front().dateTime,
                 curCandles.back().dateTime);
}

Stock Stocks::insertCandles(const Stock &stock)
{
    QWriteLocker lock(&rwMutex);

    Stock &ownStock = findStock(stock.key);

    auto newStock = ownStock.appendStock(stock);

    if (!newStock.candles.empty()) {
        emit dataChanged();
        logInfo << QString("DataStocks;insertStockData();Append;%1;candles;%2;%3")
                   .arg(newStock.candles.size()).arg(newStock.candles.front().dateTime.toString(), newStock.candles.back().dateTime.toString());
    }
    return newStock;
}

Stock &Stocks::findStock(const StockKey &key)
{
    auto stock = std::find_if(stocks.begin(),
                              stocks.end(),
                              [&key](const auto &it){
        return key == it.key;
    } );
    if (stock == stocks.end()) {
        stocks.emplace_front(key);
        return stocks.front();
    }
    return *stock;
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

Stock Stock::appendStock(const Stock &appendStock)
{
    assert(key == appendStock.key && "Stock::appendStock(): the keys must be the same");

    Stock resultStock(appendStock.key);
    resultStock.candles = appendCandles(appendStock.candles);

    return resultStock;
}

Candles Stock::appendCandles(const Candles &appendCandles)
{
    Candles newCandles;

    std::copy_if(appendCandles.begin(),
                 appendCandles.end(),
                 std::back_inserter(newCandles),
                 [&ownCandles = candles] (const auto& candle) { return std::count(ownCandles.begin(), ownCandles.end(), candle) == 0; } );

    if (!newCandles.empty()) {
        candles.reserve(candles.size() + newCandles.size());
        std::copy(newCandles.begin(), newCandles.end(), std::back_inserter(candles));
        std::sort(candles.begin(), candles.end());
    }

    return newCandles;
}

}
