#ifndef STOCKKEY_H
#define STOCKKEY_H

#include <QDateTime>
#include <QJsonObject>

namespace Data {


/** @ingroup Data
  * @brief Класс ключа акции
  * @details Ключ акции состоит из figi (ID свечи) и interval (длительности свечи).\n
  * Роль interval поясню на примере: есть акция apple, на сервере брокера она хранится под figi: BBG000B9XRY4,
  * но по каждой акции имеется свечная информация с различным временным интервалом (interval). Тоесть есть бывают свечи
  * за 1 минуту, за 5 минут, за 15 минут и т.д. и если группировать свечи только по figi, то в одном листе (CandlesDataVector)
  * окажутся свечи с различными interval'ами, что затруднит дульнейшую работу с ними.\n
  * Плюс это можно назвать разновидность паттерна FlyWeight, т.к. мы не храним в каждой структуре CandleData: figi и interval.\n
  * @see DataStocks
  */
class StockKey
{
public:
    /// Перечисление длительности свечных интервалов
    enum class INTERVAL : uint8_t
    {
        MIN1,   ///< Свечи длительностью 1 минута
        MIN5,   ///< Свечи длительностью 5 минут
        MIN15,  ///< Свечи длительностью 15 минут
        MIN30,  ///< Свечи длительностью 30 минут
        HOUR,   ///< Свечи длительностью 1 час
        DAY,    ///< Свечи длительностью 1 день
        WEEK    ///< Свечи длительностью 1 неделя
    };

    explicit StockKey();
    explicit StockKey(const QString &figi, const INTERVAL &interval);
    StockKey& operator =(const StockKey &other);

    ///Возвращает figi акции
    const QString &figi() const;

    ///Возвращает interval свечи
    const INTERVAL &interval()const;

    ///Преобразует значение interval в строку
    QString intervalToString() const;

    ///Возвращает количество секунд в отрезке времени interval
    long intervalToSec() const;

    ///Возвращает ключ акции в формате строки
    const QString keyToString() const;

    ///Преобразует строку QString в interval
    static INTERVAL stringToInterval(QString stringInterval);

    /** @brief Взвращает поля первичного ключа акции (StockKey) из json объекта
      * @param[IN] json объект в котором содержится первичный ключ
      * @return Пару QString figi и QString interval
      * @details Вспомогательная статическая функция, для получения полей первичного ключа акции из json объекта,
      * можно использовать при анализе ответа от брокера. */
    void fromJson(const QJsonObject &json);

    friend bool operator== (const StockKey &c1, const StockKey &c2) { return ( (c1._figi == c2._figi) && (c1._interval == c2._interval) ); }
    friend bool operator!= (const StockKey &c1, const StockKey &c2) { return !(c1 == c2); }

private:
    QString _figi;             //ID для запроса свечи через OpenAPI Tinkoff
    INTERVAL _interval; //интервал из enum CANDLE_INTERVAL, определяет длительность свечи (1min, 5min, 15min, 30min, hour, 4hour, day, week)
};

}

#endif // STOCKKEY_H
