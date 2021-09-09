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
    ///@return FALSE - если одна из дат !isValid или если begin >= end
    bool isValid() const;

    ///@return TRUE - если одна isValid();
    operator bool() const;

    ///@return TRUE - если одна !isValid();
    bool operator !() const;

    ///@return количество секунд в диапазоне
    qint64 toSec() const;

    ///@return TRUE - если date находится внутри диапазона (нестрогая проверка <=, >=)
    bool contains(const QDateTime &date) const;
    ///@return TRUE - если диапазон range является поддиапазоном (нестрогая проверка <=, >=)
    bool contains(const Range &range) const;

    /**
     * @brief Определяет имеют ли текущий диапазон пересечение с range
     * @return TRUE - если между диапазонами нет "зазора"
     */
    bool isIntersected(const Range &range) const;

    ///Задает начало диапазона
    void setBegin(const QDateTime &begin_);
    ///Задает конец диапазона
    void setEnd(const QDateTime &end_);

    /**
     * @brief Формирует начало и конец диапазона
     * @param[IN] date - начало/конец диапазона
     * @param[IN] duration - длительность диапазона
     *
     * Если duration положительное число, то интервал будет = [date ... date + duration].\n
     * Если duration отрицательное число, то интервал будет = [data + duration ... date], помним, что duration отрицательное значение
     * значит оно будет вычтено из date.
     */
    void setRange(const QDateTime &date, const long &duration);

    ///Добавляет заданное число секунд к дате начала и дате конца (может быть отрицательным)
    void addSecs(const long &secs);

    ///Добавляет заданное число дней к дате начала и дате конца (может быть отрицательным)
    void addDays(const long &days);

    ///Перемещает начало и конец диапазона на заданное число секунд
    void displace(const long &beginSecs, const long &endSecs);

    ///Ограничивает диапазон, делает не больше чем range
    void constrain(const Range &range);

    /**
     * @brief Удаляет часть диапазона
     * @param[IN] range диапазон, который необходимо удалить
     * @warning Есть одно НО в работе функции:  если диапазон other находится внутри нашего диапазона (*this), по идее нужно разбивать
     * текущий диапазон на 2 диапазона "до" range и "после" range. Данная же функция обрежет текущий диапазон (*this) до начала range.begin
     *
     * Результат выполнения:
     * 1. Если range находится внутри текущего диапазона(*this) этот случай описан в warning.
     * 2. Если диапазоны не пересекаются, ничего не произойдет.
     * 3. Если диапазоны пересекаются, и range находится справа, то this->end = range.begin.
     * 4. Если диапазоны пересекаются, и range находится слева, то this->begin = range.end.
     */
    void remove(const Range &range);

    /**
     * @brief Расширяет текущий диапазон
     * @param[IN] range - диапазон с которым будет сложен текущий диапазон
     * @return диапазон начало которого соответствует min(begin, range.begin), а конеч соответсует max(end, range.end).
     */
    void extend(const Range &range);

private:
    QDateTime begin;
    QDateTime end;
};

}

#endif // RANGE_H
