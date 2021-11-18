#ifndef STOCK_H
#define STOCK_H

#include <deque>
#include <QReadWriteLock>

#include "stockkey.h"
#include "candle.h"
#include "Data/range.h"

namespace Data {

class Stock
{
public:
    using DequeIt = std::_Deque_iterator<Data::Candle, const Data::Candle&, const Data::Candle*>;
    using ReverseDequeIt = std::reverse_iterator<DequeIt>;

    QReadWriteLock mutex;

    explicit Stock();
    explicit Stock(const StockKey &stockKey);

    explicit Stock(Stock &&other);
    Stock& operator =(Stock &&other);

    virtual ~Stock();

    ///Задать ключ акции
    void setStockKey(const StockKey &key);

    ///Возвращает ключ акции
    virtual const StockKey& key() const;

    ///Возвращает диапазон хранимых свечей
    virtual Range range() const;

    ///Возвращает количество хранимых свечей
    virtual size_t size() const;

    ///Ищет свечу с меткой времени равной time
    //virtual const Candle* find(const QDateTime &time) const;

    /** @brief Проверяет достаточно ли свечей доступно
      * @param range - доступен ли данный интервал?
      * @param minCandleCount - доступно ли minCandleCount свечей
      * @param ignoreRightBorder - пропустить проверку правой границы, данный флаг нужен для корректной проверки после
      * загрузки свечей от брокера, подробнее написано перед телом функции
      * @return true если _candles содержит интервал range и содержит minCandleCount свечей */
    virtual bool isEnoughCandles(Range range, const size_t minCandleCount, const bool ignoreRightBorder = false) const;

    ///Добавляет свечи appendCandles в список candles
    virtual Range append(Stock &stock);

    /** @brief Добавляет свечу в указанное место
      * @param it - итератор, после которого будет добавлена свеча
      * @param candle - добавляемая свеча */
    virtual void insertCandle(const DequeIt &it, Candle &&candle);

    /*///Производит реверс свечей
    virtual void reverse();

    ///Сортирует свечи по дате
    virtual void sort();*/

    ///итератор на начало deque со свечами
    virtual const DequeIt begin() const;

    ///итератор на конец deque со свечами
    virtual const DequeIt end() const;

    ///реверс итератор на начало deque со свечами
    virtual const ReverseDequeIt rbegin() const;

    ///реверс итератор на конец deque со свечами
    virtual const ReverseDequeIt rend() const;

protected:
    StockKey _key;
    std::shared_ptr<std::deque<Candle>> _candles;

    explicit Stock(Stock &other);
    Stock& operator =(const Stock &other) = delete;
};

}

#endif // STOCK_H
