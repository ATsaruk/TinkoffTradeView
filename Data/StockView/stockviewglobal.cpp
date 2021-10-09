#include "stockviewglobal.h"

#include "Core/globals.h"

namespace Data {


StockViewGlobal::StockViewGlobal(const StockKey &key, const QDateTime &begin, const QDateTime &end, const size_t minCandlesCount)
{
    stock = Glo.stocks->getCandlesForRead(key, begin, end, minCandlesCount);
    range = stock->getRange();
}

StockView::DequeIt StockViewGlobal::begin() const
{
    return lower_bound(range.getBegin());
}

StockView::DequeIt StockViewGlobal::end() const
{
    return upper_bound(range.getEnd());
}

StockView::ReverseDequeIt StockViewGlobal::rbegin() const
{
    if (!range.isValid())
        return nullVector.rend();

    auto time = range.getEnd();
    auto isNotGreateThanTime = [&time](const auto &it){ return it.dateTime() <= time; };
    return std::find_if(stock->rbegin(), stock->rend(), isNotGreateThanTime);
}

StockView::ReverseDequeIt StockViewGlobal::rend() const
{
    if (!range.isValid())
        return nullVector.rend();

    auto time = range.getBegin();
    auto isLessThanTime = [&time](const auto &it){ return it.dateTime() < time; };
    return std::find_if(stock->rbegin(), stock->rend(), isLessThanTime);
}

StockView::DequeIt StockViewGlobal::lower_bound(const QDateTime &time) const
{
    if (!range.isValid())
        return nullVector.end();

    auto isNotLessThanTime = [&time](const auto &it){ return it.dateTime() >= time; };
    return std::find_if(stock->begin(), stock->end(), isNotLessThanTime);
}

StockView::DequeIt StockViewGlobal::upper_bound(const QDateTime &time) const
{
    if (!range.isValid())
        return nullVector.end();

    auto isGreaterThanTime = [&time](const auto &it){ return it.dateTime() > time; };
    return std::find_if(stock->begin(), stock->end(), isGreaterThanTime);
}

}
