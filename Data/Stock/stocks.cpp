#include "stocks.h"
#include "Core/global.h"

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

Candles Stocks::insertCandles(const StockKey &key, Candles &candles)
{
    Candles newCandles;
    if (candles.empty())
        return newCandles;     //Информация может отстутствовать, например если загрузка была за выходной день

    QWriteLocker lock(&rwMutex);

    Candles &dataCandles = stocks[key.keyToString()];
    dataCandles.reserve(dataCandles.size() + candles.size());

    bool isChanged = false;
    for (auto &newCandle : candles) {
        //Проверяем наличие свечи candle в списке candlesData.candles
        bool isExisted = std::any_of( dataCandles.begin(), dataCandles.end(), [&newCandle] (const auto& candle) {return candle == newCandle;} );

        //Если свеча не найдена, добавляем ее
        if (!isExisted) {
            dataCandles.push_back(newCandle);
            newCandles.push_back(newCandle);
            if (!isChanged)
                isChanged = true;
        }
    }

    std::sort(dataCandles.begin(), dataCandles.end());

    if (isChanged) {
        emit dataChanged();
        logInfo << QString("DataStocks;insertStockData();Append;%1;candles;%2;%3")
                   .arg(newCandles.size()).arg(newCandles.front().dateTime.toString(), newCandles.back().dateTime.toString());
    }

    return newCandles;
}

}
