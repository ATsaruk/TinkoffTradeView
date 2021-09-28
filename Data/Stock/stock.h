#ifndef STOCK_H
#define STOCK_H

#include <vector>
#include <QReadWriteLock>

#include "stockkey.h"
#include "candle.h"
#include "Data/range.h"

namespace Data {

class Stock
{
public:
    QReadWriteLock mutex;

    explicit Stock();
    explicit Stock(const StockKey &stockKey);
    explicit Stock(Stock &other);

    ///Задать ключ акции
    void setStockKey(const StockKey &key);

    ///Возвращает ключ акции
    const StockKey& key() const;

    ///Возвращает диапазон хранимых свечей
    Range range() const;

    ///Возвращает количество хранимых свечей
    size_t count() const;

    ///Ищет свечу с меткой времени равной time
    std::optional<const Candle*> find(const QDateTime &time) const;

    ///Добавляет свечи appendCandles в список candles
    Range append(Stock &stock);

    std::vector<Candle>& getCandles();
    const std::vector<Candle>& getCandles() const;

private:
    StockKey stockKey;
    std::vector<Candle> candles;
};

}

#endif // STOCK_H
