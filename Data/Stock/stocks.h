/** @ingroup Data
  * @class DataStocks
  * @details Пример получения списка свечной информации для акции c ключем StockKey: @code
  *   //Получаем доступ к свечной информации
  *   const CandlesDataVector_t &candlesList = DataStocks::get()->getStocks(stockKey);
  *   //Производим необходимые действия/вычисления
  *   ...
  *   //Освобождаем доступ к свечной информации
  *   DataStocks::get()->releaseStocks(); @endcode
  * Доступ захватывается на чтение и производить чтение могут одновременно несколько потоков,
  * функция getStocks() гарантирует что до вызова releaseStocks() свечные данные изменены не будут. */

#ifndef STOCKS_H
#define STOCKS_H

#include <QObject>
#include <QReadWriteLock>

#include "stockkey.h"
#include "candle.h"
#include "Data/range.h"
//#include "Core/safe_ptr.h"

namespace Data {


/// Класс хранит свечные данные по акциям.
class Stocks : public QObject
{
public:
    QReadWriteLock rwMutex;

    explicit Stocks();

    /** @brief Предоставляет доступ к списку свечной информации
      * @todo возвращать потокобезопасную ссылку, который мы будет возвращать, который d конструкторе будет блокировать mutex, а в деструкторе отсобождать
      * @param[IN] key - ключ акции
      * @return Список свечной информации для акции с ключем StockKey
      * @details Вызывает метод getAccess(READ); базового класса IBaseData, после получения доступа
      * @warning По завершению работы с данными необходимо вызвать метод releaseStocks() */
    const Candles& getStock(const StockKey &key);


    /// Возвращает количество свечей по акции key
    long getCandlesCount(const StockKey &key);


    /** @brief Возвращает дату первой и последней свечи у заданной акции
      * @param[IN] key - ключ акции
      * @return Если есть, возвращает пару: дату первой свечи и дату последней, если список свечей пуст то пустую пару QDateTime()
      * @details Свечи хранятся в отсортированном виде, и дата первой и последней свечи это интервал, за который есть данные */
    Range getRange(const StockKey &key);


    /** @brief Добавляет список свечей
      * @param[IN] key - ключ акции
      * @param[IN] candles - список добавляемых свечей
      * @warning свечные данные будут перемещены из переданного массива candles (std::move).
      * @details Добавляет в список stocks переданные свечи, если какие то свечи уже были в списке stocks, то эти
      * свечи будут проигнорированы. Добавлятся будут только новые свечи.\n
      * Перед добавлением свечей, запрашивается доступ на изменение данных.\n
      * После добавления свечей, производится сортировка свечей по дате и освобождение доступа к данным. */
    Candles insertCandles(const StockKey &key, Candles &candles);

signals:
    void dataChanged();

protected:
    /** @brief Список акций
      * @param QString - ключ акции StockKey::keyToString();
      * @param CandlesDataVector - список свечных данных для данного ключа */
    std::unordered_map<QString, Candles> stocks;
    //Core::SafePtr< std::unordered_map<QString, Candles> > stocks;

private:
    Q_OBJECT
};

}

#endif // STOCKS_H
