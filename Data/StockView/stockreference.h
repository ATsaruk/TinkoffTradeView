#ifndef STOCKREFERENCE_H
#define STOCKREFERENCE_H

#include <type_traits>

#include "stockview.h"
#include "Data/Stock/stock.h"

namespace Data {


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
    StockReference(const QSharedPointer<Stock> &baseStock, const QDateTime &begin = QDateTime(), const QDateTime &end = QDateTime(), const size_t minCandlesCount = 0)
        : stock(baseStock), locker(const_cast<QReadWriteLock*>(&baseStock->mutex))
    {
        range = stock->range();
        Range newRange = Range((begin.isValid() ? begin : range.getBegin()), (end.isValid() ? end : range.getEnd()));
        range.constrain(newRange);  //обрезаем диапазон доступный в переданной акцие до не более чем заданного

        if (minCandlesCount  > 0) {
            auto it = upper_bound(range.getEnd());
            size_t availableCandlesCount = std::distance(static_cast<ConstDequeIt>(stock->getCandles().begin()), it);
            if (availableCandlesCount > minCandlesCount)
                availableCandlesCount = minCandlesCount;
            std::advance(it, -availableCandlesCount);
            if (it != stock->getCandles().begin())
                range.setBegin(it->dateTime());
        }
    }

    StockReference(const StockReference &other) noexcept
        : StockReference(other.stock, other.range.getBegin(), other.range.getEnd()) {}

    ///Возвращает const итератор на элемент, дата которого не меньше чем time
    ConstDequeIt lower_bound(const QDateTime &time) const override
    {
        const auto &candles = stock->getCandles();
        auto isNotLessThanTime = [&time](const auto &it){ return it.dateTime() >= time; };
        return std::find_if(candles.begin(), candles.end(), isNotLessThanTime);
    }

    ///Возвращает const итератор на элемент, дата которого больше чем time
    ConstDequeIt upper_bound(const QDateTime &time) const override
    {
        const auto &candles = stock->getCandles();
        auto isGeaterThanTime = [&time](const auto &it){ return it.dateTime() > time; };
        return std::find_if(candles.begin(), candles.end(), isGeaterThanTime);
    }

    ///итератор на первую свечу, время которой не меньше (lower_bound), чем range.getBegin()
    DequeIt begin() override
    {
        if constexpr (std::is_same_v<Locker, QReadLocker>) {
            //для QReadLocker доступен только ConstDequeIt (begin() const)
            throw std::logic_error("StockViewReference::begin();try begin() with QReadLocker! use begin() const!");
        } else {
            return lower_bound(range.getBegin());
        }
    }

    ///итератор на свечу, время которой больше (upper_bound), чем range.getEnd()
    DequeIt end() override
    {
        if constexpr (std::is_same_v<Locker, QReadLocker>) {
            //для QReadLocker доступен только ConstDequeIt (end() const)
            throw std::logic_error("StockViewReference::end();try begin() with QReadLocker! use end() const!");
        } else {
            return lower_bound(range.getEnd());
        }
    }

    ConstDequeIt begin() const override
    {
        return lower_bound(range.getBegin());
    }
    ConstDequeIt end() const override
    {
        return upper_bound(range.getEnd());
    }
};


}

#endif // STOCKREFERENCE_H
