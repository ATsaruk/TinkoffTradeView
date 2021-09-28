#ifndef STOCKVIEWREFERENCE_H
#define STOCKVIEWREFERENCE_H

#include "stockview.h"
#include "Data/Stock/stock.h"

namespace Data {


template<class Locker = QReadLocker>    //QReadLocker / QWriteLocker
class StockViewReference : public StockView
{
public:
    StockViewReference(const Stock &baseStock, const QDateTime &begin = QDateTime(), const QDateTime &end = QDateTime())
        : stock(baseStock), locker(const_cast<QReadWriteLock*>(&baseStock.mutex))
    {
        range = stock.range();
        Range newRange = Range(begin, end);
        range.constrain(newRange);  //обрезаем изначальный диапазон до не более чем заданного
        //если !newRange.isValid() (диапазон не задан), то constrain не будет иметь эффекта и будет использоваться весь диапазон stock.range()
    }

    StockViewReference(const Stock &baseStock, const Range &range)
        : StockViewReference(baseStock, range.getBegin(), range.getEnd()) { }

    std::vector<Candle>::const_iterator begin()
    {
        if constexpr (std::is_same_v<Locker, QWriteLocker>) {
            auto notLessThanBegin = [&](const auto &it){ return it.dateTime() >= range.getBegin(); };
            const auto &candles = stock.getCandles();
            return std::find_if(candles.begin(), candles.end(), notLessThanBegin);
        }
        return nullVector.end();
    }
    std::vector<Candle>::const_iterator end()
    {
        if constexpr (std::is_same_v<Locker, QWriteLocker>) {
            const auto &candles = stock.getCandles();
            auto notLessThanEnd = [&](const auto &it){ return it.dateTime() >= range.getEnd(); };
            return std::find_if(candles.begin(), candles.end(), notLessThanEnd);
        }
        return nullVector.end();
    }

    const std::vector<Candle>::const_iterator begin() const
    {
        auto notLessThanBegin = [&](const auto &it){ return it.dateTime() >= range.getBegin(); };
        const auto &candles = stock.getCandles();
        return std::find_if(candles.begin(), candles.end(), notLessThanBegin);
    }
    const std::vector<Candle>::const_iterator end() const
    {
        const auto &candles = stock.getCandles();
        auto notLessThanEnd = [&](const auto &it){ return it.dateTime() >= range.getEnd(); };
        return std::find_if(candles.begin(), candles.end(), notLessThanEnd);
    }

private:
    const Stock &stock;
    Locker locker;
};


}

#endif // STOCKVIEWREFERENCE_H
