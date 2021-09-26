#include "stockviewglobal.h"

#include "Core/globals.h"

namespace Data {


StockViewGlobal::StockViewGlobal(const StockKey &key, const QDateTime &begin, const QDateTime &end)
{
    stock = Glo.stocks->getCandlesForRead(key, begin, end);
    range = stock->getRange();
}

StockViewGlobal::StockViewGlobal(const StockKey &key, const Range &range)
    : StockViewGlobal(key, range.getBegin(), range.getEnd())
{

}

std::vector<Candle>::const_iterator StockViewGlobal::begin()
{
    return nullVector.end();
}

std::vector<Candle>::const_iterator StockViewGlobal::end()
{
    return nullVector.end();
}

const std::vector<Candle>::const_iterator StockViewGlobal::begin() const
{
    if (!range.isValid())
        return nullVector.end();

    auto notLessThanBegin = [&](const auto &it){ return it.dateTime() >= range.getBegin(); };
    return std::find_if(stock->begin(), stock->end(), notLessThanBegin);
}

const std::vector<Candle>::const_iterator StockViewGlobal::end() const
{
    if (!range.isValid())
        return nullVector.end();

    auto notLessThanEnd = [&](const auto &it){ return it.dateTime() >= range.getEnd(); };
    return std::find_if(stock->begin(), stock->end(), notLessThanEnd);
}

}
