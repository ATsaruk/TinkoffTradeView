#ifndef CANDLE_H
#define CANDLE_H

#include <QDateTime>
#include <QJsonObject>

namespace Data {


/** @ingroup Data
  * @brief Структура содержащая данные по 1 японской свечи
  * @details https://ru.wikipedia.org/wiki/Японские_свечи
  * @see DataStocks */
class Candle
{
public:
    Candle(const QDateTime &date_, const float &open_, const float &close_, const float &high_, const float &low_, const long long volume_)
        : _dateTime(date_), _open(open_), _close(close_), _high(high_), _low(low_), _volume(volume_) { }
    Candle(Candle &&) noexcept = default;
    Candle(const Candle &) = default;
    Candle& operator =(Candle&&) noexcept = default;
    Candle& operator =(const Candle&) = default;

    const QDateTime& dateTime() const;
    const float& open() const;
    const float& close() const;
    const float& high() const;
    const float& low() const;
    const long long& volume() const;

    /** @brief Заполняет данные по свечи из Json объекта
      * @param[OUT] candle структура свечной информации, в которую будет помещен результат
      * @param[IN] json объект в котором содержится свечная информация
      * @details Вспомогательная статическая функция, для заполнения данных по свечи из json объекта,
      * можно использовать при анализе ответа от брокера. */
    static Candle fromJson(const QJsonObject &json);

    friend bool operator>  (const Candle &c1, const Candle &c2) { return (c1._dateTime >  c2._dateTime); }
    friend bool operator<  (const Candle &c1, const Candle &c2) { return (c1._dateTime <  c2._dateTime); }
    friend bool operator>= (const Candle &c1, const Candle &c2) { return (c1._dateTime >= c2._dateTime); }
    friend bool operator<= (const Candle &c1, const Candle &c2) { return (c1._dateTime <= c2._dateTime); }
    friend bool operator== (const Candle &c1, const Candle &c2) { return (c1._dateTime == c2._dateTime); }
    friend bool operator!= (const Candle &c1, const Candle &c2) { return (c1._dateTime != c2._dateTime); }
private:
    QDateTime _dateTime; ///<дата/время начала свечи
    float _open;         ///<цена открытия
    float _close;        ///<цена закрытия
    float _high;         ///<максимальная цена
    float _low;          ///<минимальная цена
    long long _volume;        ///<объем торгов
};

}

#endif // CANDLE_H
