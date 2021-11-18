#include "stock.h"

#include <QString>

#include "Core/globals.h"

namespace Data {


Stock::Stock()
    : mutex(QReadWriteLock::Recursive),
      _candles(std::make_shared<std::deque<Candle>>())
{
}

Stock::Stock(const StockKey &stockKey)
    : _key(stockKey),
      _candles(std::make_shared<std::deque<Candle>>())
{
}

Stock::Stock(Stock &&other)
    : mutex(QReadWriteLock::Recursive),
      _key(std::move(other._key))
{
    _candles.swap(other._candles);
}

Stock &Stock::operator =(Stock &&other)
{
    _key = std::move(other._key);
    _candles.swap(other._candles);

    return *this;
}

Stock::~Stock()
{

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
    if (_candles->empty())
        return Range();
    return Range(_candles->begin()->dateTime(), _candles->rbegin()->dateTime());
}

size_t Stock::size() const
{
    return _candles->size();
}

/*const Candle* Stock::find(const QDateTime &time) const
{
    auto isEqual = [&time](const auto &it) { return it.dateTime() == time; };
    const auto &item = std::find_if(_candles->begin(), _candles->end(), isEqual);
    if (item == _candles->end())
        throw std::logic_error("Stock::find;can't find item!;");
    return &(*item);
}*/

/* про ignoreRightBorder, данный флаг нужен для проверки доступности свечей, после загрузки свечей от брокера,
 * дело в том, что биржа не работает в ночью, в выходные, в праздники, и допустим сейчас воскресенье 14.11.2021,
 * и мы произвели запрос свечей от брокера и получили свечи до пятницы и их может быть меньше чем запрошено в minCandleCount
 * но других свечей просто не существует! и именно такой случай учитывает ignoreRightBorder.
 * т.е. при ignoreRightBorder = true по сути достаточно, что бы totalRange.begin() был <= range.begin() */
bool Stock::isEnoughCandles(Range range, const size_t minCandleCount, const bool ignoreRightBorder) const
{
    auto totalRange = Stock::range();
    if (!totalRange.isValid())
        return false;

    if (range.isEndNull() && ignoreRightBorder)
        range.end() = totalRange.end();

    bool isCountEnough = minCandleCount > 0 && minCandleCount >= size();
    return isCountEnough || (range.isValid() && totalRange.contains(range));
}

Range Stock::append(Stock &stock)
{
    if (stock._candles->empty())
        return Range();

    Range newRange(stock._candles->begin()->dateTime(), stock._candles->rbegin()->dateTime());
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
    auto begin = std::remove_if(stock._candles->begin(), stock._candles->end(), outOfRange);
    stock._candles->erase(begin, stock._candles->end());

    if (newRange > existedRange)    ///@todo !сравнить производительность с std::merge
        std::move(stock._candles->begin(), stock._candles->end(), std::back_inserter(*_candles));
     else if (newRange < existedRange)
        std::move(stock._candles->rbegin(), stock._candles->rend(), std::front_inserter(*_candles));
     else   //На всякий случай, такого быть не должно!
        logCritical << QString("Stock::appendCandles();%1;%2;%3;%4")
                       .arg(newRange.begin().toString()).arg(newRange.end().toString())
                       .arg(existedRange.begin().toString()).arg(existedRange.end().toString());

    return newRange;
}

void Stock::insertCandle(const DequeIt &it, Candle &&candle)
{
    _candles->insert(it, std::move(candle));
}

/*void Stock::reverse()
{
    std::reverse(_candles->begin(), _candles->end());
}

void Stock::sort()
{
    std::sort(_candles->begin(), _candles->end());
}*/

const Stock::DequeIt Stock::begin() const
{
    return _candles->begin();
}

const Stock::DequeIt Stock::end() const
{
    return _candles->end();
}

const Stock::ReverseDequeIt Stock::rbegin() const
{
    return _candles->rbegin();
}

const Stock::ReverseDequeIt Stock::rend() const
{
    return _candles->rend();
}

Stock::Stock(Stock &other)
    : mutex(QReadWriteLock::Recursive),
      _key(other._key),
     _candles(other._candles)
{
}

}
