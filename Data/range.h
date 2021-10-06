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

    Range(const Range &range);
    Range(const QDateTime &begin_, const QDateTime &end_);
    Range(const QDateTime &date, const long &duration);  //см. фунцию setRange(date, duration)
    Range& operator= (const Range &range);

    Range(Range &&range) noexcept;
    Range(QDateTime &&begin_, QDateTime &&end_) noexcept;
    Range& operator= (Range &&range) noexcept;

    ///@return начало диапазона
    const QDateTime& getBegin() const;
    ///@return конец диапазона
    const QDateTime& getEnd() const;
    ///@return FALSE - если одна из дат !isValid или если begin > end
    bool isValid() const;

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

    ///@return TRUE - если начале текущего диапазона находится правее конца диапазона other
    bool operator >(const Range &other) const;
    ///@return TRUE - если конец текущего диапазона находится левее начала диапазона other
    bool operator <(const Range &other) const;

    ///Задает начало диапазона
    void setBegin(const QDateTime &begin_);
    ///Задает конец диапазона
    void setEnd(const QDateTime &end_);

    /** @brief Формирует начало и конец диапазона
      * @param[IN] date - начало/конец диапазона
      * @param[IN] duration - длительность диапазона
      *
      * Если duration положительное число, то интервал будет = [date ... date + duration].\n
      * Если duration отрицательное число, то интервал будет = [data + duration ... date], помним, что duration
      *   отрицательное значение и по факту оно будет вычтено из date. */
    void setRange(const QDateTime &date, const long &duration);

    ///Добавляет заданное число секунд к дате начала и дате конца (может быть отрицательным)
    void addSecs(const long secs);

    ///Ограничивает диапазон, делает не больше чем other
    void constrain(const Range &other);

    /** @brief Удаляет часть диапазона
      * @param[IN] other диапазон, который необходимо удалить
      * @warning Есть одно НО в работе функции:  если диапазон other находится внутри нашего диапазона (*this), по идее нужно разбивать
      * текущий диапазон на 2 диапазона "до" other и "после" other. Данная же функция обрежет текущий диапазон (*this) до начала other.begin
      *
      * Результат выполнения:
      * 1. Если other находится внутри текущего диапазона(*this) этот случай описан в warning
      * 2. Если диапазоны не пересекаются, ничего не произойдет
      * 3. Если диапазоны пересекаются, и other находится справа, то this->end = other.begin
      * 4. Если диапазоны пересекаются, и other находится слева, то this->begin = other.end  */
    void remove(const Range &other);

    /** @brief Возвращает результат вычитация из текущего диапазона
      * @param[IN] other диапазон, который необходимо удалить
      * @warning Есть одно НО в работе функции:  если диапазон other находится внутри нашего диапазона (*this), по идее нужно разбивать
      * текущий диапазон на 2 диапазона "до" other и "после" other. Данная же функция обрежет текущий диапазон (*this) до начала other.begin
      *
      * Результат выполнения:
      * 1. Если диапазоны не пересекаются, вернет пустой диапазон
      * 2. Если диапазоны пересекаются, и other находится справа, то this->end = other.begin
      * 3. Если диапазоны пересекаются, и other находится слева, то this->begin = other.end
      * 4. Если range находится внутри текущего диапазона(*this) этот случай описан в warning
      *
      * В отличии от предыдущей функции, она не изменяет текущий диапазон, а лишь возвращает результат вычитания */
    [[nodiscard]] Range remove(const Range &other) const;

    /** @brief Расширяет текущий диапазон
      * @param[IN] other - диапазон с которым будет сложен текущий диапазон
      * @return диапазон начало которого соответствует min(begin, other.begin), а конеч соответсует max(end, other.end). */
    void extend(const Range &other);

private:
    QDateTime begin;
    QDateTime end;
};

}

#endif // RANGE_H
