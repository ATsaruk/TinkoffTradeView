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
    if (stock.candles.empty())    ///@todo !отладить
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
        return newRange.contains(it.dateTime());
    };
    auto begin = std::remove_if(stock.candles.begin(), stock.candles.end(), outOfRange);
    stock.candles.erase(begin, stock.candles.end());

    if (newRange > existedRange) {
        ///@todo !!сравнить производительность с std::merge
        candles.reserve(candles.size() + stock.candles.size());
        std::move(stock.candles.begin(), stock.candles.end(), candles.end());
    } else if (newRange < existedRange) {
        stock.candles.reserve(candles.size() + stock.candles.size());
        std::move(candles.begin(), candles.end(), stock.candles.end());
        candles.swap(stock.candles);
    } else {
        //На всякий случай, такого быть не должно!
        logCritical << QString("Stock::appendCandles();%1;%2;%3;%4")
                       .arg(newRange.getBegin().toString()).arg(newRange.getEnd().toString())
                       .arg(existedRange.getBegin().toString()).arg(existedRange.getEnd().toString());
    }

    return newRange;
}

std::vector<Candle> &Stock::getCandles()
{
    ///@todo !!паттерн visitor?
    return candles;
}

const std::vector<Candle> &Stock::getCandles() const
{
    return candles;
}

}
