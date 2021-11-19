#ifndef RANGE_H
#define RANGE_H

#include <QDateTime>

namespace Data {


/** @ingroup Data
  * @brief Класс содержит поля начало и конец диапазона и предоставляет методы для работы с ними */
class Range
{
public:
    explicit Range();

    explicit Range(const QDateTime &start, const QDateTime &end);
    explicit Range(const QDateTime &date, const long &duration);  //см. фунцию setRange(date, duration)
    explicit Range(QDateTime &&start, QDateTime &&end) noexcept;

    Range(const Range &other);
    Range(Range &&other) noexcept;

    Range& operator= (const Range &other);
    Range& operator= (Range &&other) noexcept;

    bool operator< (const Range &other) const;
    bool operator> (const Range &other) const;

    ///@return начало диапазона
    QDateTime& start();
    ///@return конец диапазона
    QDateTime& end();

    ///@return начало диапазона
    const QDateTime& start() const;
    ///@return конец диапазона
    const QDateTime& end() const;

    ///@return TRUE - если start().isValid()
    bool isStartValid() const;
    ///@return TRUE - если end().isValid()
    bool isEndValid() const;
    ///@return TRUE - если being() и end() isValid и если start <= end
    bool isValid() const;

    ///@return TRUE - если start().isNull()
    bool isStartNull() const;
    ///@return TRUE - если end().isNull()
    bool isEndNull() const;
    ///@return TRUE - если start().isNull() и end().isNull()
    bool isNull() const;

    ///@return TRUE - если isValid();
    operator bool() const;

    ///@return количество секунд в диапазоне
    qint64 toSec() const;

    ///@return TRUE - если date находится внутри диапазона (нестрогая проверка <=, >=)
    bool contains(const QDateTime &date) const;
    ///@return TRUE - если диапазон other является поддиапазоном (нестрогая проверка <=, >=)
    bool contains(const Range &other) const;

    /** @brief Определяет имеет ли текущий диапазон пересечение с other
      * @return TRUE - если между диапазонами нет "зазора" */
    bool isIntersected(const Range &other) const;

    /** @brief Формирует начало и конец диапазона
      * @param[IN] date - начало/конец диапазона
      * @param[IN] duration - длительность диапазона
      *
      * Если duration положительное число, то интервал будет = [date ... date + duration].\n
      * Если duration отрицательное число, то интервал будет = [data + duration ... date], помним, что duration
      *   отрицательное значение и по факту оно будет вычтено из date. */
    void setRange(const QDateTime &date, const long &duration);

    ///Сдигает текущий интервал на заданное число секунд (может быть отрицательным)
    void shift(const long &secs);

    ///Ограничивает диапазон, делает не больше чем other
    void constrain(const Range &other);

    /** @brief Удаляет часть диапазона
      * @param[IN] other диапазон, который необходимо удалить
      * @warning Есть одно НО в работе функции:  если диапазон other находится внутри нашего диапазона (*this), по идее нужно разбивать
      * текущий диапазон на 2 диапазона "до" other и "после" other. Данная же функция обрежет текущий диапазон (*this) до начала other.start
      *
      * Результат выполнения:
      * 1. Если other находится внутри текущего диапазона(*this) этот случай описан в warning
      * 2. Если диапазоны не пересекаются, ничего не произойдет
      * 3. Если диапазоны пересекаются, и other находится справа, то this->end = other.start
      * 4. Если диапазоны пересекаются, и other находится слева, то this->start = other.end  */
    void remove(const Range &other);

private:
    QDateTime _start;
    QDateTime _end;
};

}

#endif // RANGE_H
