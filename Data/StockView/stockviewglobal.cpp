#include "stockviewglobal.h"

#include "Core/globals.h"

namespace Data {


StockViewGlobal::StockViewGlobal(const StockKey &key, const QDateTime &begin, const QDateTime &end, const size_t minCandlesCount)
{
    stock = Glo.stocks->getCandlesForRead(key, begin, end, minCandlesCount);
    range = stock->getRange();
}

StockView::ConstDequeIt StockViewGlobal::lower_bound(const QDateTime &time) const
{
    if (!range.isValid())
        return nullVector.end();

    auto isNotLessThanTime = [&time](const auto &it){ return it.dateTime() >= time; };
    return std::find_if(stock->begin(), stock->end(), isNotLessThanTime);
}

StockView::ConstDequeIt StockViewGlobal::upper_bound(const QDateTime &time) const
{
    if (!range.isValid())
        return nullVector.end();

    auto isGreaterThanTime = [&time](const auto &it){ return it.dateTime() > time; };
    return std::find_if(stock->begin(), stock->end(), isGreaterThanTime);
}

std::deque<Candle>::iterator StockViewGlobal::begin()
{
    if (!range.isValid())
        return nullVector.end();

    auto notLessThanBegin = [&](const auto &it){ return it.dateTime() >= range.getBegin(); };
    return std::find_if(stock->begin(), stock->end(), notLessThanBegin);
}

std::deque<Candle>::iterator StockViewGlobal::end()
{
    if (!range.isValid())
        return nullVector.end();

    auto notLessThanEnd = [&](const auto &it){ return it.dateTime() > range.getEnd(); };
    return std::find_if(stock->begin(), stock->end(), notLessThanEnd);
}

StockView::ConstDequeIt StockViewGlobal::begin() const
{
    if (!range.isValid())
        return nullVector.end();

    auto notLessThanBegin = [&](const auto &it){ return it.dateTime() >= range.getBegin(); };
    return std::find_if(stock->begin(), stock->end(), notLessThanBegin);
}

StockView::ConstDequeIt StockViewGlobal::end() const
{
    if (!range.isValid())
        return nullVector.end();

    auto notLessThanEnd = [&](const auto &it){ return it.dateTime() > range.getEnd(); };
    return std::find_if(stock->begin(), stock->end(), notLessThanEnd);
}

}
