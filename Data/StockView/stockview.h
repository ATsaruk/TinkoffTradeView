/** @ingroup Data
  * @defgroup StockView */
#ifndef STOCKVIEW_H
#define STOCKVIEW_H

#include "deque"
#include "Data/range.h"
#include "Data/Stock/candle.h"

namespace Data {


/** @todo !Подумать как это реализовать по другому, получилось как то сложно/замороченно и пока непонятно насколько удобно?
  * возможно просто стоит в класс Stock добавить 2 итератора на первый и последний элемент для итерированния в заданном
  * диапазоне, итераторы эти менять при вызове метода Stock::setRange(Data::Range). А мютекс блокировать ручками...
  * Но тут возникает вопрос, есть 2 обхекта, каждый из которых хочет посмотреть какой то диапазон дат, и обоих он разный...
  * Можно сделать функция std::pair<ConstDequeIt, ConstDequeIt> getRange(Data::Range range), которая будет возвращать
  * итератор на начало и конец диапазона и каждый будет работать со своим диапазоном, вызов будет такой:
  *     ... где то добыли Stock stock (интересующая нас акци) и Range range(необходимый диапазон)
  *     QReadLocker loker (&stock.mutex);
  *     auto [begin, end] = stock.getRange(range);
  *     std::for_each(begin, end, [](const auto &it){...});
  * В любом случае нужна какая то обертка, подумать!
  * Сейчас в плане рефакторин Plotter'a и при этом станет понятно на сколько удобен или наоборот неудобен StockView и
  * возможно всплывут какие то новые требования к оберкте */


/** @ingroup StockView
  * @brief Базовый класс доступа к свечной информации хранимой в Stock.candles
  *
  * Базовый класс обертка для Data::Stock, который позволяет получить доступ к свечной информации в заданном диапазоне Data::Range
  * @see StockViewGlobal, StockViewReference */
class StockView
{
public:
    using DequeIt = std::_Deque_iterator<Data::Candle, const Data::Candle&, const Data::Candle*>;
    using ReverseDequeIt = std::reverse_iterator<DequeIt>;

    StockView();
    virtual ~StockView();

    ///Возвращает диапазон дат, в котором доступны свечи (между итераторами begin() и end())
    const Range getRange() const;

    [[nodiscard]] bool empty() const;

    ///Возвращает число свечей в диапазоне range
    size_t size() const;

    ///Возвращает пару const ссылку на свечу с индексом index и признак что такая свеча существует
    std::pair<const Candle*, bool> at(size_t index) const;

    ///изменить дату начала просматриваемого интервала
    void setBegin(const QDateTime &time);

    ///изменить дату конца просматриваемого интервала
    void setEnd(const QDateTime &time);

    ///итератор на первую свечу, время которой не меньше (lower_bound), чем range.getBegin()
    virtual DequeIt begin() const = 0;

    ///итератор на свечу, время которой больше (upper_bound), чем range.getEnd()
    virtual DequeIt end() const = 0;

    ///реверс итератор на первую свечу
    virtual ReverseDequeIt rbegin() const = 0;

    ///реверс итератор на последнюю свечу
    virtual ReverseDequeIt rend() const = 0;

protected:
    Range range;                    //хранит ограничивающий интервал, т.е. интервал, в которым доступны свечи через данную обертку
    std::deque<Candle> nullVector;  //нулевой вектор, нужен для оператора [] const, в случае отсутсвии необходимой свечи
};


}

#endif // STOCKVIEW_H
