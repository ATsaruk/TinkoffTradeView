#ifndef ISTOCKS_H
#define ISTOCKS_H

#include <QObject>

#include "stock.h"
#include "../range.h"

namespace Data {

/** @brief Интерфейс для просмотра свечных данных
  * @todo 1. при получении доступа к свечам должен блокироваться mutex и сам разблокироваться
  * @todo 2. определить механизм загрузки новых свечей
  */

class StockViewReference;

class IStocks : public QObject
{
public:
    IStocks() {}
    virtual ~IStocks() {}

    /** @brief Возвращает запрашиваемую свечу
      * @param key - ключ акции
      * @param time - время свечи
      * @return Candle - запрашиваемая свеча
      *
      * Если свеча отсутсвует будет возвращен нулевой std::nullopt */
    virtual std::optional<const Candle*> getCandle(const StockKey &key, const QDateTime &time) = 0;

    /** @brief Проверяет начилие свечей из заданного диапазона
      * @param key - ключ акции
      * @param range - диапазон необходимых свечей
      * @return true - если все свечи из запрашиваемого диапазона доступны, false - запущена загрузка недостающих свечей
      *
      * Если свечи из запрашиваемого диапазона не загружены, загружает их
      * После загрузки будет сформирован сигнал newCandles(const Range &) */
    virtual bool checkCandles(const StockKey &key, const Range &range) = 0;

    /** @brief Сохраняет свечную информацию по акции
      * @param candles - сохраняемая акция
      * @return Возвращает диапазон вставленных свечей
      *
      *     Если данная акция отсутвует в списке акцй, сохраняет её целиком, если акция уже существует, то добавляет
      * свечи которые отсутствовали, возвращает диапазон, которому принадлежат добавленные свечи */
    virtual Range insert(Stock &candles) = 0;

protected:
    //virtual CandlePtr getCandlesForWrite(const StockKey &key) = 0;
    virtual std::shared_ptr<StockViewReference> getCandlesForRead(const StockKey &key, const QDateTime &begin = QDateTime(), const QDateTime &end = QDateTime()) const = 0;

signals:
    void newCandles(const StockKey &, const Range &);

    friend class StockViewGlobal;
};

}

#endif // ISTOCKS_H
