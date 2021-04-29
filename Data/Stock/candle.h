#ifndef CANDLE_H
#define CANDLE_H

#include <vector>
#include <QDateTime>
#include <QJsonObject>

namespace Data {


/** @ingroup Data
  * @brief Структура содержащая данные по 1 японской свечи
  * @details https://ru.wikipedia.org/wiki/Японские_свечи
  * @see DataStocks */
struct Candle
{
    QDateTime dateTime; ///<дата/время начала свечи
    float open;         ///<цена открытия
    float close;        ///<цена закрытия
    float high;         ///<максимальная цена
    float low;          ///<минимальная цена
    uint64_t volume;    ///<объем торгов

    explicit Candle() = default;
    Candle(Candle &&) noexcept = default;
    Candle(const Candle &) = default;
    Candle& operator =(Candle&&) noexcept = default;
    Candle& operator =(const Candle&) = default;

    /** @brief Заполняет данные по свечи из Json объекта
      * @param[OUT] candle структура свечной информации, в которую будет помещен результат
      * @param[IN] json объект в котором содержится свечная информация
      * @details Вспомогательная статическая функция, для заполнения данных по свечи из json объекта,
      * можно использовать при анализе ответа от брокера. */
    void fromJson(const QJsonObject &json);

    friend bool operator>  (const Candle &c1, const Candle &c2) { return (c1.dateTime >  c2.dateTime); }
    friend bool operator<  (const Candle &c1, const Candle &c2) { return (c1.dateTime <  c2.dateTime); }
    friend bool operator>= (const Candle &c1, const Candle &c2) { return (c1.dateTime >= c2.dateTime); }
    friend bool operator<= (const Candle &c1, const Candle &c2) { return (c1.dateTime <= c2.dateTime); }
    friend bool operator== (const Candle &c1, const Candle &c2) { return (c1.dateTime == c2.dateTime); }
    friend bool operator!= (const Candle &c1, const Candle &c2) { return (c1.dateTime != c2.dateTime); }
};

/** @ingroup Data
  * @brief Список свечной информации */
using Candles = std::vector<Candle>;

}

#endif // CANDLE_H
