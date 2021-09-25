#include "stockviewreference.h"

namespace Data {


StockViewReference::StockViewReference(const Stock &baseStock, const QDateTime &begin, const QDateTime &end)
    : cRef(baseStock)
{
    range = cRef.range();
    Range newRange = Range(begin, end);
    if (newRange.isValid()) {
        //обрезаем изначальный диапазон до не более чем заданного
        range.constrain(newRange);
    }
    //если диапазон не задан, то используем весь доступный диапазон
}

const std::vector<Candle>::const_iterator StockViewReference::begin() const
{
    auto notLessThanBegin = [&](const auto &it){ return it.dateTime() >= range.getBegin(); };
    const auto &candles = cRef.getCandles();
    return std::find_if(candles.begin(), candles.end(), notLessThanBegin);
}

const std::vector<Candle>::const_iterator StockViewReference::end() const
{
    const auto &candles = cRef.getCandles();
    auto notLessThanEnd = [&](const auto &it){ return it.dateTime() >= range.getEnd(); };
    return std::find_if(candles.begin(), candles.end(), notLessThanEnd);
}

}
