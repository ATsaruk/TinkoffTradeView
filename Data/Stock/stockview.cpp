#include "stockview.h"

namespace Data {

StockView::StockView(QSharedPointer<Stock> &stock, const Range &targetRange, const size_t minCandlesCount)
    : Stock(*stock),
      locker(&stock->mutex),
      _range(Stock::range())       //инициализируем _range всем доступным диапазоном
{
    _range.constrain(targetRange);  //обрезаем диапазон доступный в переданной акцие до не более чем заданного

    if ( (targetRange.isValid()) != (minCandlesCount > 0) ) {
        throw std::logic_error("StockView::setRange;targetRange.isValid() && minCandlesCount > 0!");
    }

    if (_candles->empty() || minCandlesCount == 0)
        return;

    auto BeginIt = targetRange.isBeginNull() ? _candles->begin() : lower_bound(_range.begin());
    auto EndIt   = targetRange.isEndNull()   ? _candles->end()   : upper_bound(_range.end());       ///@todo !!возможно lower_bound
    size_t availableCandlesCount = std::distance(BeginIt, EndIt);
    if (availableCandlesCount > minCandlesCount)
        availableCandlesCount = minCandlesCount;

    if (targetRange.isBeginNull()) {
        std::advance(EndIt, -availableCandlesCount);
        _range.begin() = EndIt->dateTime();
    } else { //if (targetRange.isEndNull())
        std::advance(BeginIt, availableCandlesCount);
        _range.end() = BeginIt->dateTime();
    }
}

StockView::StockView(const StockView &other)
    : Stock(other._key),
      locker(other.locker.readWriteLock()),
      _range(other._range)
{
    _candles = other._candles;
}

const StockKey &StockView::key() const
{
    return _key;
}

Range StockView::range() const
{
    return _range;
}

size_t StockView::size() const {
    return std::distance(begin(), end());
}

bool StockView::isEnoughCandles(const Range &range, const size_t minCandleCount) const
{
    return Stock::isEnoughCandles(range, minCandleCount);
}

const Stock::DequeIt StockView::begin() const
{
    if (!_range.isValid())
        return _candles->end();
    return lower_bound(_range.begin());
}

const Stock::DequeIt StockView::end() const
{
    if (!_range.isValid())
        return _candles->end();
    return lower_bound(_range.end());
}

const Stock::ReverseDequeIt StockView::rbegin() const
{
    if (!_range.isValid())
        return _candles->rend();
    auto time = _range.end();
    auto isNotGreateThanTime = [&time](const auto &it){ return it.dateTime() <= time; };
    return std::find_if(_candles->rbegin(), _candles->rend(), isNotGreateThanTime);
}

const Stock::ReverseDequeIt StockView::rend() const
{
    if (!_range.isValid())
        return _candles->rend();
    auto time = _range.begin();
    auto isLessThanTime = [&time](const auto &it){ return it.dateTime() < time; };
    return std::find_if(_candles->rbegin(), _candles->rend(), isLessThanTime);
}

void StockView::setBegin(const QDateTime &time)
{
    if (_range.isEndValid() && _range.end() >= time)
        _range.begin() = time;
    else
        _range = Range();
}

void StockView::setEnd(const QDateTime &time)
{
    if (_range.isBeginValid() && _range.begin() <= time)
        _range.end() = time;
    else
        _range = Range();
}

Stock::DequeIt StockView::lower_bound(const QDateTime &time) const
{
    auto isNotLessThanTime = [&time](const auto &it){ return it.dateTime() >= time; };
    return std::find_if(_candles->begin(), _candles->end(), isNotLessThanTime);
}

Stock::DequeIt StockView::upper_bound(const QDateTime &time) const
{
    auto isGeaterThanTime = [&time](const auto &it){ return it.dateTime() > time; };
    return std::find_if(_candles->begin(), _candles->end(), isGeaterThanTime);
}


}
