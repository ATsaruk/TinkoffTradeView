#ifndef STOCKREFERENCE_H
#define STOCKREFERENCE_H

#include <type_traits>

#include "stockview.h"
#include "Data/Stock/stock.h"

namespace Data {

///@fixme переделать интерфейс доступа

/** @ingroup StockView
  * @brief Обертка, предоставляющая доступ к свечам из Data::Stock
  * @param Locker - скласс локера, для Stock.mutex;
  *
  * Предоставляет доступ на чтение/запись (в зависимости от локера) к интервалу акции stock, на время жизни обертки
  * Stock.mutex блокируется по средством указанного локера. */
template<class Locker = QReadLocker>    //QReadLocker / QWriteLocker
class StockReference : public StockView
{
    QSharedPointer<Stock> stock; //ссылка на акцию, к которой предоставляется доступ
    Locker locker;      //локер, блокирующий доступ к акции

public:
    /** @brief Формирует временной интервал range
      * @param baseStock - акция, к которой предоставляется доступ
      * @param begin - время первой доступной свечи (lower_bound(begin))
      * @param end - время за последней доступной свечой (upper_bound(end))
      * @param minCandlesCount - минимальное количество доступных свечей
      * @warning правила формирования границ такие же, как у функции: @see Stocks::getCandlesForRead() */
    explicit StockReference(const QSharedPointer<Stock> &baseStock, const QDateTime &begin = QDateTime(), const QDateTime &end = QDateTime(), const size_t minCandlesCount = 0)
        : stock(baseStock), locker(const_cast<QReadWriteLock*>(&baseStock->mutex))
    {
        range = stock->range();
        Range newRange = Range((begin.isValid() ? begin : range.begin()), (end.isValid() ? end : range.end()));
        range.constrain(newRange);  //обрезаем диапазон доступный в переданной акцие до не более чем заданного

        if (minCandlesCount == 0)
            return;

        if (size() < minCandlesCount) {
            auto stockBeginIt = static_cast<DequeIt>(stock->getCandles().begin());
            auto curEndIt = upper_bound(range.end());
            size_t availableCandlesCount = std::distance(stockBeginIt, curEndIt);
            if (availableCandlesCount > minCandlesCount)
                availableCandlesCount = minCandlesCount;

            std::advance(curEndIt, -availableCandlesCount); //+1 т.к. curEndIt указывает на элемент за необходимым
            range.begin() = curEndIt->dateTime();
        }
    }

    StockReference(const StockReference &other) noexcept
        : StockReference(other.stock, other.range.begin(), other.range.end()) {}

    ///итератор на первую свечу, время которой не меньше (lower_bound), чем range.getBegin()
    DequeIt begin() const override
    {
        return lower_bound(range.begin());
    }

    ///итератор на свечу, время которой больше (upper_bound), чем range.getEnd()
    DequeIt end() const override
    {
        return upper_bound(range.end());
    }

    ///реверс итератор на первую свечу
    ReverseDequeIt rbegin() const override
    {
        auto time = range.end();
        const auto &candles = stock->getCandles();
        auto isNotGreateThanTime = [&time](const auto &it){ return it.dateTime() <= time; };
        return std::find_if(candles.rbegin(), candles.rend(), isNotGreateThanTime);
    }

    ///реверс итератор на последнюю свечу
    ReverseDequeIt rend() const override
    {
        auto time = range.begin();
        const auto &candles = stock->getCandles();
        auto isLessThanTime = [&time](const auto &it){ return it.dateTime() < time; };
        return std::find_if(candles.rbegin(), candles.rend(), isLessThanTime);
    }

protected:
    ///Возвращает const итератор на элемент, дата которого не меньше чем time
    DequeIt lower_bound(const QDateTime &time) const
    {
        const auto &candles = stock->getCandles();
        auto isNotLessThanTime = [&time](const auto &it){ return it.dateTime() >= time; };
        return std::find_if(candles.begin(), candles.end(), isNotLessThanTime);
    }

    ///Возвращает const итератор на элемент, дата которого больше чем time
    DequeIt upper_bound(const QDateTime &time) const
    {
        const auto &candles = stock->getCandles();
        auto isGeaterThanTime = [&time](const auto &it){ return it.dateTime() > time; };
        return std::find_if(candles.begin(), candles.end(), isGeaterThanTime);
    }
};


}

#endif // STOCKREFERENCE_H
