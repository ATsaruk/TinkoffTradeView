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
    : _key(stockKey)
{
}

Stock::Stock(Stock &&other)
    : _key(std::move(other._key))
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

bool Stock::isEnoughCandles(const Range &range, const size_t minCandleCount) const
{
    auto totalRange = Stock::range();
    return totalRange.isValid()
            && range.isValid()
            && totalRange.contains(range)
            && size() >= minCandleCount;
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

void Stock::reverse()
{
    std::reverse(_candles->begin(), _candles->end());
}

void Stock::sort()
{
    std::sort(_candles->begin(), _candles->end());
}

/* Проверяем последнюю свечу и если она незавершенная удаляем ее, поясню зачем это:
 * Например сейчас 17:42 и мы запрашиваем информацию по 15 минутным свечам с начала суток.
 * Время у последней полученной свечи будет 17:30 и если ничего не делать, то эта свеча будет записана в базу данных с таким временем.
 * И допустим через час мы захотим вновь загрузить свечную информацию, в итоге мы опять получим свечу на 17:30, только в этот раз она
 * будет содержать в себе полные данные за 15 минут, но в базу данных она уже не запишется, т.к. так уже етсь свеча на 17:30!
 * Т.к. в базе данных primary key для записи это figi + interval + time.
 * В итоге свеча так и останется незавершенной! это было вяснено постфактум, когда заметил отличие на моем графике и графике брокера! */
void Stock::removeIncompleteCandle()
{
    if (_candles->empty())
        return;

    const Data::Candle &lastCandle = _candles->back();

    QDateTime lastCompleteCandle = _key.prevCandleTime(QDateTime::currentDateTime());
    if (lastCandle.dateTime() >= lastCompleteCandle)
        _candles->pop_back();
}

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
    :_key(other._key),
     _candles(other._candles)
{
}

}
