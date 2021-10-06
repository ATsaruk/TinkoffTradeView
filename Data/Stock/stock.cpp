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
    this->stockKey = stockKey;
}

Stock::Stock(Stock &&other)
{
    this->stockKey = std::move(other.stockKey);
    this->candles.swap(other.candles);
}

Stock &Stock::operator =(Stock &&other)
{
    this->stockKey = std::move(other.stockKey);
    this->candles.swap(other.candles);

    return *this;
}

void Stock::setStockKey(const StockKey &key)
{
    stockKey = key;
}

const StockKey &Stock::key() const
{
    return stockKey;
}

Range Stock::range() const
{
    if (candles.empty())
        return Range();
    return Range(candles.begin()->dateTime(), candles.rbegin()->dateTime());
}

size_t Stock::count() const
{
    return candles.size();
}

std::optional<const Candle*> Stock::find(const QDateTime &time) const
{
    for (const auto &it : candles)
        if (it.dateTime() == time)
            return &it;
    return std::nullopt;
}

Range Stock::append(Stock &stock)
{
    if (stock.candles.empty())
        return Range();

    Range newRange(stock.candles.begin()->dateTime(), stock.candles.rbegin()->dateTime());
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
    auto begin = std::remove_if(stock.candles.begin(), stock.candles.end(), outOfRange);
    stock.candles.erase(begin, stock.candles.end());

    if (newRange > existedRange)    ///@todo !сравнить производительность с std::merge
        std::move(stock.candles.begin(), stock.candles.end(), std::back_inserter(candles));
     else if (newRange < existedRange)
        std::move(stock.candles.rbegin(), stock.candles.rend(), std::front_inserter(candles));
     else   //На всякий случай, такого быть не должно!
        logCritical << QString("Stock::appendCandles();%1;%2;%3;%4")
                       .arg(newRange.getBegin().toString()).arg(newRange.getEnd().toString())
                       .arg(existedRange.getBegin().toString()).arg(existedRange.getEnd().toString());

    return newRange;
}

std::deque<Candle> &Stock::getCandles()
{
    return candles;
}

const std::deque<Candle> &Stock::getCandles() const
{
    return candles;
}

}
