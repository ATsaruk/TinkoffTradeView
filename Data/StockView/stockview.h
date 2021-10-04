/** @ingroup Data
  * @defgroup StockView */
#ifndef STOCKVIEW_H
#define STOCKVIEW_H

#include "deque"
#include "Data/range.h"
#include "Data/Stock/candle.h"

namespace Data {


/** @ingroup StockView
  * @brief Базовый класс доступа к свечной информации хранимой в Stock.candles
  *
  * Базовый класс обертка для Data::Stock, который позволяет получить доступ к свечной информации в заданном диапазоне Data::Range
  * @see StockViewGlobal, StockViewReference */
class StockView
{
public:
    using DequeIt = std::_Deque_iterator<Data::Candle, Data::Candle&, Data::Candle*>;
    using ConstDequeIt = const std::_Deque_iterator<Data::Candle, const Data::Candle&, const Data::Candle*>;

    StockView();
    virtual ~StockView();

    ///Возвращает диапазон дат, в котором доступны свечи (между итераторами begin() и end())
    const Range getRange() const;

    ///Возвращает число свечей в диапазоне range
    size_t size() const;

    ///Возвращает пару указатель на свечу с индексом index и признак что такая свеча существует
    std::pair<const Candle*, bool> operator [](size_t index);

    ///Возвращает пару const ссылку на свечу с индексом index и признак что такая свеча существует
    std::pair<const Candle&, bool> operator [](size_t index) const;

    ///Возвращает const итератор на элемент, дата которого не меньше чем time
    virtual ConstDequeIt lower_bound(const QDateTime &time) const = 0;

    ///Возвращает const итератор на элемент, дата которого больше чем time
    virtual ConstDequeIt upper_bound(const QDateTime &time) const = 0;

    ///итератор на первую свечу, время которой не меньше (lower_bound), чем range.getBegin()
    virtual DequeIt begin() = 0;

    ///итератор на свечу, время которой больше (upper_bound), чем range.getEnd()
    virtual DequeIt end() = 0;

    virtual ConstDequeIt begin() const = 0;
    virtual ConstDequeIt end() const = 0;

protected:
    Range range;                    //хранит ограничивающий интервал, т.е. интервал, в которым доступны свечи через данную обертку
    std::deque<Candle> nullVector;  //нулевой вектор, нужен для оператора [] const, в случае отсутсвии необходимой свечи
};


}

#endif // STOCKVIEW_H
