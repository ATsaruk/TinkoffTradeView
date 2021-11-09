#include "stock.h"

#include <QString>

#include "Core/globals.h"

namespace Data {


Stock::Stock()
    : mutex(QReadWriteLock::Recursive)
{

}

Stock::Stock(const StockKey &stockKey)
{
    this->_key = stockKey;
}

Stock::Stock(Stock &&other)
{
    this->_key = std::move(other._key);
    this->_candles.swap(other._candles);
}

Stock &Stock::operator =(Stock &&other)
{
    this->_key = std::move(other._key);
    this->_candles.swap(other._candles);

    return *this;
}

void Stock::setStockKey(const StockKey &key)
{
    _key = key;
}

const StockKey &Stock::key() const
{
    return _key;
}

Range Stock::range() const
{
    if (_candles.empty())
        return Range();
    return Range(_candles.begin()->dateTime(), _candles.rbegin()->dateTime());
}

size_t Stock::count() const
{
    return _candles.size();
}

const Candle* Stock::find(const QDateTime &time) const
{
    auto isEqual = [&time](const auto &it) { return it.dateTime() == time; };
    const auto &item = std::find_if(_candles.begin(), _candles.end(), isEqual);
    if (item == _candles.end())
        throw std::logic_error("Stock::find;can't find item!;");
    return &(*item);
}

Range Stock::append(Stock &stock)
{
    if (stock._candles.empty())
        return Range();

    Range newRange(stock._candles.begin()->dateTime(), stock._candles.rbegin()->dateTime());
    if (!newRange.isValid())
        return Range();

    Range existedRange = range();
    if (existedRange.contains(newRange))
        return Range(); //Добавляемые свечи уже существуют!

    newRange.remove(existedRange);  //Вычитаем из диапазаона newRange диапазон existedRange

    //Удаляем свечи, которые находятся вне диапазона newRange
    const auto &outOfRange = [&newRange](const auto &it) {
        return !newRange.contains(it.dateTime());
    };
    auto begin = std::remove_if(stock._candles.begin(), stock._candles.end(), outOfRange);
    stock._candles.erase(begin, stock._candles.end());

    if (newRange > existedRange)    ///@todo !сравнить производительность с std::merge
        std::move(stock._candles.begin(), stock._candles.end(), std::back_inserter(_candles));
     else if (newRange < existedRange)
        std::move(stock._candles.rbegin(), stock._candles.rend(), std::front_inserter(_candles));
     else   //На всякий случай, такого быть не должно!
        logCritical << QString("Stock::appendCandles();%1;%2;%3;%4")
                       .arg(newRange.begin().toString()).arg(newRange.end().toString())
                       .arg(existedRange.begin().toString()).arg(existedRange.end().toString());

    return newRange;
}

std::deque<Candle> &Stock::getCandles()
{
    return _candles;
}

const std::deque<Candle> &Stock::getCandles() const
{
    return _candles;
}

}
