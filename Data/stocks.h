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

    std::pair<Range, size_t> getRange(const StockKey &key) const override;

public slots:
    void candlesLoaded();

protected:
    void appedStock(Stock &stock) override;
    SharedStockVewRef getCandlesForRead(const StockKey &key, const QDateTime &begin = QDateTime(), const QDateTime &end = QDateTime()) const override;

private:
    std::unordered_map<StockKey, Stock> stocks; /// Список акций
};

}

#endif // STOCKS_H
