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

#include <unordered_map>

#include "istocks.h"
#include "Stock/stock.h"

namespace Data {

class StockVew;

/// Класс хранит свечные данные по акциям.
class Stocks : public IStocks
{
    Q_OBJECT

public:
    explicit Stocks();
    ~Stocks();

    /** @brief Возвращает запрашиваемую свечу
      * @param key - ключ акции
      * @param time - время свечи
      * @return Candle - запрашиваемая свеча
      *
      * Если свеча отсутсвует будет возвращен нулевой std::shared_ptr */
    std::optional<const Candle*> getCandle(const StockKey &key, const QDateTime &time) override;

    /** @brief Проверяет начилие свечей из заданного диапазона
      * @param key - ключ акции
      * @param range - диапазон необходимых свечей
      * @return true - если все свечи из запрашиваемого диапазона доступны, false - запущена загрузка недостающих свечей
      *
      * Если свечи из запрашиваемого диапазона не загружены, загружает их
      * После загрузки будет сформирован сигнал newCandles(const Range &) */
    bool checkCandles(const StockKey &key, const Range &range) override;

    /** @brief Сохраняет свечную информацию по акции
      * @param key - сохраняемая акция
      * @param candles - сохраняемая акция
      * @return Возвращает диапазон вставленных свечей
      *
      *     Если данная акция отсутвует в списке акцй, сохраняет её целиком, если акция уже существует, то добавляет
      * свечи которые отсутствовали, возвращает диапазон, которому принадлежат добавленные свечи */
    Range insert(Stock &candles) override;

    /**********************************************************************/

public slots:
    void candlesLoaded();

protected:
    void appedStock(Stock &stock) override;
    std::shared_ptr<Data::StockViewReference<QReadLocker>> getCandlesForRead(const StockKey &key, const QDateTime &begin = QDateTime(), const QDateTime &end = QDateTime()) const override;

private:
    std::unordered_map<StockKey, Stock> stocks; /// Список акций
};

}

#endif // STOCKS_H
